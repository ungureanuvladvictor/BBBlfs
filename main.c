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
#include "utils.h"

void hexDump (char *desc, void *addr, int len);

int main(int argc, const char * argv[]) {
    int actual;
    
    ssize_t fullSize = sizeof(bootp_packet) + sizeof(udp_t) + sizeof(struct iphdr) + sizeof(struct ethhdr) + sizeof(rndis_hdr);
    ssize_t rndisSize = sizeof(rndis_hdr);
    ssize_t etherSize = sizeof(struct ethhdr);
    ssize_t ipSize = sizeof(struct iphdr);
    ssize_t udpSize = sizeof(udp_t);
    ssize_t bootpSize = sizeof(bootp_packet);

    unsigned char *data = (unsigned char*)calloc(1, fullSize);
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
    
    r = libusb_bulk_transfer(dev_handle, (129 | LIBUSB_ENDPOINT_IN), 
    							buffer, 450, &actual, 0);

    hexDump("Received", buffer, 450);
    bootp_packet *breq = (bootp_packet*)calloc(1, sizeof(bootp_packet));
    make_bootp(servername, filename, breq);
    
    udp_t *udp = (udp_t*)calloc(1, sizeof(udp_t));
    make_udp(udp, sizeof(bootp_packet), 67, 68);
    
    struct iphdr *ip = (struct iphdr*)calloc(1, sizeof(struct iphdr));
    make_ipv4(ip, server_ip, BBB_ip, 17, 0);
    
    struct ethhdr *ether = (struct ethhdr*)calloc(1, sizeof(struct ethhdr));
    memcpy(ether->h_dest, BBB_hwaddr, 6);
    memcpy(ether->h_source, my_hwaddr, 6);
    ether->h_proto = ETH_P_IP;

    rndis_hdr *receivedRndis = (rndis_hdr*)buffer;

    hexDump("RNDIS Received", receivedRndis, rndisSize);
    debug_rndis(receivedRndis);
    rndis_hdr *rndis = (rndis_hdr*)calloc(1, sizeof(rndis_hdr));
    make_rndis(rndis, fullSize - sizeof(rndis_hdr));
    debug_rndis(rndis);
    hexDump("RNDIS", rndis, rndisSize);
    
    memcpy(data, rndis, rndisSize);
    memcpy(data + rndisSize, ether, etherSize);
    memcpy(data + rndisSize + etherSize, ip, ipSize);
    memcpy(data + rndisSize + etherSize + ipSize, udp, udpSize);
    memcpy(data + rndisSize + etherSize + ipSize + udpSize, breq, bootpSize);
    
    /*r = libusb_bulk_transfer(dev_handle, (2 | LIBUSB_ENDPOINT_OUT), 
                                data, fullSize, &actual, 0);

    r = libusb_bulk_transfer(dev_handle, (129 | LIBUSB_ENDPOINT_IN),
                                buffer, 450, &actual, 0);
    printf("%d\n", actual);
    hexDump("After sending", buffer, actual);*/

    r = libusb_release_interface(dev_handle, 1); 
    if(r!=0) {
        printf("Cannot release interface!\n");
        return 1;
    }

    libusb_close(dev_handle); 
    libusb_exit(ctx);
    free(ether);
    free(breq);
    free(ip);
    free(udp);
    free(data);
    free(buffer);

    return 0;
}
/*
struct ethhdr *eth2 = calloc(1, sizeof(struct ethhdr));
    eth2 = (struct ethhdr*)(data + sizeof(rndis_hdr));
       
    struct iphdr *ip = calloc(1, sizeof(struct iphdr));
    ip = (struct iphdr*)(data + sizeof(rndis_hdr) + sizeof(struct ethhdr));
       
    udp_t *udp = calloc(1, sizeof(udp_t));
    udp = (udp_t*)(data + sizeof(rndis_hdr) + sizeof(struct ethhdr) 
                        + sizeof(struct iphdr));
        
    bootp_packet *bootp = calloc(1, sizeof(bootp_packet));
    bootp = (bootp_packet*)(data + sizeof(rndis_hdr) + sizeof(struct ethhdr)
                                + sizeof(struct iphdr) + sizeof(udp_t));
        
    bootp_packet *response = (bootp_packet*)malloc(sizeof(bootp_packet));
    make_bootp(servername, filename, response);
    memcpy(&response->hwaddr, &bootp->hwaddr, 6);
       
    udp_t *udp_answer = (udp_t*)malloc(sizeof(udp_t));
    make_udp(udp_answer, sizeof(bootp_packet), udp->udpDst, udp->udpSrc);
       
    struct iphdr *ip_answer = (struct iphdr*)calloc(1, sizeof(struct iphdr) 
                                + sizeof(udp_t) + sizeof(bootp_packet));
    make_ipv4(ip_answer, "255.255.255.0", "0.0.0.0", 17, ip->id);
        
    unsigned char srcHwAddr[] = {0xc8, 0xbc, 0xc8, 0xe3, 0x5b, 0xac};
    struct ethhdr *eth_answer = (struct ethhdr*)calloc(1, sizeof(struct ethhdr));
    memcpy(&eth_answer->h_source, &srcHwAddr, 6);
    memcpy(&eth_answer->h_dest, &eth2->h_source, 6);
    eth_answer->h_proto = ETH_P_IP;
        
    rndis_hdr *rndis_answer = (rndis_hdr*)calloc(1, sizeof(rndis_hdr) 
        + sizeof(struct ethhdr) + sizeof(struct iphdr) 
        + sizeof(udp_t) + sizeof(bootp_packet));
    make_rndis(rndis_answer, sizeof(struct ethhdr) 
        + sizeof(struct iphdr) + sizeof(udp_t) + sizeof(bootp_packet));
    
    unsigned char* dump = (unsigned char*)malloc(800*sizeof(unsigned char));
    memcpy(dump, rndis_answer, sizeof(rndis_hdr));
    memcpy((dump+sizeof(rndis_hdr)), eth_answer, sizeof(struct ethhdr));
    memcpy(dump + sizeof(rndis_hdr) + sizeof(struct ethhdr), ip_answer, 
            sizeof(struct iphdr));
    memcpy(dump + sizeof(rndis_hdr) + sizeof(struct ethhdr) 
            + sizeof(struct iphdr), udp_answer, sizeof(udp_t));
    memcpy(dump + sizeof(rndis_hdr) + sizeof(struct ethhdr) 
        + sizeof(struct iphdr) + sizeof(udp_t), response, sizeof(bootp_packet));
    hexDump("Packet", dump, 384);

*/