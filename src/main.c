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
    int actual;
    int result;

    ssize_t fullSize = sizeof(bootp_packet) + sizeof(udp_t) +
                       sizeof(iphdr_t) + sizeof(ethhdr_t) +
                       sizeof(rndis_hdr);
    ssize_t rndisSize = sizeof(rndis_hdr);
    ssize_t etherSize = sizeof(ethhdr_t);
    ssize_t arpSize = sizeof(arp_hdr);
    ssize_t ipSize = sizeof(iphdr_t);
    ssize_t udpSize = sizeof(udp_t);
    ssize_t bootpSize = sizeof(bootp_packet);
    ssize_t tftpSize = sizeof(tftp_data);

    unsigned char *data = (unsigned char*)calloc(1, 1000);
    unsigned char *buffer = (unsigned char*)malloc(450 * sizeof(unsigned char));

    FILE *send;

    libusb_device **devs;
    libusb_device_handle *dev_handle;
    libusb_context *ctx = NULL;

    int r;

    r = libusb_init(&ctx);
    if(r < 0) {
        printf("Init error!\n");
        exit(1);
    }
    libusb_set_debug(ctx, 3);

    while (libusb_get_device_list(ctx, &devs) > 0 && 
        (dev_handle = libusb_open_device_with_vid_pid(ctx, ROMVID, ROMPID)) == NULL);

    libusb_free_device_list(devs, 1);
    if(libusb_kernel_driver_active(dev_handle, 0) == 1) {
        libusb_detach_kernel_driver(dev_handle, 0);
    }
    r = libusb_claim_interface(dev_handle, 1);
    if(r < 0) {
        printf("Cannot Claim Interface!\n");
        exit(1);
    }

    r = libusb_bulk_transfer(dev_handle, (129 | LIBUSB_ENDPOINT_IN),
                                buffer, 450, &actual, 0);

    rndis_hdr *rndis = (rndis_hdr*)calloc(1, rndisSize);
    make_rndis(rndis, fullSize - rndisSize);

    ethhdr_t *ether = (ethhdr_t*)(buffer+rndisSize);
    ethhdr_t *eth2 = (ethhdr_t*)calloc(1, etherSize);
#ifdef __APPLE__
    make_ether2(eth2, ether->ether_shost, (unsigned char*)my_hwaddr);
#else
    make_ether2(eth2, ether->h_source, (unsigned char*)my_hwaddr);
#endif

    iphdr_t *ip = (iphdr_t*)calloc(1, ipSize);
    make_ipv4(ip, server_ip, BBB_ip, IPUDP, 0, ipSize + udpSize + bootpSize);

    udp_t *udp = (udp_t*)calloc(1, udpSize);
    make_udp(udp, bootpSize, BOOTPS, BOOTPC);

    bootp_packet *breq = (bootp_packet*)calloc(1, bootpSize);
#ifdef __APPLE__
    make_bootp(servername, filename, breq, 1, ether->ether_shost);
#else
    make_bootp(servername, filename, breq, 1, ether->h_source);
#endif

    memcpy(data, rndis, rndisSize);
    memcpy(data + rndisSize, eth2, etherSize);
    memcpy(data + rndisSize + etherSize, ip, ipSize);
    memcpy(data + rndisSize + etherSize + ipSize, udp, udpSize);
    memcpy(data + rndisSize + etherSize + ipSize + udpSize, breq, bootpSize);

    r = libusb_bulk_transfer(dev_handle, (2 | LIBUSB_ENDPOINT_OUT),
                                data, fullSize, &actual, 0);
    r = libusb_bulk_transfer(dev_handle, (129 | LIBUSB_ENDPOINT_IN),
                                buffer, 450, &actual, 0);
    arp_hdr *receivedArp = (arp_hdr*)(buffer + rndisSize + etherSize);

    arp_hdr *arpResponse = (arp_hdr*)calloc(1, arpSize);

    make_arp(arpResponse, 2, my_hwaddr, &receivedArp->ip_dest,
             (const uint8_t*)&receivedArp->hw_source, &receivedArp->ip_source);

    memset(data, 0, fullSize);

    make_rndis(rndis, etherSize + arpSize);
#ifdef __APPLE__
    eth2->ether_type = htons(ETHARPP);
#else
    eth2->h_proto = htons(ETHARPP);
#endif
    memcpy(data, rndis, rndisSize);
    memcpy(data + rndisSize, eth2, etherSize);
    memcpy(data + rndisSize + etherSize, arpResponse, arpSize);

    r = libusb_bulk_transfer(dev_handle, (2 | LIBUSB_ENDPOINT_OUT),
                                data, rndisSize + etherSize
                                + arpSize, &actual, 0);

    memset(buffer, 0, 450);

    r = libusb_bulk_transfer(dev_handle, (129 | LIBUSB_ENDPOINT_IN),
                                buffer, 450, &actual, 0);

    udp_t *udpSPL = (udp_t*)(buffer + rndisSize + etherSize + ipSize);
    tftp_data *tftp = (tftp_data*)calloc(1, sizeof(tftp_data));
#ifdef __APPLE__
    eth2->ether_type = htons(ETHIPP);
#else
    eth2->h_proto = htons(ETHIPP);
#endif
    int blk_number = 1;

    send = fopen("spl" ,"rb");

    if (send == NULL) {
        perror("Something wrong!\n");
    }

    char *reader = (char*)malloc(512 * sizeof(char));

    while(!feof(send)) {
        memset(reader, 0, 512);
        memset(data, 0, fullSize);
        memset(rndis, 0, rndisSize);
        memset(ip, 0, ipSize);
        memset(udp, 0, udpSize);
        result = fread(reader, sizeof(char), 512, send);

        make_rndis(rndis, etherSize + ipSize + udpSize + tftpSize + result);
        make_ipv4(ip, server_ip, BBB_ip, IPUDP, 0, ipSize + udpSize +
                  tftpSize + result);
        make_udp(udp, tftpSize + result, ntohs(udpSPL->udpDst),
                 ntohs(udpSPL->udpSrc));
        make_tftp_data(tftp, 3, blk_number);

        memcpy(data, rndis, rndisSize);
        memcpy(data + rndisSize, eth2, etherSize);
        memcpy(data + rndisSize + etherSize, ip, ipSize);
        memcpy(data + rndisSize + etherSize + ipSize, udp, udpSize);
        memcpy(data + rndisSize + etherSize + ipSize + udpSize, tftp, tftpSize);
        memcpy(data + rndisSize + etherSize + ipSize + udpSize +
               tftpSize, reader, result);

        r = libusb_bulk_transfer(dev_handle, (2 | LIBUSB_ENDPOINT_OUT),
                                data, rndisSize + etherSize + ipSize +
                                udpSize + tftpSize + result, &actual, 0);

        memset(buffer, 0, 450);

        r = libusb_bulk_transfer(dev_handle, (129 | LIBUSB_ENDPOINT_IN),
                                buffer, 450, &actual, 0);

        blk_number++;
    }

    fclose(send);
    libusb_close(dev_handle);
    
    sleep(1.5);
    
    libusb_get_device_list(ctx, &devs); 
    dev_handle = libusb_open_device_with_vid_pid(ctx, SPLVID, SPLPID);

    while (dev_handle == NULL) {
        libusb_get_device_list(ctx, &devs); 
        dev_handle = libusb_open_device_with_vid_pid(ctx, SPLVID, SPLPID);
    }
    
    if (dev_handle == NULL) {
        printf("Error! Cannot open SPL device!\n");
        return -1;
    }
    
    libusb_free_device_list(devs, 1);

    if(libusb_kernel_driver_active(dev_handle, 0) == 1) {
        libusb_detach_kernel_driver(dev_handle, 0);
    }

    r = libusb_claim_interface(dev_handle, 1);

    printf(" SPL has started!\n\n");

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
#ifdef __APPLE__
    eth2->ether_type = htons(ETHIPP);
#else
    eth2->h_proto = htons(ETHIPP);
#endif
    make_ipv4(ip, server_ip, BBB_ip, IPUDP, 0, ipSize + udpSize + bootpSize);
    make_udp(udp, bootpSize,
             ntohs(((udp_t*)(buffer + rndisSize + etherSize + ipSize))->udpDst),
             ntohs(((udp_t*)(buffer + rndisSize + etherSize + ipSize))->udpSrc));
#ifdef __APPLE__
    make_bootp(servername, uboot, breq,
               ntohl(((bootp_packet*)(buffer + rndisSize + etherSize +
                ipSize + udpSize))->xid), ether->ether_shost);
#else
    make_bootp(servername, uboot, breq,
               ntohl(((bootp_packet*)(buffer + rndisSize + etherSize +
                ipSize + udpSize))->xid), ether->h_source);
#endif

    memcpy(data, rndis, rndisSize);
    memcpy(data + rndisSize, eth2, etherSize);
    memcpy(data + rndisSize + etherSize, ip, ipSize);
    memcpy(data + rndisSize + etherSize + ipSize, udp, udpSize);
    memcpy(data + rndisSize + etherSize + ipSize + udpSize, breq, bootpSize);

    r = libusb_bulk_transfer(dev_handle, (1 | LIBUSB_ENDPOINT_OUT),
                                data, fullSize, &actual, 0);

    r = libusb_bulk_transfer(dev_handle, (129 | LIBUSB_ENDPOINT_IN),
                            buffer, 450, &actual, 0);

    memset(data, 0 , fullSize);
    memset(rndis, 0, rndisSize);

    make_rndis(rndis, etherSize + arpSize);
#ifdef __APPLE__
    eth2->ether_type = htons(ETHARPP);
#else
    eth2->h_proto = htons(ETHARPP);
#endif
    memcpy(data, rndis, rndisSize);
    memcpy(data + rndisSize, eth2, etherSize);
    memcpy(data +rndisSize + etherSize, arpResponse, arpSize);

    r = libusb_bulk_transfer(dev_handle, (1 | LIBUSB_ENDPOINT_OUT),
                            data, rndisSize + etherSize +arpSize, &actual, 0);

    r = libusb_bulk_transfer(dev_handle, (129 | LIBUSB_ENDPOINT_IN),
                        buffer, 450, &actual, 0);

    udp_t *received = (udp_t*)(buffer + rndisSize + etherSize + ipSize);
#ifdef __APPLE__
    eth2->ether_type = htons(ETHIPP);
#else
    eth2->h_proto = htons(ETHIPP);
#endif

    blk_number = 1;
    send = fopen("uboot", "rb");

    if (send == NULL) {
        perror("Something wrong!\n");
    }

    memset(reader, 0, 512);

    while(!feof(send)) {
        memset(data, 0, fullSize);
        memset(rndis, 0, rndisSize);
        memset(ip, 0, ipSize);
        memset(udp, 0, udpSize);

        result = fread(reader, sizeof(char), 512, send);

        make_rndis(rndis, etherSize + ipSize + udpSize + tftpSize + result);
        make_ipv4(ip, server_ip, BBB_ip, IPUDP, 0, ipSize + udpSize +
                  tftpSize + result);
        make_udp(udp, tftpSize + result, ntohs(received->udpDst),
                 ntohs(received->udpSrc));
        make_tftp_data(tftp, 3, blk_number);

        memcpy(data, rndis, rndisSize);
        memcpy(data + rndisSize, eth2, etherSize);
        memcpy(data + rndisSize + etherSize, ip, ipSize);
        memcpy(data + rndisSize + etherSize + ipSize, udp, udpSize);
        memcpy(data + rndisSize + etherSize + ipSize + udpSize, tftp, tftpSize);
        memcpy(data + rndisSize + etherSize + ipSize + udpSize +
               tftpSize, reader, result);

        r = libusb_bulk_transfer(dev_handle, (1 | LIBUSB_ENDPOINT_OUT),
                                data, rndisSize + etherSize + ipSize +
                                udpSize + tftpSize + result, &actual, 0);

        memset(buffer, 0, 450);

        r = libusb_bulk_transfer(dev_handle, (129 | LIBUSB_ENDPOINT_IN),
                                buffer, 450, &actual, 0);

        blk_number++;
    }

    r = libusb_release_interface(dev_handle, 1);
    if(r!=0) {
        printf("Cannot release interface!\n");
        exit(1);
    }
    libusb_close(dev_handle);
    
    sleep(3);

    while (libusb_get_device_list(ctx, &devs) > 0 && 
        (dev_handle = libusb_open_device_with_vid_pid(ctx, UBOOTVID, UBOOTPID)) == NULL);

    libusb_free_device_list(devs, 1);
    
    if(libusb_kernel_driver_active(dev_handle, 0) == 1) {
        libusb_detach_kernel_driver(dev_handle, 0);
    }

    r = libusb_claim_interface(dev_handle, 1);

    printf(" U-Boot has started! Sending now the FIT image!\n\n");

    memset(data, 0, fullSize);
    make_rndis(rndis, etherSize + arpSize);
#ifdef __APPLE__
    eth2->ether_type = htons(ETHARPP);
#else
    eth2->h_proto = htons(ETHARPP);
#endif
    memcpy(data, rndis, rndisSize);
    memcpy(data + rndisSize, eth2, etherSize);
    memcpy(data + rndisSize + etherSize, arpResponse, arpSize);

    r = libusb_bulk_transfer(dev_handle, (1 | LIBUSB_ENDPOINT_OUT),
                                data, rndisSize + etherSize
                                + arpSize, &actual, 0);
    memset(buffer, 0, 450);

    r = libusb_bulk_transfer(dev_handle, (129 | LIBUSB_ENDPOINT_IN),
                            buffer, 450, &actual, 0);

#ifdef __APPLE__
    eth2->ether_type = htons(ETHIPP);
#else
    eth2->h_proto = htons(ETHIPP);
#endif
    blk_number = 1;
    send = fopen("fit", "rb");

    if (send == NULL) {
        perror("Something wrong!\n");
        exit(1);
    }

    memset(reader, 0, 512);

    while(!feof(send)) {
        memset(data, 0, fullSize);
        memset(rndis, 0, rndisSize);
        memset(ip, 0, ipSize);
        memset(udp, 0, udpSize);

        result = fread(reader, sizeof(char), 512, send);
        make_rndis(rndis, etherSize + ipSize + udpSize + tftpSize + result);
        make_ipv4(ip, server_ip, BBB_ip, IPUDP, 0, ipSize + udpSize +
                  tftpSize + result);
        make_udp(udp, tftpSize + result, ntohs(received->udpDst),
                 ntohs(received->udpSrc));
        make_tftp_data(tftp, 3, blk_number);

        memcpy(data, rndis, rndisSize);
        memcpy(data + rndisSize, eth2, etherSize);
        memcpy(data + rndisSize + etherSize, ip, ipSize);
        memcpy(data + rndisSize + etherSize + ipSize, udp, udpSize);
        memcpy(data + rndisSize + etherSize + ipSize + udpSize, tftp, tftpSize);
        memcpy(data + rndisSize + etherSize + ipSize + udpSize +
               tftpSize, reader, result);

        r = libusb_bulk_transfer(dev_handle, (1 | LIBUSB_ENDPOINT_OUT),
                                data, rndisSize + etherSize + ipSize +
                                udpSize + tftpSize + result, &actual, 0);

        memset(buffer, 0, 450);

        r = libusb_bulk_transfer(dev_handle, (129 | LIBUSB_ENDPOINT_IN),
                                buffer, 450, &actual, 0);

        blk_number++;
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

    /* Freeing data buffers */
    free(data);
    free(buffer);
    free(reader);
    
    return 0;
}
