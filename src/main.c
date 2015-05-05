#define _WINSOCKAPI_
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <conio.h>

#include "rndis.h"
#include "ether2.h"
#include "ipv4.h"
#include "udp.h"
#include "bootp.h"
#include "tftp.h"
#include "arp.h"
#include "libusb.h"
#include "utils.h"


int main(int argc, char **argv) {
	int r = 0;
	int actual;
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
	libusb_device **devs = NULL;
	libusb_device_handle *dev_handle = NULL;
	libusb_context *ctx = NULL;
	
	unsigned char *data = (unsigned char*)calloc(1, 1000);
	unsigned char *buffer = (unsigned char*)malloc(450 *
		sizeof(unsigned char));
	unsigned char *rndis_buf = (unsigned char *)malloc(CONTROL_BUFFER_SIZE * sizeof(unsigned char));
	rndis_init_hdr *init_msg = (rndis_init_hdr*)malloc(sizeof(rndis_init_hdr));
	rndis_set_hdr *set_msg = (rndis_set_hdr*)malloc(sizeof(rndis_set_hdr));
	
	r = libusb_init(&ctx);
	if (r < 0) {
		printf("Init error!\n");
		exit(1);
	}
	libusb_set_debug(ctx, 3);

	printf("After you connected the BBB into USB boot mode press enter!\n");
	r = _getch();

	while (dev_handle == NULL) {
		r = libusb_get_device_list(ctx, &devs);
		if (r < 0) {
			printf("Cannot get device list.\n");
		}
		dev_handle = libusb_open_device_with_vid_pid(ctx,
			ROMVID, ROMPID);
		libusb_free_device_list(devs, 1);
	}

	r = libusb_claim_interface(dev_handle, 0);
	if (r < 0) {
		printf("Cannot claim ctl interface!\n");
		exit(1);
	}

	r = libusb_claim_interface(dev_handle, 1);
	if (r < 0) {
		printf("Cannot claim bulk interface!\n");
		exit(1);
	}
	
	make_rndis_init_hdr(init_msg);
	memset(rndis_buf, 0, CONTROL_BUFFER_SIZE);
	memcpy(rndis_buf, init_msg, sizeof(rndis_init_hdr));
	r = rndis_send_init(dev_handle, rndis_buf, sizeof(rndis_init_hdr));
	make_rndis_set_hdr(set_msg);
	memset(rndis_buf, 0, CONTROL_BUFFER_SIZE);
	memcpy(rndis_buf, set_msg, sizeof(rndis_set_hdr) + 4);
	*(uint32_t *)(rndis_buf + sizeof(rndis_set_hdr)) = cpu_to_le32(RNDIS_DEFAULT_FILTER);
	r = rndis_send_filter(dev_handle, rndis_buf, sizeof(rndis_set_hdr) + 4);
	r = libusb_bulk_transfer(dev_handle, (129 | LIBUSB_ENDPOINT_IN),
		buffer, 450, &actual, 0);

	rndis_data_hdr *rndis = (rndis_data_hdr*)calloc(1, rndisSize);
	make_rndis_data_hdr(rndis, fullSize - rndisSize);

	struct ethhdr *ether = (struct ethhdr*)(buffer + rndisSize);
	struct ethhdr *eth2 = (struct ethhdr*)calloc(1, etherSize);
	make_ether2(eth2, ether->h_source, (unsigned char*)my_hwaddr);

	struct iphdr *ip = (struct iphdr*)calloc(1, ipSize);
	make_ipv4(ip, server_ip, BBB_ip, IPUDP, 0, ipSize + udpSize +
		bootpSize);

	udp_t *udp = (udp_t*)calloc(1, udpSize);
	make_udp(udp, bootpSize, BOOTPS, BOOTPC);

	bootp_packet *breq = (bootp_packet*)calloc(1, bootpSize);
	make_bootp(servername, filename, breq, 1, ether->h_source);

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

	make_arp(arpResponse, 2, my_hwaddr, &receivedArp->ip_dest,
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


	libusb_close(dev_handle);
	libusb_exit(ctx);

	free(buffer);
	free(data);
	free(rndis_buf);
	free(init_msg);
	free(set_msg);

	_getch();
	
	return 0;
}