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

#ifndef BOOTP_H
#define BOOTP_H

#include <sys/types.h>
#include "utils.h"

typedef u_int8_t ipv4addr[4];

typedef struct bootp {
    u_int8_t opcode; /*!< 1 for BOOTP_REQUEST, 2 for BOOTP_RESPONSE */
    u_int8_t hw; /*!< */
    u_int8_t hwlength; /*!< Length of the MAC */
    u_int8_t hopcount; /*!< Number of routers to pass until discarded */
    u_int32_t xid; /*!< Transaction ID */
    u_int16_t secs; /*!< Seconds since boot */
    u_int16_t flags; /*!<  */
    ipv4addr ciaddr; /*!< IP address filled by client in REQUEST if known */
    ipv4addr yiaddr; /*!< IP address of the client filled by server in RESPONSE */
    ipv4addr server_ip; /*!< IP address of the BOOTP server */
    ipv4addr bootp_gw_ip; /*!< IP address of the gateway */
    u_int8_t hwaddr[16]; /*!< MAC address of the client */
    u_int8_t servername[64]; /*!< Name of the server */
    u_int8_t bootfile[128]; /*!< File to be booted */
    u_int8_t vendor[64]; /*!< Vendor extensions(subnet here for this project) */
} bootp_packet;

typedef u_int8_t hwaddr_t[6];

void make_bootp(const char *servername, const char *filename, 
                bootp_packet *bpp, u_int32_t xid, unsigned char *hw_Dest);
void debug_bootp(bootp_packet *breq, int breqlen);

#endif
