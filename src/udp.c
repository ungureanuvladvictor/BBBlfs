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

#include "udp.h"

void make_udp_packet(udp_t *udp_packet, uint16_t udp_len,
		     uint16_t srcPort, uint16_t dstPort) {
	memset(&udp_packet->chkSum, 0, sizeof(udp_packet->chkSum));
	memset(&udp_packet->udpLen, 0, sizeof(udp_packet->udpLen));

	udp_packet->udpSrc = srcPort;
	udp_packet->udpDst = dstPort;

	udp_packet->udpLen += htons(8 + udp_len);
}
