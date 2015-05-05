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

#include <string.h>
#include <stdio.h>
#include <inttypes.h>

#include "bootp.h"

void make_bootp(const char *servername, const char *filename,
	bootp_packet *bpp, uint32_t xid, unsigned char *hw_Dest) {
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

	strcpy((char *)&bpp->servername, servername);
	strcpy((char *)&bpp->bootfile, filename);

	memcpy(&bpp->vendor, vendor, 17);
}
