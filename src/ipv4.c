/*
 * Copyright 2013 Vlad V. Ungureanu <ungureanuvladvictor@gmail.com>.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this Github repository and wiki except in
 * compliance with the License. You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#ifdef __APPLE__
#include <netinet/ip.h>
typedef struct ip iphdr_t;
#else
#include <linux/ip.h>
typedef struct iphdr iphdr_t;
#endif
#include <syslog.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <arpa/inet.h>

#include "udp.h"
#include "bootp.h"

uint16_t ip_checksum(const void *buf, size_t hdr_len);

void make_ipv4(iphdr_t *ip, char *src_addr, char *dst_addr,
                uint8_t proto, uint16_t id, ssize_t total_len) {
#ifdef __APPLE__
	ip->ip_v = 4;
	ip->ip_hl = 5;
	ip->ip_ttl = 64;
	ip->ip_id = htons(id);
	ip->ip_len = htons(total_len);
	ip->ip_p = proto;

	if(inet_aton(src_addr, (struct in_addr *)&ip->ip_src) == 0) {
		printf("Cannot add IPv4 src address!\n");
	}

	if(inet_aton(dst_addr, (struct in_addr *)&ip->ip_dst) == 0) {
		printf("Cannot add IPv4 dst address!\n");
	}

	ip->ip_sum = ip_checksum(ip, sizeof(iphdr_t));
#else
	ip->version = 4;
	ip->ihl = 5;
	ip->ttl = 64;
	ip->id = htons(id);
	ip->tot_len = htons(total_len);
	ip->protocol = proto;

	if(inet_aton(src_addr, (struct in_addr *)&ip->saddr) == 0) {
		printf("Cannot add IPv4 src address!\n");
	}

	if(inet_aton(dst_addr, (struct in_addr *)&ip->daddr) == 0) {
		printf("Cannot add IPv4 dst address!\n");
	}

	ip->check = ip_checksum(ip, sizeof(iphdr_t));
#endif
}

uint16_t ip_checksum(const void *buf, size_t hdr_len) {
	unsigned long sum = 0;
	const uint16_t *ip1;
	ip1 = buf;

	while(hdr_len > 1) {
		sum += *ip1++;
		if (sum & 0x80000000)
			sum = (sum & 0xFFFF) + (sum >> 16);
		hdr_len -= 2;
	}

	while (sum >> 16)
		sum = (sum & 0xFFFF) + (sum >> 16);

	return(~sum);
}

void debug_ipv4(iphdr_t *ip) {
	syslog(LOG_DEBUG, "STARTING IPV4 LOG");
#ifdef __APPLE__
	syslog(LOG_DEBUG, "IP Version: %d", ip->ip_v);
	syslog(LOG_DEBUG, "IP Header Length: %d", ip->ip_hl);
	syslog(LOG_DEBUG, "IP ID: %d", ip->ip_id);
	syslog(LOG_DEBUG, "IP Total Length: %d", ntohs(ip->ip_len));
	syslog(LOG_DEBUG, "IP Protocol: %d", ip->ip_p);
	syslog(LOG_DEBUG, "IP TTL: %d", ip->ip_ttl);
	syslog(LOG_DEBUG, "IP SrcAddr: %s", inet_ntoa(*(struct in_addr*)&ip->ip_src));
	syslog(LOG_DEBUG, "IP DstAddr: %s", inet_ntoa(*(struct in_addr*)&ip->ip_dst));
	syslog(LOG_DEBUG, "IP CheckSum: %dd", ip->ip_sum);
#else
	syslog(LOG_DEBUG, "IP Version: %d", ip->version);
	syslog(LOG_DEBUG, "IP Header Length: %d", ip->ihl);
	syslog(LOG_DEBUG, "IP ID: %d", ip->id);
	syslog(LOG_DEBUG, "IP Total Length: %d", ntohs(ip->tot_len));
	syslog(LOG_DEBUG, "IP Protocol: %d", ip->protocol);
	syslog(LOG_DEBUG, "IP TTL: %d", ip->ttl);
	syslog(LOG_DEBUG, "IP SrcAddr: %s", inet_ntoa(*(struct in_addr*)&ip->saddr));
	syslog(LOG_DEBUG, "IP DstAddr: %s", inet_ntoa(*(struct in_addr*)&ip->daddr));
	syslog(LOG_DEBUG, "IP CheckSum: %dd", ip->check);
#endif
	syslog(LOG_DEBUG, "END OF IPV4 LOG");
}
