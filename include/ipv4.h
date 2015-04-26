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

#ifndef IPV4_H
#define IPV4_H

#include <syslog.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <arpa/inet.h>

#include "udp.h"
#include "bootp.h"

struct iphdr {
#if defined(__LITTLE_ENDIAN__)
	uint8_t		ihl:4,
				version:4;
#else
	uint8_t		version:4,
				ihl:4;
#endif
	uint8_t		tos;
	uint16_t	tot_len;
	uint16_t	id;
	uint16_t	frag_off;
	uint8_t		ttl;
	uint8_t		protocol;
	uint16_t	check;
	uint32_t	saddr;
	uint32_t	daddr;
};

void make_ipv4(struct iphdr *ip, const char *src_addr, const char *dst_addr,
                uint8_t proto, uint16_t id, ssize_t total_len);
uint16_t ip_checksum(const void *buf, size_t hdr_len);;
void debug_ipv4(struct iphdr *ip);

#endif
