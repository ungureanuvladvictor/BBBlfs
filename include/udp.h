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

#ifndef UDP_H
#define UDP_H

#include "utils.h"

typedef struct udp {
	uint16_t udpSrc; /*!< Source port of UDP packet */
	uint16_t udpDst; /*!< Destination port of UDP packet */
	uint16_t udpLen; /*!< Length of header + data of the UDP packet */
	uint16_t chkSum; /*!< Checksum of the UDP packet */
} udp_t;

void make_udp_packet(udp_t *udp_packet, uint16_t udp_len,
		     uint16_t srcPort, uint16_t dstPort);

#endif
