#ifndef ARP_H
#define ARP_H

#include <inttypes.h>

typedef struct arp {
	uint16_t htype;
	uint16_t ptype;
	uint8_t hlen;
	uint8_t plen;
	uint16_t opcode;
	uint8_t hw_source[6];
	uint32_t ip_source;
	uint8_t hw_dest[6];
	uint32_t ip_dest;
} __attribute__ ((packed)) arp_hdr;

void make_arp(arp_hdr *arp, uint16_t opcode, uint8_t hw_source[6], char *ip_source, uint8_t hw_dest[6], char *ip_dest);

#endif 