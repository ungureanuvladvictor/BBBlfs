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

#include "ether2.h"

void make_ether2_packet(struct ethhdr *eth2, unsigned char *h_dest,
			unsigned char *h_source)
{
	memcpy(&eth2->h_dest, h_dest, 6);
	memcpy(&eth2->h_source, h_source, 6);
	eth2->h_proto = htons(0x0800);
}

void update_proto_ether2(struct ethhdr *eth2, uint16_t protocol)
{
	eth2->h_proto = htons(protocol);
}
