#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libusb.h>
#include <linux/ip.h>
#include <unistd.h>

#include "rndis.h"
#include "ether2.h"
#include "ipv4.h"
#include "udp.h"
#include "bootp.h"
#include "tftp.h"

#include "arp.h"

#include "utils.h"

int main(int argc, const char * argv[]) {
    int actual;
    
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

    unsigned char *data = (unsigned char*)calloc(1, 1000);
    unsigned char *buffer = (unsigned char*)malloc(450 * sizeof(unsigned char));

    libusb_device **devs; 
    libusb_device_handle *dev_handle;
    libusb_context *ctx = NULL;

    int r;
    ssize_t cnt;

    r = libusb_init(&ctx);
    if(r < 0) {
        printf("Init error!\n");
        return 1;
    }
    libusb_set_debug(ctx, 3);
    cnt = libusb_get_device_list(ctx, &devs);
    if(cnt < 0) {
        printf("Cannot get device list!\n");
        return 1;
    }

    dev_handle = libusb_open_device_with_vid_pid(ctx, ROMVID, ROMPID);
    if(dev_handle == NULL) {
        printf("Cannot open device!\n");
        return 1;
    }

    libusb_free_device_list(devs, 1);
    if(libusb_kernel_driver_active(dev_handle, 0) == 1) {
        printf("Kernel has driver!\n");
        if(libusb_detach_kernel_driver(dev_handle, 0) == 0)
            printf("Kernel deattached!\n");
    }
    r = libusb_claim_interface(dev_handle, 1);
    if(r < 0) {
        printf("Cannot Claim Interface!\n");
        return 1;
    }
    
    bootp_packet *breq = (bootp_packet*)calloc(1, bootpSize);
    make_bootp(servername, filename, breq, 1);
    
    udp_t *udp = (udp_t*)calloc(1, udpSize);
    make_udp(udp, bootpSize, BOOTPS, BOOTPC);
    
    struct iphdr *ip = (struct iphdr*)calloc(1, ipSize);
    make_ipv4(ip, server_ip, BBB_ip, IPUDP, 0, ipSize + udpSize + bootpSize);
    
    struct ethhdr *ether = (struct ethhdr*)calloc(1, etherSize);
    memcpy(ether->h_dest, BBB_hwaddr, 6);
    memcpy(ether->h_source, my_hwaddr, 6);
    ether->h_proto = htons(ETHIPP);

    rndis_hdr *rndis = (rndis_hdr*)calloc(1, rndisSize);
    make_rndis(rndis, fullSize - rndisSize);
    
    memcpy(data, rndis, rndisSize);
    memcpy(data + rndisSize, ether, etherSize);
    memcpy(data + rndisSize + etherSize, ip, ipSize);
    memcpy(data + rndisSize + etherSize + ipSize, udp, udpSize);
    memcpy(data + rndisSize + etherSize + ipSize + udpSize, breq, bootpSize);

    r = libusb_bulk_transfer(dev_handle, (2 | LIBUSB_ENDPOINT_OUT), 
                                data, fullSize, &actual, 0);

    r = libusb_bulk_transfer(dev_handle, (129 | LIBUSB_ENDPOINT_IN),
                                buffer, 450, &actual, 0);

    arp_hdr *receivedArp = buffer + rndisSize + etherSize;

    arp_hdr *arpResponse = (arp_hdr*)calloc(1, arpSize);

    make_arp(arpResponse, 2, my_hwaddr, &receivedArp->ip_dest, 
             &receivedArp->hw_source, &receivedArp->ip_source);

    memset(data, 0, fullSize);

    make_rndis(rndis, etherSize + arpSize);
    ether->h_proto = htons(ETHARPP);
    memcpy(data, rndis, rndisSize);
    memcpy(data + rndisSize, ether, etherSize);
    memcpy(data + rndisSize + etherSize, arpResponse, arpSize);

    r = libusb_bulk_transfer(dev_handle, (2 | LIBUSB_ENDPOINT_OUT), 
                                data, rndisSize + etherSize 
                                + arpSize, &actual, 0);

    memset(buffer, 0, 450);

    r = libusb_bulk_transfer(dev_handle, (129 | LIBUSB_ENDPOINT_IN),
                                buffer, 450, &actual, 0);

    udp_t *udpSPL = buffer + rndisSize + etherSize + ipSize;
    tftp_data *tftp = (tftp_data*)calloc(1, sizeof(tftp_data));
    ether->h_proto = htons(ETHIPP);
    int blk_number = 1;
    FILE *send;
    send = fopen("/home/vvu/boot/MLO" ,"rb");
    char *reader = (char*)malloc(512 * sizeof(char));

    while(!feof(send)) {
        memset(reader, 0, 512);
        memset(data, 0, fullSize);
        memset(rndis, 0, rndisSize);
        memset(ip, 0, ipSize);
        memset(udp, 0, udpSize);
        int result = fread(reader, sizeof(char), 512, send);
        
        make_rndis(rndis, etherSize + ipSize + udpSize + tftpSize + result);
        make_ipv4(ip, server_ip, BBB_ip, IPUDP, 0, ipSize + udpSize + 
                  tftpSize + result);
        make_udp(udp, tftpSize + result, ntohs(udpSPL->udpDst), 
                 ntohs(udpSPL->udpSrc));
        make_tftp_data(tftp, 3, blk_number);
            
        memcpy(data, rndis, rndisSize);
        memcpy(data + rndisSize, ether, etherSize);
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

    sleep(1);
    
    r = libusb_init(&ctx);
    if(r < 0) {
        printf("Init error!\n");
        return 1;
    }
    libusb_set_debug(ctx, 3);
    cnt = libusb_get_device_list(ctx, &devs);
    if(cnt < 0) {
        printf("Cannot get device list!\n");
        return 1;
    }

    dev_handle = libusb_open_device_with_vid_pid(ctx, SPLVID, SPLPID);
    if(dev_handle == NULL) {
        printf("Cannot open device!\n");
        return 1;
    }

    libusb_free_device_list(devs, 1);
    if(libusb_kernel_driver_active(dev_handle, 0) == 1) {
        printf("Kernel has driver!\n");
        if(libusb_detach_kernel_driver(dev_handle, 0) == 0)
            printf("Kernel deattached!\n");
    }

    r = libusb_claim_interface(dev_handle, 1);
    
    memset(buffer, 0, 450);
    memset(data, 0, 1000);
    memset(rndis, 0, rndisSize);
    memset(ether, 0, etherSize);
    memset(ip, 0, ipSize);
    memset(udp, 0, udpSize);
    memset(breq, 0, bootpSize);

    
    r = libusb_bulk_transfer(dev_handle, (129 | LIBUSB_ENDPOINT_IN),
                            buffer, 450, &actual, 0);
    
    make_rndis(rndis, fullSize - rndisSize);
    ether->h_proto = htons(ETHIPP);
    make_ipv4(ip, server_ip, BBB_ip, IPUDP, 0, ipSize + udpSize + bootpSize);
    make_udp(udp, bootpSize, 
             ntohs(((udp_t*)(buffer + rndisSize + etherSize + ipSize))->udpDst),
             ntohs(((udp_t*)(buffer + rndisSize + etherSize + ipSize))->udpSrc));
    make_bootp(servername, uboot, breq, 
               ntohl(((bootp_packet*)(buffer + rndisSize + etherSize +
                ipSize + udpSize))->xid));
    
    memcpy(data, rndis, rndisSize);
    memcpy(data + rndisSize, ether, etherSize);
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
    ether->h_proto = htons(ETHARPP);
    memcpy(data, rndis, rndisSize);
    memcpy(data + rndisSize, ether, etherSize);
    memcpy(data +rndisSize + etherSize, arpResponse, arpSize);

    r = libusb_bulk_transfer(dev_handle, (1 | LIBUSB_ENDPOINT_OUT), 
                            data, rndisSize + etherSize +arpSize, &actual, 0);
    
    memset(buffer, 0, 450);
    
    r = libusb_bulk_transfer(dev_handle, (129 | LIBUSB_ENDPOINT_IN),
                        buffer, 450, &actual, 0);
    
    udp_t *received = buffer + rndisSize + etherSize + ipSize;
    ether->h_proto = htons(ETHIPP);

    blk_number = 1;
    send = fopen("/home/vvu/boot/uboot", "rb");
    memset(reader, 0, 512);
    
    while(!feof(send)) {
    	memset(data, 0, fullSize);
        memset(rndis, 0, rndisSize);
        memset(ip, 0, ipSize);
        memset(udp, 0, udpSize);

        int result = fread(reader, sizeof(char), 512, send);
        
        make_rndis(rndis, etherSize + ipSize + udpSize + tftpSize + result);
        make_ipv4(ip, server_ip, BBB_ip, IPUDP, 0, ipSize + udpSize + 
                  tftpSize + result);
        make_udp(udp, tftpSize + result, ntohs(received->udpDst), 
                 ntohs(received->udpSrc));
        make_tftp_data(tftp, 3, blk_number);
        
        memcpy(data, rndis, rndisSize);
        memcpy(data + rndisSize, ether, etherSize);
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
        return 1;
    }


    libusb_close(dev_handle); 
    libusb_exit(ctx);

    free(ether);
    free(arpResponse);
    free(breq);
    free(ip);
    free(udp);
    free(data);
    free(reader);
    free(send);
    free(buffer);

    return 0;
}