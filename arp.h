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

#ifndef ARP_H
#define ARP_H

#include <inttypes.h>

typedef struct arp {
	uint16_t htype;
	uint16_t ptype;
	uint8_t hlen;
	uint8_t plen;
	uint16_t opcode;
	uint8_t hw_source[6];
	uint32_t ip_source;
	uint8_t hw_dest[6];
	uint32_t ip_dest;
} __attribute__ ((packed)) arp_hdr;

void make_arp(arp_hdr *arp, uint16_t opcode, const uint8_t hw_source[6], const uint32_t *ip_source, const uint8_t *hw_dest, const uint32_t *ip_dest);

#endif 