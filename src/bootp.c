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

#include "bootp.h"

void make_bootp_packet(const char *servername, const char *filename,
		       bootp_packet *bpp, u_int32_t xid, unsigned char *hw_Dest)
{
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
