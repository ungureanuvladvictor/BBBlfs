
#ifndef BOOTP_H
#define BOOTP_H

#include <sys/types.h>

#define BOOTP_OPTION_NETMASK		1
#define BOOTP_OPTION_GATEWAY		3
#define BOOTP_OPTION_DNS		    6
#define BOOTP_OPTION_HOSTNAME		12
#define BOOTP_OPTION_DOMAIN		    15

static const u_int8_t gateway[4] = { 192, 168, 0, 1 };
static const u_int8_t your_ip[4] = { 192, 168, 0, 2 };
static const u_int8_t BBB_ip[4] = { 192, 168, 0, 3 };
static const u_int8_t BBB_hwaddr[6] = {0xc8, 0xa0, 0x30, 0xa7, 0xd0, 0xfc};
static const u_int8_t magic_cookie[4] = { 99, 130, 83, 99 };

static const char servername[4] = {'v', 'v', 'u', '\0' };
static const char filename[4] = {'M', 'L', 'O', '\0' };

static const u_int8_t vendor[] = {
    99, 130, 83, 99,
    BOOTP_OPTION_NETMASK,  4, 255, 255, 255, 0, 0xFF
};

typedef u_int8_t ipv4addr[4];

typedef struct {
    u_int8_t opcode; /*!< 1 for BOOTP_REQUEST, 2 for BOOTP_RESPONSE */
    u_int8_t hw; /*!<  */
    u_int8_t hwlength; /*!< Length of the MAC */
    u_int8_t hopcount; /*!< Number of routers to pass until discarded */
    u_int32_t xid; /*!< Not used */
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

#endif
