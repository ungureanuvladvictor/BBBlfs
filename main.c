#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libusb.h>
#include <linux/ip.h>

#include "bootp.h"
#include "udp.h"
#include "ipv4.h"
#include "ether2.h"
#include "rndis.h"

void hexDump (char *desc, void *addr, int len);

int main(int argc, const char * argv[]) {
    unsigned char *data = (unsigned char*)malloc(800*sizeof(unsigned char));
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
    
    while(libusb_bulk_transfer(dev_handle, (129 | LIBUSB_ENDPOINT_IN), data, 800, &actual, 0) ==0) {
        printf("Got %d\n",actual);
        rndis_hdr *rndis = calloc(1,sizeof(rndis_hdr));
        rndis = (rndis_hdr*)data;
        debug_rndis(rndis);
        
        struct ethhdr *eth2 = calloc(1,sizeof(struct ethhdr));
        eth2 = (struct ethhdr*)(data+sizeof(rndis_hdr));
        debug_ether2(eth2);

        struct iphdr *ip = calloc(1, sizeof(struct iphdr));
        ip = (struct iphdr*)(data + sizeof(rndis_hdr) + sizeof(struct ethhdr));
        debug_ipv4(ip);

        udp_t *udp = calloc(1, sizeof(udp_t));
        udp = (udp_t*)(data + sizeof(rndis_hdr) + sizeof(struct ethhdr) + sizeof(struct iphdr));
        debug_udp(udp, sizeof(udp_t));
        
        bootp_packet *bootp = calloc(1, sizeof(bootp_packet));
        bootp = (bootp_packet*)(data + sizeof(rndis_hdr) + sizeof(struct ethhdr) + sizeof(struct iphdr) + sizeof(udp_t));
        debug_bootp(bootp, sizeof(bootp_packet));

        free(data);
        data = (unsigned char*)malloc(800*sizeof(unsigned char));
    }
    
    r = libusb_release_interface(dev_handle, 1); 
    if(r!=0) {
        printf("Cannot release interface!\n");
        return 1;
    }
    libusb_close(dev_handle); 
    libusb_exit(ctx);
    free(data);

    /*
    bootp_packet *breq = (bootp_packet *)malloc(sizeof(bootp_packet));
    make_bootp(servername, filename, breq);
    debug_bootp(breq, sizeof(*breq));
    
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

    rndis_hdr *rndis = (rndis_hdr*)calloc(1,sizeof(rndis_hdr));
    make_rndis(rndis, 10);
    debug_rndis(rndis);
    free(rndis);
    */
    return 0;
}

void hexDump (char *desc, void *addr, int len) {
    int i;
    unsigned char buff[17];
    unsigned char *pc = addr;

    // Output description if given.
    if (desc != NULL)
        printf ("%s:\n", desc);

    // Process every byte in the data.
    for (i = 0; i < len; i++) {
        // Multiple of 16 means new line (with line offset).

        if ((i % 16) == 0) {
            // Just don't print ASCII for the zeroth line.
            if (i != 0)
                printf ("  %s\n", buff);

            // Output the offset.
            printf ("  %04x ", i);
        }

        // Now the hex code for the specific character.
        printf (" %02x", pc[i]);

        // And store a printable ASCII character for later.
        if ((pc[i] < 0x20) || (pc[i] > 0x7e))
            buff[i % 16] = '.';
        else
            buff[i % 16] = pc[i];
        buff[(i % 16) + 1] = '\0';
    }

    // Pad out last line if not exactly 16 characters.
    while ((i % 16) != 0) {
        printf ("   ");
        i++;
    }

    // And print the final ASCII bit.
    printf ("  %s\n", buff);
}