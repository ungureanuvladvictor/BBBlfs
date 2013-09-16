#include <linux/if_ether.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <arpa/inet.h>

#include "ether2.h"

void make_ether2(struct ethhdr *eth2, unsigned char	*h_dest, unsigned char *h_source) {
	memcpy(&eth2->h_dest, h_dest, 6);
	memcpy(&eth2->h_source, h_source, 6);
	eth2->h_proto = htons(0x0800);
}

void update_proto_ether2(struct ethhdr *eth2, uint16_t protocol) {
	eth2->h_proto = htons(protocol);
}

void debug_ether2(struct ethhdr *eth2) {
	syslog(LOG_DEBUG, "STARTING ETHER2 LOG");
	syslog (LOG_DEBUG, "DstHWAddr: %02X:%02X:%02X:%02X:%02X:%02X", 
            eth2->h_dest[0], eth2->h_dest[1], eth2->h_dest[2],
            eth2->h_dest[3], eth2->h_dest[4], eth2->h_dest[5]);
	syslog (LOG_DEBUG, "SrcHWAddr: %02X:%02X:%02X:%02X:%02X:%02X", 
            eth2->h_source[0], eth2->h_source[1], eth2->h_source[2],
            eth2->h_source[3], eth2->h_source[4], eth2->h_source[5]);
	syslog(LOG_DEBUG, "Protocol: %d", eth2->h_proto);
	syslog(LOG_DEBUG, "ETHER2 LOG ENDED");
}