#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libusb.h>
#include "bootp.h"
#include "udp.h"

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
    free(breq);*/

    udp_t *udp = (udp_t *)malloc(sizeof(udp_t));
    make_udp(udp, 30, 67, 68);
    debug_udp(udp, udp->udpLen);

    return 0;
}

