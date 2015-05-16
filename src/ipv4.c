/*
 * Copyright (C) 2013-2015 Vlad Ungureanu
 * Contact: Vlad Ungureanu <vvu@vdev.ro>
 *
 * This source is subject to the license found in the file 'LICENSE' which must
 * be be distributed together with this source. All other rights reserved.
 *
 * THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
 * EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.
 */

#include "ipv4.h"

void make_ipv4_packet(struct iphdr *ip, const char *src_addr,
		      const char *dst_addr,
		      uint8_t proto, uint16_t id, ssize_t total_len)
{
	ip->version = 4;
	ip->ihl = 5;
	ip->ttl = 64;
	ip->id = htons(id);
	ip->tot_len = htons(total_len);
	ip->protocol = proto;

	if(inet_aton(src_addr, (struct in_addr *)&ip->saddr) == 0) {
		printf("Cannot add IPv4 src address!\n");
		exit(1);
	}

	if(inet_aton(dst_addr, (struct in_addr *)&ip->daddr) == 0) {
		printf("Cannot add IPv4 dst address!\n");
		exit(1);
	}

	ip->check = ip_checksum(ip, sizeof(struct iphdr));
}

uint16_t ip_checksum(const void *buf, size_t hdr_len)
{
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
