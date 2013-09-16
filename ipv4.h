#include <linux/ip.h>
#include <syslog.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <arpa/inet.h>
      
#include "udp.h"
#include "bootp.h"

void make_ipv4(struct iphdr *ip, const char *src_addr, const char *dst_addr, uint8_t proto, uint16_t id, ssize_t total_len);
uint16_t ip_checksum(const void *buf, size_t hdr_len);;
void debug_ipv4(struct iphdr *ip);

