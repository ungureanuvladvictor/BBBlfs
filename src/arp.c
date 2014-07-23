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
