/*
 * Copyright 2014 Vlad V. Ungureanu <ungureanuvladvictor@gmail.com>.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this Github repository and wiki except in
 * compliance with the License. You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libusb.h>
#include <linux/ip.h>
#include <unistd.h>
#include <sys/stat.h>

#include "rndis.h"
#include "ether2.h"
#include "ipv4.h"
#include "udp.h"
#include "bootp.h"
#include "tftp.h"
#include "arp.h"

#include "utils.h"

int main(int UNUSED argc, const char UNUSED * argv[]) {
	int actual;  //variable for actual bytes transferred
	int result;	 //reads data stream of spl and uboot
	int r; // 0 on success and LIB_USB error code on failure
	int n = 100; // For handling can't open device

	// define size for all packets blocks
	ssize_t fullSize = sizeof(bootp_packet) + sizeof(udp_t) +
			   sizeof(struct iphdr) + sizeof(struct ethhdr) +
			   sizeof(rndis_hdr);
	ssize_t rndisSize = sizeof(rndis_hdr);
	ssize_t etherSize = sizeof(struct ethhdr);
	ssize_t arpSize = sizeof(arp_hdr);
	ssize_t ipSize = sizeof(struct iphdr);
	ssize_t udpSize = sizeof(udp_t);
	ssize_t bootpSize = sizeof(bootp_packet);
	ssize_t tftpSize = sizeof(tftp_data);

	// allocate memory for buffer and data packets
	unsigned char *data = (unsigned char*)calloc(1, 1000);
	unsigned char *buffer = (unsigned char*)malloc(450 *
				sizeof(unsigned char));

	FILE *send;	// file object

	libusb_device **devs = NULL;	//structure representing USB device
	libusb_device_handle *dev_handle = NULL;	//structure representing handler on USB device
	libusb_context *ctx = NULL;	//structure representing libusb session

	r = libusb_init(&ctx);	//Inirialize the libusb
	if (r < 0) {			//handling faliure
		printf("Init error!\n");
		exit(1);
	}
	libusb_set_debug(ctx, 3);	// Set log message verbosity	


	int a = n;	// Variable for n times trying to open device
	while (dev_handle == NULL) {	// when no device is opened
		r = libusb_get_device_list(ctx, &devs);	// get the usb device list
		if (r < 0) {	//handling failure
			printf("Cannot get device list for RNDIS.\n");
			exit(1);
		}
		dev_handle = libusb_open_device_with_vid_pid(ctx,
							     ROMVID, ROMPID);	//opening device with rom vendor and product id
		libusb_free_device_list(devs, 1);	// free device list as per documentation
		
		// Try n times
		a--;
		if(a==0){
			printf("Can't open RNDIS device. \n");
			exit(1);
		 }
	
	}

	


	if (libusb_kernel_driver_active(dev_handle, 0) == 1) { //check if kernel driver is active on interface 0
		libusb_detach_kernel_driver(dev_handle, 0);
	}

	r = libusb_claim_interface(dev_handle, 1);	// Claim the interface of the device
	if (r < 0) {
		printf("Cannot Claim Interface!\n");
		exit(1);
	}

// requesting buffer of 450 bytes from the device
	r = libusb_bulk_transfer(dev_handle, (129 | LIBUSB_ENDPOINT_IN),
				 buffer, 450, &actual, 0);

// RNDIS Packet header
	rndis_hdr *rndis = (rndis_hdr*)calloc(1, rndisSize);
	make_rndis(rndis, fullSize - rndisSize);

// Ethernet packet header
	struct ethhdr *ether = (struct ethhdr*)(buffer+rndisSize);
	struct ethhdr *eth2 = (struct ethhdr*)calloc(1, etherSize);
	make_ether2(eth2, ether->h_source, (unsigned char*)my_hwaddr);


// IP packet header having ip of server, client(BBB), port number
	struct iphdr *ip = (struct iphdr*)calloc(1, ipSize);
	make_ipv4(ip, server_ip, BBB_ip, IPUDP, 0, ipSize + udpSize +
		  bootpSize);

// UDP packet having BOOTP implemented UDP server and client ports
	udp_t *udp = (udp_t*)calloc(1, udpSize);
	make_udp(udp, bootpSize, BOOTPS, BOOTPC);

// BOOTP packet having server name, SPL filename, mac address of client
	bootp_packet *breq = (bootp_packet*)calloc(1, bootpSize);
	make_bootp(servername, filename, breq, 1, ether->h_source);

// Wrapping the packets layer by layer by coping them to memory starting from data 
	memcpy(data, rndis, rndisSize);
	memcpy(data + rndisSize, eth2, etherSize);
	memcpy(data + rndisSize + etherSize, ip, ipSize);
	memcpy(data + rndisSize + etherSize + ipSize, udp, udpSize);
	memcpy(data + rndisSize + etherSize + ipSize + udpSize,
	       breq, bootpSize);

	r = libusb_bulk_transfer(dev_handle, (2 | LIBUSB_ENDPOINT_OUT),
				 data, fullSize, &actual, 0); // BOOTP is transferred to USB device
	r = libusb_bulk_transfer(dev_handle, (129 | LIBUSB_ENDPOINT_IN),
				 buffer, 450, &actual, 0);	// Buffer is requested again from device


	arp_hdr *receivedArp = (arp_hdr*)(buffer + rndisSize + etherSize); //Received ARP header
	arp_hdr *arpResponse = (arp_hdr*)calloc(1, arpSize);	// Response ARP header

// ARP response is made with data(IP address, hw address) carried by ARP request
	make_arp(arpResponse, 2, my_hwaddr, &receivedArp->ip_dest,
		 (const uint8_t*)&receivedArp->hw_source,
		 &receivedArp->ip_source);

	memset(data, 0, fullSize);	// Memory block starting with data is reset with 0 to carry new data


	make_rndis(rndis, etherSize + arpSize);		// RNDIS packet
	eth2->h_proto = htons(ETHARPP);		// Host to Network conversion of ARP packet ID 
	
// Wrapping the packets for ARP response
	memcpy(data, rndis, rndisSize);
	memcpy(data + rndisSize, eth2, etherSize);
	memcpy(data + rndisSize + etherSize, arpResponse, arpSize);

// SEnding ARP response
	r = libusb_bulk_transfer(dev_handle, (2 | LIBUSB_ENDPOINT_OUT),
				 data, rndisSize + etherSize + arpSize,
				 &actual, 0);

	memset(buffer, 0, 450);	// Emptying the buffer

	r = libusb_bulk_transfer(dev_handle, (129 | LIBUSB_ENDPOINT_IN),
				 buffer, 450, &actual, 0);	// Buffer is requested agin from device

	udp_t *udpSPL = (udp_t*)(buffer + rndisSize + etherSize + ipSize);	// UDP packet
	tftp_data *tftp = (tftp_data*)calloc(1, sizeof(tftp_data));	// TFTP packet
	eth2->h_proto = htons(ETHIPP);	// Host to Network conversion of IP packet ID
	
	int blk_number = 1; // Numbering for blocks sent through TFTP

	send = fopen("spl", "rb");	// Open SPL file stream in read binary mode

	if (send == NULL) {		// Handling file not found
		perror("Cannot open spl binary");
	}

	char *reader = (char*)malloc(512 * sizeof(char));	// Allocationg memory block to reader

	while (!feof(send)) {		// looping till end of the file
		// Initializing memory to 0 for all packets
		memset(reader, 0, 512);
		memset(data, 0, fullSize);
		memset(rndis, 0, rndisSize);
		memset(ip, 0, ipSize);
		memset(udp, 0, udpSize);

		// 	Copy data from file in bytes to memory location pointed by reader 
		result = fread(reader, sizeof(char), 512, send);

		// Packets for all protocols
		make_rndis(rndis, etherSize + ipSize +
			   udpSize + tftpSize + result);
		make_ipv4(ip, server_ip, BBB_ip, IPUDP, 0, ipSize + udpSize +
			  tftpSize + result);
		make_udp(udp, tftpSize + result, ntohs(udpSPL->udpDst),
			 ntohs(udpSPL->udpSrc));	// network to host conversion of UDP port
		make_tftp_data(tftp, 3, blk_number);

		// Protocols are wrapped around data read from SPL file stream
		memcpy(data, rndis, rndisSize);
		memcpy(data + rndisSize, eth2, etherSize);
		memcpy(data + rndisSize + etherSize, ip, ipSize);
		memcpy(data + rndisSize + etherSize + ipSize, udp, udpSize);
		memcpy(data + rndisSize + etherSize + ipSize + udpSize,
		       tftp, tftpSize);
		memcpy(data + rndisSize + etherSize + ipSize + udpSize +
		       tftpSize, reader, result);

		// Transfer of SPL binary in TFTP blocks
		r = libusb_bulk_transfer(dev_handle, (2 | LIBUSB_ENDPOINT_OUT),
					 data, rndisSize + etherSize + ipSize +
					 udpSize + tftpSize + result,
					 &actual, 0);

		memset(buffer, 0, 450);	// Clear buffer with 0

		// Buffer is requested by host from device
		r = libusb_bulk_transfer(dev_handle,
					 (129 | LIBUSB_ENDPOINT_IN), buffer,
					 450, &actual, 0);

		blk_number++;	// TFTP block number increment
	}

	fclose(send);	// SPL FIle closed

	// release interface claimed earlier
	r = libusb_release_interface(dev_handle, 1);
	if (r < 0) {
		printf("Cannot release interface!\n");
		exit(1);
	}

	libusb_close(dev_handle);	// libusb closed to set dev_handle to NULL

	sleep(1.5);





// Now u boot file transfer





	libusb_get_device_list(ctx, &devs);	// Getting device list
	dev_handle = libusb_open_device_with_vid_pid(ctx, SPLVID, SPLPID);	// Finding device now by SPL vid an pid

	int b = n;	// Variable for n times trying to open device
	while (dev_handle == NULL) {	// Trying to find SPL device
		r = libusb_get_device_list(ctx, &devs);
		if (r < 0) {
			printf("Cannot get device list for SPL.\n");
			exit (1);
		}
		dev_handle = libusb_open_device_with_vid_pid(ctx, SPLVID,
							     SPLPID);
		libusb_free_device_list(devs, 1);

		// Try n times
		b--;
		if(b==0){
			printf("Can't open SPL device. \n");
			exit(1);
		 }
	}


	if (libusb_kernel_driver_active(dev_handle, 0) == 1) {
		libusb_detach_kernel_driver(dev_handle, 0);
	}

	r = libusb_claim_interface(dev_handle, 1);

	printf("SPL has started!\n\n");

	memset(buffer, 0, 450);
	memset(data, 0, 1000);
	memset(rndis, 0, rndisSize);
	memset(eth2, 0, etherSize);
	memset(ip, 0, ipSize);
	memset(udp, 0, udpSize);
	memset(breq, 0, bootpSize);

	r = libusb_bulk_transfer(dev_handle, (129 | LIBUSB_ENDPOINT_IN),
				 buffer, 450, &actual, 0);

	make_rndis(rndis, fullSize - rndisSize);
	eth2->h_proto = htons(ETHIPP);
	make_ipv4(ip, server_ip, BBB_ip, IPUDP, 0, ipSize +
		  udpSize + bootpSize);
	make_udp(udp, bootpSize, ntohs(((udp_t*)(buffer + rndisSize +
						 etherSize + ipSize))->udpDst),
		 ntohs(((udp_t*)(buffer + rndisSize +
				 etherSize + ipSize))->udpSrc));
	make_bootp(servername, uboot, breq, ntohl(((bootp_packet*)(buffer +
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

	make_rndis(rndis, etherSize + arpSize);
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
	send = fopen("uboot", "rb");

	if (send == NULL) {
		perror("Cannot open uboot binary");
	}

	memset(reader, 0, 512);

	while (!feof(send)) {
		memset(data, 0, fullSize);
		memset(rndis, 0, rndisSize);
		memset(ip, 0, ipSize);
		memset(udp, 0, udpSize);

		result = fread(reader, sizeof(char), 512, send);

		make_rndis(rndis, etherSize + ipSize +
			   udpSize + tftpSize + result);
		make_ipv4(ip, server_ip, BBB_ip, IPUDP, 0, ipSize + udpSize +
			  tftpSize + result);
		make_udp(udp, tftpSize + result, ntohs(received->udpDst),
			 ntohs(received->udpSrc));
		make_tftp_data(tftp, 3, blk_number);

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

	r = libusb_release_interface(dev_handle, 1);
	if (r < 0) {
		printf("Cannot release interface!\n");
		exit(1);
	}
	libusb_close(dev_handle);

	sleep(3);





// Now FIT binary transfer





	dev_handle = NULL;

	int c = n;	// Variable for n times trying to open device
	while (dev_handle == NULL) {
		r = libusb_get_device_list(ctx, &devs);
		if (r < 0) {
			printf("Cannot get device list uboot.\n");
			exit (1);
		}
		dev_handle = libusb_open_device_with_vid_pid(ctx,
				UBOOTVID, UBOOTPID);
		libusb_free_device_list(devs, 1);

		// Try n times
		c--;
		if(c==0){
			printf("Can't open uboot device. \n");
			exit(1);
		 }
	}

	if (libusb_kernel_driver_active(dev_handle, 0) == 1) {
		libusb_detach_kernel_driver(dev_handle, 0);
	}

	r = libusb_claim_interface(dev_handle, 1);

	printf("U-Boot has started! Sending now the FIT image!\n\n");

	memset(data, 0, fullSize);
	make_rndis(rndis, etherSize + arpSize);
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
	send = fopen("fit", "rb");

	if (send == NULL) {
		perror("Cannot open fit binary");
		exit(1);
	}

	memset(reader, 0, 512);

	while (!feof(send)) {
		memset(data, 0, fullSize);
		memset(rndis, 0, rndisSize);
		memset(ip, 0, ipSize);
		memset(udp, 0, udpSize);

		result = fread(reader, sizeof(char), 512, send);
		make_rndis(rndis, etherSize + ipSize +
			   udpSize + tftpSize + result);
		make_ipv4(ip, server_ip, BBB_ip, IPUDP, 0, ipSize + udpSize +
			  tftpSize + result);
		make_udp(udp, tftpSize + result, ntohs(received->udpDst),
			 ntohs(received->udpSrc));
		make_tftp_data(tftp, 3, blk_number);

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

	// Release interface claimed earlier
	r = libusb_release_interface(dev_handle, 1);
	if (r < 0) {
		printf("Cannot release interface!\n");
		exit(1);
	}

	libusb_close(dev_handle);
	libusb_exit(ctx);

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

	return 0;
}
