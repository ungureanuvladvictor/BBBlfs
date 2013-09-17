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
#include <syslog.h>

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