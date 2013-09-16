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

void show_usage() {
    printf("Use like:\nboot emmc/card image\n");
    printf("emmc / card -- to choose if you want to flash the eMMC or a uSD card\n");
    printf("image -- the name of the image you want to flash. Needs to be either .xz or .zip format\n");
}

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
    
    char *flash_type;
    FILE *send;

    libusb_device **devs; 
    libusb_device_handle *dev_handle;
    libusb_context *ctx = NULL;

    int r;
    ssize_t cnt;

    if (argc != 3) {
        show_usage();
        exit(1);
    }

    if (strcmp(argv[1], "emmc") == 0) {
        flash_type = "nsd";
    }
    else if (strcmp(argv[1], "card") == 0) {
        flash_type = "ysd";
    }
    else {
        show_usage();
        exit(1);
    }

    r = libusb_init(&ctx);
    if(r < 0) {
        printf("Init error!\n");
        exit(1);
    }
    libusb_set_debug(ctx, 3);
    cnt = libusb_get_device_list(ctx, &devs);
    if(cnt < 0) {
        printf("Cannot get device list!\n");
        exit(1);
    }

    dev_handle = libusb_open_device_with_vid_pid(ctx, ROMVID, ROMPID);
    if(dev_handle == NULL) {
        printf("Cannot open device!\n");
        exit(1);
    }

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
    
    struct ethhdr *ether = (struct ethhdr*)(buffer+rndisSize);
    struct ethhdr *eth2 = (struct ethhdr*)calloc(1, etherSize);
    make_ether2(eth2, ether->h_source, (unsigned char*)my_hwaddr);
    
    struct iphdr *ip = (struct iphdr*)calloc(1, ipSize);
    make_ipv4(ip, server_ip, BBB_ip, IPUDP, 0, ipSize + udpSize + bootpSize);

    udp_t *udp = (udp_t*)calloc(1, udpSize);
    make_udp(udp, bootpSize, BOOTPS, BOOTPC);

    bootp_packet *breq = (bootp_packet*)calloc(1, bootpSize);
    make_bootp(servername, filename, breq, 1, ether->h_source);

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
    eth2->h_proto = htons(ETHARPP);
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
    eth2->h_proto = htons(ETHIPP);
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
        int result = fread(reader, sizeof(char), 512, send);
        
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
    sleep(1);

    cnt = libusb_get_device_list(ctx, &devs);
    if(cnt < 0) {
        printf("Cannot get device list!\n");
        exit(1);
    }

    dev_handle = libusb_open_device_with_vid_pid(ctx, SPLVID, SPLPID);
    if(dev_handle == NULL) {
        printf("Cannot open device!\n");
        exit(1);
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
    eth2->h_proto = htons(ETHIPP);
    make_ipv4(ip, server_ip, BBB_ip, IPUDP, 0, ipSize + udpSize + bootpSize);
    make_udp(udp, bootpSize, 
             ntohs(((udp_t*)(buffer + rndisSize + etherSize + ipSize))->udpDst),
             ntohs(((udp_t*)(buffer + rndisSize + etherSize + ipSize))->udpSrc));
    make_bootp(servername, uboot, breq, 
               ntohl(((bootp_packet*)(buffer + rndisSize + etherSize +
                ipSize + udpSize))->xid), ether->h_source);

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
    eth2->h_proto = htons(ETHARPP);
    memcpy(data, rndis, rndisSize);
    memcpy(data + rndisSize, eth2, etherSize);
    memcpy(data +rndisSize + etherSize, arpResponse, arpSize);

    r = libusb_bulk_transfer(dev_handle, (1 | LIBUSB_ENDPOINT_OUT), 
                            data, rndisSize + etherSize +arpSize, &actual, 0);
    
    r = libusb_bulk_transfer(dev_handle, (129 | LIBUSB_ENDPOINT_IN),
                        buffer, 450, &actual, 0);

    udp_t *received = (udp_t*)(buffer + rndisSize + etherSize + ipSize);
    eth2->h_proto = htons(ETHIPP);

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

        int result = fread(reader, sizeof(char), 512, send);
        
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
    sleep(5);

    cnt = libusb_get_device_list(ctx, &devs);
    if(cnt < 0) {
        printf("Cannot get device list!\n");
        exit(1);
    }

    dev_handle = libusb_open_device_with_vid_pid(ctx, UBOOTVID, UBOOTPID);
    if(dev_handle == NULL) {
        printf("Cannot open device!\n");
        exit(1);
    }

    libusb_free_device_list(devs, 1);
    if(libusb_kernel_driver_active(dev_handle, 0) == 1) {
        libusb_detach_kernel_driver(dev_handle, 0);
    }

    r = libusb_claim_interface(dev_handle, 1);

    printf(" U-Boot has started! Sending now the FIT image!\n\n");

    memset(data, 0, fullSize);
    make_rndis(rndis, etherSize + arpSize);
    eth2->h_proto = htons(ETHARPP);
    memcpy(data, rndis, rndisSize);
    memcpy(data + rndisSize, eth2, etherSize);
    memcpy(data + rndisSize + etherSize, arpResponse, arpSize);

    r = libusb_bulk_transfer(dev_handle, (1 | LIBUSB_ENDPOINT_OUT), 
                                data, rndisSize + etherSize 
                                + arpSize, &actual, 0);
    memset(buffer, 0, 450);
        
    r = libusb_bulk_transfer(dev_handle, (129 | LIBUSB_ENDPOINT_IN),
                            buffer, 450, &actual, 0);
    
    eth2->h_proto = htons(ETHIPP);
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

        int result = fread(reader, sizeof(char), 512, send);
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

    sleep(10);

    cnt = libusb_get_device_list(ctx, &devs);
    if(cnt < 0) {
        printf("Cannot get device list!\n");
        exit(1);
    }

    dev_handle = libusb_open_device_with_vid_pid(ctx, SERIALVID, SERIALPID);
    if(dev_handle == NULL) {
        printf("Cannot open device!\n");
        exit(1);
    }

    printf(" Kernel has started! Now the image archive is being sent to the board.\n "\
        "During sending all 4 LEDs will be blinking.\n When flashing is done only 2 LEDs "\
        "will be blinking.\n");

    libusb_free_device_list(devs, 1);
    if(libusb_kernel_driver_active(dev_handle, 0) == 1) {
        libusb_detach_kernel_driver(dev_handle, 0);
    }
    r = libusb_claim_interface(dev_handle, 1);
    
    r = libusb_bulk_transfer(dev_handle, (1 | LIBUSB_ENDPOINT_OUT), 
                                (unsigned char*)flash_type, 3, &actual, 0);
    r = libusb_bulk_transfer(dev_handle, (1 | LIBUSB_ENDPOINT_OUT), 
                            (unsigned char*)argv[2], 12, &actual, 0);

    send = fopen(argv[2], "rb");

    if (send == NULL) {
        perror("Something wrong!\n");
        exit(1);
    }

    struct stat st;
    stat(argv[2], &st);
    int size = st.st_size;
    memcpy(data, (char*)&size, 4);
    r = libusb_bulk_transfer(dev_handle, (1 | LIBUSB_ENDPOINT_OUT), 
                            data, 4, &actual, 0);
    memset(reader, 0, 512);
    int total=0;
    free(reader);
    reader = malloc(4096 * sizeof(char));
    while(!feof(send)) {

        int result = fread(reader, sizeof(char), 4096, send);
        
    
        r = libusb_bulk_transfer(dev_handle, (1 | LIBUSB_ENDPOINT_OUT), 
                                (unsigned char*)reader, result, &actual, 0);
        total+=actual;
        memset(buffer, 0, 450);
        
        r = libusb_bulk_transfer(dev_handle, (129 | LIBUSB_ENDPOINT_IN),
                                buffer, 450, &actual, 0);
    }
    
    r = libusb_bulk_transfer(dev_handle, (129 | LIBUSB_ENDPOINT_IN),
                            buffer, 450, &actual, 0);
    sleep(2);
    r = libusb_bulk_transfer(dev_handle, (1 | LIBUSB_ENDPOINT_OUT), 
                                (unsigned char*)"FIN", 3, &actual, 0);
    libusb_close(dev_handle); 
    libusb_exit(ctx);

    free(data);
    free(buffer);

    return 0;
}