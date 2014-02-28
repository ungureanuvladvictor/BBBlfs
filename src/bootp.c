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

#include <syslog.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <inttypes.h>

#include "../includes/bootp.h"
#include "../includes/utils.h"

void make_bootp(const char *servername, const char *filename,
                bootp_packet *bpp, u_int32_t xid, unsigned char *hw_Dest) {
    bpp->opcode = 2;
    bpp->hw = 1;
    bpp->hwlength = 6;
    bpp->hopcount = 0;

    bpp->xid = htonl(xid);
    bpp->secs = htons(0);
    bpp->flags = htons(0);

    memcpy(bpp->yiaddr, bootp_BBB_ip, 4);
    memcpy(bpp->server_ip, bootp_server_ip, 4);
    memcpy(bpp->bootp_gw_ip, server_ip, 4);
    memcpy(bpp->hwaddr, hw_Dest, 6);

    strncpy((char *)&bpp->servername, servername, sizeof(bpp->servername));
    strncpy((char *)&bpp->bootfile, filename, sizeof(bpp->bootfile));

    memcpy(&bpp->vendor, vendor, 17);
}

void debug_bootp(bootp_packet *breq, int breqlen) {
    char vendor[100];
    struct in_addr address;
    u_int8_t *vndptr;

    memset(&address, 0, sizeof(address));

    syslog (LOG_DEBUG, "opcode: %i", breq->opcode);
    syslog (LOG_DEBUG, "hw: %i", breq->hw);
    syslog (LOG_DEBUG, "hwlength: %i", breq->hwlength);
    syslog (LOG_DEBUG, "hopcount: %i", breq->hopcount);
    syslog (LOG_DEBUG, "xid: 0x%08x", breq->xid);
    syslog (LOG_DEBUG, "secs: %i", breq->secs);
    syslog (LOG_DEBUG, "flags: 0x%04x", breq->flags);

    memcpy(&address.s_addr, breq->ciaddr, 4);
    syslog (LOG_DEBUG, "ciaddr: %s", inet_ntoa (address));

    memcpy(&address.s_addr, breq->yiaddr, 4);
    syslog (LOG_DEBUG, "yiaddr: %s", inet_ntoa (address));

    memcpy(&address.s_addr, breq->server_ip, 4);
    syslog (LOG_DEBUG, "server_ip: %s", inet_ntoa (address));

    memcpy(&address.s_addr, breq->bootp_gw_ip, 4);
    syslog (LOG_DEBUG, "bootp_gw_ip: %s", inet_ntoa (address));

    syslog (LOG_DEBUG, "hwaddr: %02X:%02X:%02X:%02X:%02X:%02X",
            breq->hwaddr[0], breq->hwaddr[1], breq->hwaddr[2],
            breq->hwaddr[3], breq->hwaddr[4], breq->hwaddr[5]);

    syslog (LOG_DEBUG, "servername: %s", breq->servername);
    syslog (LOG_DEBUG, "bootfile: %s", breq->bootfile);

    vndptr = breq->vendor;
    sprintf (vendor, "Magic cookie:%d.%d.%d.%d \
        ID: %d Length %d Subnet: %d.%d.%d.%d END: 0x%2X",
        *vndptr, *(vndptr + 1), *(vndptr + 2), *(vndptr + 3), *(vndptr + 4),
         *(vndptr + 5), *(vndptr + 6), *(vndptr + 7), *(vndptr + 8),
          *(vndptr + 9), *(vndptr + 10));

    syslog (LOG_DEBUG, "vendor: %s", vendor);
    syslog(LOG_DEBUG, "END DEBUG!");
}
