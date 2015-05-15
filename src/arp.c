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

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <syslog.h>
#include <arpa/inet.h>

#include "arp.h"


void make_arp(arp_hdr *arp, uint16_t opcode, const uint8_t hw_source[6],
                const uint32_t *ip_source, const uint8_t *hw_dest,
                const uint32_t *ip_dest) {
	arp->htype = htons(1);
	arp->ptype = htons(0x0800);
	arp->hlen = 6;
	arp->plen = 4;
	arp->opcode = htons(opcode);
	memcpy(&arp->hw_source, hw_source, 6);
	memcpy(&arp->ip_source, ip_source, 4);
	memcpy(&arp->hw_dest, hw_dest, 6);
	memcpy(&arp->ip_dest, ip_dest, 4);
}
