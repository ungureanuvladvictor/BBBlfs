#include <linux/if_ether.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>

void make_ether2(struct ethhdr *eth2, u_int8_t dstAddr[], u_int8_t srcAddr[]);
void debug_ether2(struct ethhdr *eth2);