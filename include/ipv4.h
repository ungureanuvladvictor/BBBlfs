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

#ifndef IPV4_H
#define IPV4_h

#include "utils.h"

struct iphdr {
#ifdef __linux__
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
	uint8_t		ihl:4,
			version:4;
#else
	uint8_t		version:4,
			ihl:4;
#endif
#endif

#ifdef __APPLE__
#if defined(__LITTLE_ENDIAN__)
	uint8_t		ihl:4,
			version:4;
#else
	uint8_t		version:4,
			ihl:4;
#endif
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

void make_ipv4_packet(struct iphdr *ip, const char *src_addr,
		      const char *dst_addr,
		      uint8_t proto, uint16_t id, ssize_t total_len);
uint16_t ip_checksum(const void *buf, size_t hdr_len);;

#endif

