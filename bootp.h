
#ifndef BOOTP_H
#define BOOTP_H

#include <sys/types.h>

static const u_int8_t server_ip[4] = {192, 168, 1, 9};
static const u_int8_t BBB_ip[4] = { 192, 168, 1, 3 };
static const u_int8_t BBB_hwaddr[6] = { 0xc8, 0xa0, 0x30, 0xa7, 0xd0, 0xfc};
static const u_int8_t my_hwaddr[6] = { 0x9a, 0x1f, 0x85, 0x1c, 0x3d, 0x0e};

static const char servername[6] = {'c', 'r', 'o', 's', 's', '\0' };
static const char filename[19] = {'/', 'h', 'o', 'm', 'e', '/', 'v', 'v', 'u', '/', 'b', 'o', 'o', 't', '/', 'M', 'L', 'O', '\0' };

static const u_int8_t vendor[] = {
    99, 130, 83, 99,
    1,  4, 255, 255, 255, 0, 0xFF
};

typedef u_int8_t ipv4addr[4];

typedef struct bootp {
    u_int8_t opcode; /*!< 1 for BOOTP_REQUEST, 2 for BOOTP_RESPONSE */
    u_int8_t hw; /*!< */
    u_int8_t hwlength; /*!< Length of the MAC */
    u_int8_t hopcount; /*!< Number of routers to pass until discarded */
    u_int32_t xid; /*!< Transaction ID */
    u_int16_t secs; /*!< Seconds since boot */
    u_int16_t flags; /*!<  */
    ipv4addr ciaddr; /*!< IP address filled by client in REQUEST if known */
    ipv4addr yiaddr; /*!< IP address of the client filled by server in RESPONSE */
    ipv4addr server_ip; /*!< IP address of the BOOTP server */
    ipv4addr bootp_gw_ip; /*!< IP address of the gateway */
    u_int8_t hwaddr[16]; /*!< MAC address of the client */
    u_int8_t servername[64]; /*!< Name of the server */
    u_int8_t bootfile[128]; /*!< File to be booted */
    u_int8_t vendor[64]; /*!< Vendor extensions(subnet here for this project) */
} bootp_packet;

typedef u_int8_t hwaddr_t[6];

void make_bootp(const char *servername, const char *filename, 
                bootp_packet *bpp);
void debug_bootp(bootp_packet *breq, int breqlen);

#endif
