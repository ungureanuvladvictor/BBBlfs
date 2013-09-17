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


void make_arp(arp_hdr *arp, uint16_t opcode, const uint8_t hw_source[6], const uint32_t *ip_source, const uint8_t *hw_dest, const uint32_t *ip_dest) {
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

/*void debug_arp(arp_hdr *arp) {
	syslog(LOG_DEBUG, "STARTING ARP LOG!");
	syslog(LOG_DEBUG, "ARP htype: %d", ntohs(arp->htype));
	syslog(LOG_DEBUG, "APR ptype: %d", ntohs(arp->ptype));
	syslog(LOG_DEBUG, "ARP hlen:  %d", arp->hlen);
	syslog(LOG_DEBUG, "ARP plen: %d", arp->plen);
	syslog(LOG_DEBUG, "ARP opcode: %d", ntohs(arp->opcode));
	syslog(LOG_DEBUG, "ARP hw_source %02X:%02X:%02X:%02X:%02X:%02X", 
            arp->hw_source[0], arp->hw_source[1], arp->hw_source[2],
            arp->hw_source[3], arp->hw_source[4], arp->hw_source[5]);
	syslog(LOG_DEBUG, "ARP ip_source: %s", inet_ntoa(*(struct in_addr*)&arp->ip_source));
	syslog(LOG_DEBUG, "ARP hw_dest %02X:%02X:%02X:%02X:%02X:%02X", 
        arp->hw_dest[0], arp->hw_dest[1], arp->hw_dest[2],
        arp->hw_dest[3], arp->hw_dest[4], arp->hw_dest[5]);
	syslog(LOG_DEBUG, "ARP ip_dest: %s", inet_ntoa(*(struct in_addr*)&arp->ip_dest));
	syslog(LOG_DEBUG, "END OF ARP LOG!");
}*/