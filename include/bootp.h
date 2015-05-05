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
#include <stdint.h>
#include <WinSock2.h>
#include "utils.h"

typedef uint8_t ipv4addr[4];

typedef struct bootp {
	uint8_t opcode; /*!< 1 for BOOTP_REQUEST, 2 for BOOTP_RESPONSE */
	uint8_t hw; /*!< */
	uint8_t hwlength; /*!< Length of the MAC */
	uint8_t hopcount; /*!< Number of routers to pass until discarded */
	uint32_t xid; /*!< Transaction ID */
	uint16_t secs; /*!< Seconds since boot */
	uint16_t flags; /*!<  */
	ipv4addr ciaddr; /*!< IP addr filled by client in REQUEST if known */
	ipv4addr yiaddr; /*!< IP addr of the client filled by server in RESPONSE */
	ipv4addr server_ip; /*!< IP addr of the BOOTP server */
	ipv4addr bootp_gw_ip; /*!< IP addr of the gateway */
	uint8_t hwaddr[16]; /*!< MAC addr of the client */
	uint8_t servername[64]; /*!< Name of the server */
	uint8_t bootfile[128]; /*!< File to be booted */
	uint8_t vendor[64]; /*!< Vendor extensions(subnet here for this project) */
} bootp_packet;

typedef uint8_t hwaddr_t[6];

void make_bootp(const char *servername, const char *filename,
	bootp_packet *bpp, uint32_t xid, unsigned char *hw_Dest);

#endif