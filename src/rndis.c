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

#include "rndis.h"

void make_rndis(rndis_hdr *rndishdr, uint32_t data_length) {
	rndishdr->data_len = data_length;
	rndishdr->data_offset = 0x24;
	rndishdr->msg_type = 0x00000001;
	rndishdr->msg_len = data_length + 44;
}

void debug_rndis(rndis_hdr *rndis) {
	syslog(LOG_DEBUG, "STARGING RNDIS LOG");
	syslog(LOG_DEBUG, "RNDIS MsgType: %d", rndis->msg_type);
	syslog(LOG_DEBUG, "RNDIS MsgLen: %d", rndis->msg_len);
	syslog(LOG_DEBUG, "RNDIS Data Offset: %d", rndis->data_offset);
	syslog(LOG_DEBUG, "RNDIS Data Len: %d", rndis->data_len);
	syslog(LOG_DEBUG, "END OF RNDIS LOG");
}
