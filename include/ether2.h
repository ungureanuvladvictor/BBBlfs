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

#ifndef ETH2_H
#define ETH2_H

#include <stdlib.h>
#include <string.h>
#include <syslog.h>

#define ETH_ALEN	6		/* Octets in one ethernet addr	 */

struct ethhdr {
	unsigned char	h_dest[ETH_ALEN];	/* destination eth addr	*/
	unsigned char	h_source[ETH_ALEN];	/* source ether addr	*/
	unsigned short	h_proto;		/* packet type ID field	*/
} __attribute__((packed));

void make_ether2(struct ethhdr *eth2, u_int8_t dstAddr[], u_int8_t srcAddr[]);
void update_proto_ether2(struct ethhdr *eth2, uint16_t protocol);
void debug_ether2(struct ethhdr *eth2);

#endif
