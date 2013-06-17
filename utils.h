#ifndef UTILS_H
#define UTILS_H

#define BOOTPS 67
#define BOOTPC 68
#define ROMXID 1
#define IPUDP 17
#define ETHIPP 0x0800
#define ETHARPP 0x0806
#define ROMVID 0x0451
#define ROMPID 0x6141
#define SPLVID 0x525
#define SPLPID 0xA4A2

static const u_int8_t server_ip[4] = { 0xc0, 0xa8, 0x01, 0x09 };
static const u_int8_t BBB_ip[4] = { 0xc0, 0xa8, 0x01, 0x03 };
static const u_int8_t BBB_hwaddr[6] = { 0xc8, 0xa0, 0x30, 0xa7, 0xd0, 0xfc};
static const u_int8_t my_hwaddr[6] = { 0x9a, 0x1f, 0x85, 0x1c, 0x3d, 0x0e};
static const char servername[6] = { 'c', 'r', 'o', 's', 's', '\0' };
static const char filename[19] = { '/', 'h', 'o', 'm', 'e', '/', 'v', 'v', 'u', '/', 'b', 'o', 'o', 't', '/', 'M', 'L', 'O', '\0' };
static const char uboot[21] = { '/', 'h', 'o', 'm', 'e', '/', 'v', 'v', 'u', '/', 'b', 'o', 'o', 't', '/', 'u', 'b', 'o', 'o', 't', '\0' };

static const u_int8_t vendor[] = {
    99, 130, 83, 99,
    1, 4, 255, 255, 255, 0, 
    3, 4, 192, 168, 1, 9, 0xFF
};

void hexDump (char *desc, void *addr, int len);

#endif