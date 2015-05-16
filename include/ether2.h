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

#ifndef ETH2_H
#define ETH2_H

#include "utils.h"

#define ETH_ALEN	6			/* Octets in one ethernet addr	 */

struct ethhdr {
	unsigned char	h_dest[ETH_ALEN];	/* destination eth addr	*/
	unsigned char	h_source[ETH_ALEN];	/* source ether addr	*/
	unsigned short	h_proto;		/* packet type ID field	*/
} __attribute__((packed));

void make_ether2_packet(struct ethhdr *eth2, u_int8_t dstAddr[],
			u_int8_t srcAddr[]);
void update_proto_ether2(struct ethhdr *eth2, uint16_t protocol);

#endif

