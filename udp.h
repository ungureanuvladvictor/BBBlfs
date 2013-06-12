#ifndef UDP_H
#define UDP_H

#include <stdint.h>

typedef struct udp {
	uint16_t udpSrc; /*!< Source port of UDP packet */
	uint16_t udpDst; /*!< Destination port of UDP packet */
	uint16_t udpLen; /*!< Length of header + data of the UDP packet */
	uint16_t chkSum; /*!< Checksum of the UDP packet */
} udp_t;

void make_udp(udp_t *udp_packet, uint16_t udp_len, uint16_t srcPort, uint16_t dstPort);
void debug_udp(udp_t *upd_packet, uint16_t udp_len);

#endif