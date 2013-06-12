#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libusb.h>
#include <linux/ip.h>

#include "bootp.h"
#include "udp.h"
#include "ipv4.h"
#include "ether2.h"

int main(int argc, const char * argv[]) {
    /*bootp_packet *breq = (bootp_packet *)malloc(sizeof(bootp_packet));
    setup_bootp_packet(servername, filename, breq);
    
    unsigned char* bootpPacket = (unsigned char*)&breq;
    unsigned char *data = (unsigned char*)malloc(406*sizeof(unsigned char));
    int actual;
    

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

    dev_handle = libusb_open_device_with_vid_pid(ctx, 0x0451, 0x6141);
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
    
    r = libusb_bulk_transfer(dev_handle, (129 | LIBUSB_ENDPOINT_IN), data, 384, &actual, 0);
    printf("Read %d\n",actual);
    r = libusb_bulk_transfer(dev_handle, (2 | LIBUSB_ENDPOINT_OUT), bootpPacket, sizeof(*breq), &actual, 0);
    printf("Wrote %d\n",actual);
    r = libusb_bulk_transfer(dev_handle, (129 | LIBUSB_ENDPOINT_IN), data, 384, &actual, 0);
    if(r==0) {
        int i;
        for(i=0; i<actual; i++)printf("%c ",data[i]);
        printf("\n");
    }
    
    r = libusb_release_interface(dev_handle, 1); 
    if(r!=0) {
        printf("Cannot release interface!\n");
        return 1;
    }
    libusb_close(dev_handle); 
    libusb_exit(ctx);
    free(data);

    debug_bootp_packet(breq, sizeof(breq));
    free(breq);
    */

    udp_t *udp = (udp_t *)malloc(sizeof(udp_t));
    make_udp(udp, 30, 67, 68);
    debug_udp(udp, udp->udpLen);
    free(udp);

    struct iphdr *ip = (struct iphdr*)malloc(sizeof(struct iphdr)+sizeof(bootp_packet)+sizeof(udp_t));
    make_ipv4(ip,"192.168.0.1","192.168.0.2",17);
    debug_ipv4(ip);
    free(ip);

    u_int8_t dst[6] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06};
    u_int8_t src[6] = {0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c};
    struct ethhdr *eth2 = (struct ethhdr*)malloc(sizeof(struct ethhdr));
    make_ether2(eth2, dst, src);
    debug_ether2(eth2);
    free(eth2);

    return 0;
}

