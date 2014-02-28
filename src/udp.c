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
#include <stdio.h>
#include <arpa/inet.h>
#include <stdint.h>
#include <string.h>
#include <syslog.h>
#include <inttypes.h>

#include "../includes/udp.h"

void make_udp(udp_t *udp_packet, uint16_t udp_len,
			uint16_t srcPort, uint16_t dstPort) {
	memset(&udp_packet->chkSum, 0, sizeof(udp_packet->chkSum));
	memset(&udp_packet->udpLen, 0, sizeof(udp_packet->udpLen));

	udp_packet->udpSrc = htons(srcPort);
	udp_packet->udpDst = htons(dstPort);

	udp_packet->udpLen += htons(8 + udp_len);
}

void debug_udp(udp_t *udp_packet) {
	syslog(LOG_DEBUG, "Starting UDP Log");
	syslog(LOG_DEBUG, "Src Port: %d", ntohs(udp_packet->udpSrc));
	syslog(LOG_DEBUG, "Dst Port: %d", ntohs(udp_packet->udpDst));
	syslog(LOG_DEBUG, "Length: %d", udp_packet->udpLen);
	syslog(LOG_DEBUG, "CheckSum: %d", udp_packet->chkSum);
	syslog(LOG_DEBUG, "End of UDP Log");
}
