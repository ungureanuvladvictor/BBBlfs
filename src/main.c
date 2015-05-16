/*
 * Copyright (C) 2013-2015 Vlad Ungureanu
 * Contact: Vlad Ungureanu <vvu@vdev.ro>
 *
 * This source is subject to the license found in the file 'LICENSE' which must
 * be be distributed together with this source. All other rights reserved.
 *
 * THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
 * EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.
 */

#include "rndis.h"
#include "ether2.h"
#include "ipv4.h"
#include "udp.h"
#include "bootp.h"
#include "tftp.h"
#include "arp.h"

#include "utils.h"
#include "config.h"

#define LIBUSB_DBG_LVL	3
#define TIMEOUT 	10
char *uboot_path, *spl_path, *fit_path;
libusb_context *ctx = NULL;

void set_paths()
{
#ifdef __linux__
	uboot_path = "../res/linux/uboot";
	spl_path = "../res/linux/spl";
#endif

#ifdef __APPLE__
	uboot_path = "../res/osx/uboot";
	spl_path = "../res/osx/spl";
#endif

	fit_path = "../res/fit";
}

int init_libusb()
{
	int ret = 0;
	ret = libusb_init(&ctx);
	if (ret < 0) {
		fprintf(stderr, "%s: cannot init libusb!\n", __FUNCTION__);
		return ret;
	}
	libusb_set_debug(ctx, LIBUSB_DBG_LVL);
	return ret;
}

int open_and_claim_dev(libusb_device_handle **dev_handle, uint16_t vid,
		       uint16_t pid)
{
	int ret = 0;
	libusb_device **devs = NULL;

	*(dev_handle) = NULL;

	time_t start = time(NULL);
	while (time(NULL) - start < TIMEOUT && *(dev_handle) == NULL) {
		ret = libusb_get_device_list(ctx, &devs);
		if (ret < 0) {
			fprintf(stderr, "%s: cannot get device list!\n", __FUNCTION__);
			return ret;
		}
		*(dev_handle) = libusb_open_device_with_vid_pid(ctx, vid, pid);
	}
	if (!*(dev_handle)) {
		fprintf(stderr, "%s: cannot open device(timeout) with vid: %#x and pid: %#x!\n", __FUNCTION__, vid, pid);
		return ret;
	}

#ifdef __linux__
	if (libusb_kernel_driver_active(*(dev_handle), 0)) {
		libusb_detach_kernel_driver(*(dev_handle), 0);
	}
	if (libusb_kernel_driver_active(*(dev_handle), 1)) {
		libusb_detach_kernel_driver(*(dev_handle), 1);
	}
#endif

	ret = libusb_claim_interface(*(dev_handle), 0);
	if (ret < 0) {
		fprintf(stderr, "%s: cannot claim control interface for vid: %#x and pid: %#x!\n", __FUNCTION__, vid, pid);
		return ret;
	}

	ret = libusb_claim_interface(*(dev_handle), 1);
	if (ret < 0) {
		fprintf(stderr, "%s: cannot claim bulk interface for vid: %#x and pid: %#x!\n", __FUNCTION__, vid, pid);
		return ret;
	}

	return ret;
}

int close_dev(libusb_device_handle **dev_handle)
{
	int ret = 0;

	ret = libusb_release_interface(*(dev_handle), 0);
	if (ret < 0) {
		fprintf(stderr, "%s: cannot release interface 0!\n", __FUNCTION__);
		return ret;
	}

	ret = libusb_release_interface(*(dev_handle), 1);
	if (ret < 0) {
		fprintf(stderr, "%s: cannot release interface 1!\n", __FUNCTION__);
		return ret;
	}

	libusb_close(*(dev_handle));

	*(dev_handle) = NULL;

	return ret;
}

int init_dev(libusb_device_handle **dev_handle)
{
	int ret = 0;
	unsigned char *rndis_buf = (unsigned char *)malloc(CONTROL_BUFFER_SIZE * sizeof(unsigned char));
	rndis_init_hdr *init_msg = (rndis_init_hdr*)malloc(1 * sizeof(rndis_init_hdr *));
	rndis_set_hdr *set_msg = (rndis_set_hdr*)malloc(1 * sizeof(rndis_set_hdr *));

	make_rndis_init_hdr(init_msg);
	memset(rndis_buf, 0, CONTROL_BUFFER_SIZE);
	memcpy(rndis_buf, init_msg, sizeof(rndis_init_hdr));

	ret = rndis_send_init(*(dev_handle), rndis_buf, sizeof(rndis_init_hdr));
	if (ret < 0) {
		fprintf(stderr, "%s: cannot send init RNDIS msg!\n", __FUNCTION__);
		return ret;
	}

	make_rndis_set_hdr(set_msg);
	memset(rndis_buf, 0, CONTROL_BUFFER_SIZE);
	memcpy(rndis_buf, set_msg, sizeof(rndis_set_hdr) + 4);
	*(uint32_t *)(rndis_buf + sizeof(rndis_set_hdr)) = cpu_to_le32(RNDIS_DEFAULT_FILTER);

	ret = rndis_send_filter(*(dev_handle), rndis_buf, sizeof(rndis_set_hdr) + 4);
	if (ret < 0) {
		fprintf(stderr, "%s: cannot set RNDIS filter!\n", __FUNCTION__);
		return ret;
	}

	return ret;
}

int main(int argc, const char **argv)
{
	int actual;
	int result;
	int r;

	ssize_t fullSize = sizeof(bootp_packet) + sizeof(udp_t) +
			   sizeof(struct iphdr) + sizeof(struct ethhdr) +
			   sizeof(rndis_data_hdr);
	ssize_t rndisSize = sizeof(rndis_data_hdr);
	ssize_t etherSize = sizeof(struct ethhdr);
	ssize_t arpSize = sizeof(arp_hdr);
	ssize_t ipSize = sizeof(struct iphdr);
	ssize_t udpSize = sizeof(udp_t);
	ssize_t bootpSize = sizeof(bootp_packet);
	ssize_t tftpSize = sizeof(tftp_data);

	unsigned char *data = (unsigned char*)calloc(1, 1000);
	unsigned char *buffer = (unsigned char*)malloc(450 *
				sizeof(unsigned char));
	unsigned char *rndis_buf = (unsigned char *)malloc(CONTROL_BUFFER_SIZE * sizeof(unsigned char));
	rndis_init_hdr *init_msg = (rndis_init_hdr*)malloc(1 * sizeof(rndis_init_hdr *));
	rndis_set_hdr *set_msg = (rndis_set_hdr*)malloc(1 * sizeof(rndis_set_hdr *));

	FILE *send;

	libusb_device **devs = NULL;
	libusb_device_handle *dev_handle = NULL;

	set_paths();

	r = init_libusb();
	if (r < 0)
		exit(EXIT_FAILURE);

	r = open_and_claim_dev(&dev_handle, ROMVID, ROMPID);
	if (r < 0) {
		exit(EXIT_FAILURE);
	}

	r = init_dev(&dev_handle);
	if (r < 0) {
		exit(EXIT_FAILURE);
	}

	r = libusb_bulk_transfer(dev_handle, (129 | LIBUSB_ENDPOINT_IN),
				 buffer, 450, &actual, 0);

	rndis_data_hdr *rndis = (rndis_data_hdr*)calloc(1, rndisSize);
	make_rndis_data_hdr(rndis, fullSize - rndisSize);

	struct ethhdr *ether = (struct ethhdr*)(buffer+rndisSize);
	struct ethhdr *eth2 = (struct ethhdr*)calloc(1, etherSize);
	make_ether2_packet(eth2, ether->h_source, (unsigned char*)my_hwaddr);

	struct iphdr *ip = (struct iphdr*)calloc(1, ipSize);
	make_ipv4_packet(ip, server_ip, BBB_ip, IPUDP, 0, ipSize + udpSize +
		  bootpSize);

	udp_t *udp = (udp_t*)calloc(1, udpSize);
	make_udp_packet(udp, bootpSize, htons(BOOTPS), htons(BOOTPC));

	bootp_packet *breq = (bootp_packet*)calloc(1, bootpSize);
	make_bootp_packet(servername, filename, breq, 1, ether->h_source);

	memcpy(data, rndis, rndisSize);
	memcpy(data + rndisSize, eth2, etherSize);
	memcpy(data + rndisSize + etherSize, ip, ipSize);
	memcpy(data + rndisSize + etherSize + ipSize, udp, udpSize);
	memcpy(data + rndisSize + etherSize + ipSize + udpSize,
	       breq, bootpSize);

	r = libusb_bulk_transfer(dev_handle, (2 | LIBUSB_ENDPOINT_OUT),
				 data, fullSize, &actual, 0);
	r = libusb_bulk_transfer(dev_handle, (129 | LIBUSB_ENDPOINT_IN),
				 buffer, 450, &actual, 0);

	arp_hdr *receivedArp = (arp_hdr*)(buffer + rndisSize + etherSize);
	arp_hdr *arpResponse = (arp_hdr*)calloc(1, arpSize);

	make_arp_packet(arpResponse, 2, my_hwaddr, &receivedArp->ip_dest,
		 (const uint8_t*)&receivedArp->hw_source,
		 &receivedArp->ip_source);

	memset(data, 0, fullSize);

	make_rndis_data_hdr(rndis, etherSize + arpSize);
	eth2->h_proto = htons(ETHARPP);
	memcpy(data, rndis, rndisSize);
	memcpy(data + rndisSize, eth2, etherSize);
	memcpy(data + rndisSize + etherSize, arpResponse, arpSize);

	r = libusb_bulk_transfer(dev_handle, (2 | LIBUSB_ENDPOINT_OUT),
				 data, rndisSize + etherSize + arpSize,
				 &actual, 0);

	memset(buffer, 0, 450);

	r = libusb_bulk_transfer(dev_handle, (129 | LIBUSB_ENDPOINT_IN),
				 buffer, 450, &actual, 0);

	udp_t *udpSPL = (udp_t*)(buffer + rndisSize + etherSize + ipSize);
	tftp_data *tftp = (tftp_data*)calloc(1, sizeof(tftp_data));
	eth2->h_proto = htons(ETHIPP);
	int blk_number = 1;

	send = fopen(spl_path, "rb");

	if (send == NULL) {
		fprintf(stderr, "%d: cannot open spl binary!\n", __LINE__);
	}

	char *reader = (char*)malloc(512 * sizeof(char));

	while (!feof(send)) {
		memset(reader, 0, 512);
		memset(data, 0, fullSize);
		memset(rndis, 0, rndisSize);
		memset(ip, 0, ipSize);
		memset(udp, 0, udpSize);
		result = fread(reader, sizeof(char), 512, send);

		make_rndis_data_hdr(rndis, etherSize + ipSize +
			   udpSize + tftpSize + result);
		make_ipv4_packet(ip, server_ip, BBB_ip, IPUDP, 0, ipSize + udpSize +
			  tftpSize + result);
		make_udp_packet(udp, tftpSize + result, udpSPL->udpDst,
			 udpSPL->udpSrc);
		make_tftp_data(tftp, TFTP_DATA, blk_number);

		memcpy(data, rndis, rndisSize);
		memcpy(data + rndisSize, eth2, etherSize);
		memcpy(data + rndisSize + etherSize, ip, ipSize);
		memcpy(data + rndisSize + etherSize + ipSize, udp, udpSize);
		memcpy(data + rndisSize + etherSize + ipSize + udpSize,
		       tftp, tftpSize);
		memcpy(data + rndisSize + etherSize + ipSize + udpSize +
		       tftpSize, reader, result);

		r = libusb_bulk_transfer(dev_handle, (2 | LIBUSB_ENDPOINT_OUT),
					 data, rndisSize + etherSize + ipSize +
					 udpSize + tftpSize + result,
					 &actual, 0);

		memset(buffer, 0, 450);

		r = libusb_bulk_transfer(dev_handle,
					 (129 | LIBUSB_ENDPOINT_IN), buffer,
					 450, &actual, 0);

		blk_number++;
	}

	fclose(send);
	r = close_dev(&dev_handle);
	if (r < 0) {
		exit(EXIT_FAILURE);
	}

	sleep(1.5);

	r = open_and_claim_dev(&dev_handle, SPLVID, SPLPID);
	if (r < 0) {
		exit(EXIT_FAILURE);
	}

	r = init_dev(&dev_handle);
	if (r < 0) {
		exit(EXIT_FAILURE);
	}

	fprintf(stdout, "SPL has started!\n");

	memset(buffer, 	0, 450);
	memset(data, 	0, 1000);
	memset(rndis, 	0, rndisSize);
	memset(eth2, 	0, etherSize);
	memset(ip, 	0, ipSize);
	memset(udp, 	0, udpSize);
	memset(breq, 	0, bootpSize);

	r = libusb_bulk_transfer(dev_handle, (129 | LIBUSB_ENDPOINT_IN),
				 buffer, 450, &actual, 0);

	make_rndis_data_hdr(rndis, fullSize - rndisSize);
	eth2->h_proto = htons(ETHIPP);
	make_ipv4_packet(ip, server_ip, BBB_ip, IPUDP, 0, ipSize +
		  udpSize + bootpSize);
	make_udp_packet(udp, bootpSize, ((udp_t*)(buffer + rndisSize +
						 etherSize + ipSize))->udpDst,
		 ((udp_t*)(buffer + rndisSize +
				 etherSize + ipSize))->udpSrc);
	make_bootp_packet(servername, uboot, breq, ntohl(((bootp_packet*)(buffer +
		   rndisSize + etherSize + ipSize + udpSize))->xid),
		   ether->h_source);

	memcpy(data, rndis, rndisSize);
	memcpy(data + rndisSize, eth2, etherSize);
	memcpy(data + rndisSize + etherSize, ip, ipSize);
	memcpy(data + rndisSize + etherSize + ipSize, udp, udpSize);
	memcpy(data + rndisSize + etherSize + ipSize + udpSize,
	       breq, bootpSize);

	r = libusb_bulk_transfer(dev_handle, (1 | LIBUSB_ENDPOINT_OUT),
				 data, fullSize, &actual, 0);

	r = libusb_bulk_transfer(dev_handle, (129 | LIBUSB_ENDPOINT_IN),
				 buffer, 450, &actual, 0);

	memset(data, 0 , fullSize);
	memset(rndis, 0, rndisSize);

	make_rndis_data_hdr(rndis, etherSize + arpSize);
	eth2->h_proto = htons(ETHARPP);
	memcpy(data, rndis, rndisSize);
	memcpy(data + rndisSize, eth2, etherSize);
	memcpy(data +rndisSize + etherSize, arpResponse, arpSize);

	r = libusb_bulk_transfer(dev_handle, (1 | LIBUSB_ENDPOINT_OUT),
				 data, rndisSize + etherSize +arpSize,
				 &actual, 0);

	r = libusb_bulk_transfer(dev_handle, (129 | LIBUSB_ENDPOINT_IN),
				 buffer, 450, &actual, 0);

	udp_t *received = (udp_t*)(buffer + rndisSize + etherSize + ipSize);
	eth2->h_proto = htons(ETHIPP);

	blk_number = 1;
	send = fopen(uboot_path, "rb");

	if (send == NULL) {
		fprintf(stderr, "%d: cannot open uboot binary!\n", __LINE__);
		exit(1);
	}

	memset(reader, 0, 512);

	while (!feof(send)) {
		memset(data, 0, fullSize);
		memset(rndis, 0, rndisSize);
		memset(ip, 0, ipSize);
		memset(udp, 0, udpSize);

		result = fread(reader, sizeof(char), 512, send);

		make_rndis_data_hdr(rndis, etherSize + ipSize +
			   udpSize + tftpSize + result);
		make_ipv4_packet(ip, server_ip, BBB_ip, IPUDP, 0, ipSize + udpSize +
			  tftpSize + result);
		make_udp_packet(udp, tftpSize + result, received->udpDst,
			 received->udpSrc);
		make_tftp_data(tftp, TFTP_DATA, blk_number);

		memcpy(data, rndis, rndisSize);
		memcpy(data + rndisSize, eth2, etherSize);
		memcpy(data + rndisSize + etherSize, ip, ipSize);
		memcpy(data + rndisSize + etherSize + ipSize, udp, udpSize);
		memcpy(data + rndisSize + etherSize + ipSize + udpSize,
		       tftp, tftpSize);
		memcpy(data + rndisSize + etherSize + ipSize + udpSize +
		       tftpSize, reader, result);

		r = libusb_bulk_transfer(dev_handle, (1 | LIBUSB_ENDPOINT_OUT),
					 data, rndisSize + etherSize + ipSize +
					 udpSize + tftpSize + result,
					 &actual, 0);

		memset(buffer, 0, 450);

		r = libusb_bulk_transfer(dev_handle,
					 (129 | LIBUSB_ENDPOINT_IN),
					 buffer, 450, &actual, 0);

		blk_number++;
	}

	fclose(send);

	r = close_dev(&dev_handle);
	if (r < 0) {
		exit(EXIT_FAILURE);
	}

	sleep(3);

	r = open_and_claim_dev(&dev_handle, UBOOTVID, UBOOTPID);
	if (r < 0) {
		exit(EXIT_FAILURE);
	}

	r = init_dev(&dev_handle);
	if (r < 0) {
		exit(EXIT_FAILURE);
	}

	fprintf(stdout, "U-Boot has started! Sending now the FIT image!\n");

	memset(data, 0, fullSize);
	make_rndis_data_hdr(rndis, etherSize + arpSize);
	eth2->h_proto = htons(ETHARPP);
	memcpy(data, rndis, rndisSize);
	memcpy(data + rndisSize, eth2, etherSize);
	memcpy(data + rndisSize + etherSize, arpResponse, arpSize);

	r = libusb_bulk_transfer(dev_handle, (1 | LIBUSB_ENDPOINT_OUT),
			         data, rndisSize + etherSize +
				 arpSize, &actual, 0);
	memset(buffer, 0, 450);

	r = libusb_bulk_transfer(dev_handle, (129 | LIBUSB_ENDPOINT_IN),
				 buffer, 450, &actual, 0);

	eth2->h_proto = htons(ETHIPP);
	blk_number = 1;
	send = fopen(fit_path, "rb");

	if (send == NULL) {
		fprintf(stderr, "%d: cannot open fit binary!\n", __LINE__);
		exit(1);
	}

	memset(reader, 0, 512);

	while (!feof(send)) {
		memset(data, 0, fullSize);
		memset(rndis, 0, rndisSize);
		memset(ip, 0, ipSize);
		memset(udp, 0, udpSize);

		result = fread(reader, sizeof(char), 512, send);
		make_rndis_data_hdr(rndis, etherSize + ipSize +
			   udpSize + tftpSize + result);
		make_ipv4_packet(ip, server_ip, BBB_ip, IPUDP, 0, ipSize + udpSize +
			  tftpSize + result);
		make_udp_packet(udp, tftpSize + result, received->udpDst,
			 received->udpSrc);
		make_tftp_data(tftp, TFTP_DATA, blk_number);

		memcpy(data, rndis, rndisSize);
		memcpy(data + rndisSize, eth2, etherSize);
		memcpy(data + rndisSize + etherSize, ip, ipSize);
		memcpy(data + rndisSize + etherSize + ipSize, udp, udpSize);
		memcpy(data + rndisSize + etherSize + ipSize +
		       udpSize, tftp, tftpSize);
		memcpy(data + rndisSize + etherSize + ipSize + udpSize +
		       tftpSize, reader, result);

		r = libusb_bulk_transfer(dev_handle, (1 | LIBUSB_ENDPOINT_OUT),
					 data, rndisSize + etherSize + ipSize +
					 udpSize + tftpSize + result,
					 &actual, 0);

		memset(buffer, 0, 450);

		r = libusb_bulk_transfer(dev_handle,
					 (129 | LIBUSB_ENDPOINT_IN),
					 buffer, 450, &actual, 0);

		blk_number++;
	}

	fclose(send);

	r = close_dev(&dev_handle);
	if (r < 0) {
		exit(EXIT_FAILURE);
	}

	/* Freeing used structures */
	free(rndis);
	free(ip);
	free(udp);
	free(arpResponse);
	free(breq);
	free(tftp);
	free(eth2);

	/* Freeing data buffers */
	free(data);
	free(buffer);
	free(reader);

	//TODO FIXME
	// libusb_exit(ctx);
	return 0;
}
