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
#include "utils.h"

int rndis_send_init(libusb_device_handle *dev_handle, unsigned char *buf,
					int buflen) {
	int r = 0;
	r = libusb_control_transfer(dev_handle,
								USB_TYPE_CLASS | USB_RECIP_INTERFACE,
								USB_CDC_SEND_ENCAPSULATED_COMMAND, 0, 0, buf,
								buflen, RNDIS_CONTROL_TIMEOUT_MS);
	if (r < 0) {
		return r;
	}

	r = libusb_control_transfer(dev_handle,
								USB_DIR_IN | USB_TYPE_CLASS |
								USB_RECIP_INTERFACE,
								USB_CDC_GET_ENCAPSULATED_RESPONSE, 0, 0, buf,
								CONTROL_BUFFER_SIZE, RNDIS_CONTROL_TIMEOUT_MS);
	if (r < 0) {
		return r;
	}

	return r;
}
int rndis_send_filter(libusb_device_handle *dev_handle, unsigned char *buf,
						int buflen) {
	int r = 0;
	r = libusb_control_transfer(dev_handle, USB_TYPE_CLASS |
								USB_RECIP_INTERFACE,
								USB_CDC_SEND_ENCAPSULATED_COMMAND, 0, 0, buf,
								buflen, RNDIS_CONTROL_TIMEOUT_MS);
	if (r < 0) {
		return r;
	}

	r = libusb_control_transfer(dev_handle, USB_DIR_IN | USB_TYPE_CLASS |
								USB_RECIP_INTERFACE,
								USB_CDC_GET_ENCAPSULATED_RESPONSE, 0, 0, buf,
								CONTROL_BUFFER_SIZE, RNDIS_CONTROL_TIMEOUT_MS);
	if (r < 0) {
		return r;
	}

	return r;
}

void make_rndis_data_hdr(rndis_data_hdr *rndis_data, uint32_t data_length) {
	rndis_data->data_len = data_length;
	rndis_data->data_offset = 0x24;
	rndis_data->msg_type = 0x00000001;
	rndis_data->msg_len = data_length + 44;
}

void make_rndis_init_hdr(rndis_init_hdr *rndis_init) {
	rndis_init->msg_type = cpu_to_le32(RNDIS_MSG_INIT);
	rndis_init->msg_len = cpu_to_le32(0x00000018);
	rndis_init->major_version = cpu_to_le32(1);
	rndis_init->request_id = cpu_to_le32(1);
	rndis_init->minor_version = cpu_to_le32(0);
	rndis_init->max_transfer_size = cpu_to_le32(64);
}

void make_rndis_set_hdr(rndis_set_hdr *rndis_set) {
	memset(rndis_set, 0, sizeof(rndis_set_hdr));
	rndis_set->msg_type = cpu_to_le32(RNDIS_MSG_SET);
	rndis_set->msg_len = cpu_to_le32(4 + sizeof(rndis_set_hdr));
	rndis_set->oid = cpu_to_le32(RNDIS_OID_GEN_CURRENT_PACKET_FILTER);
	rndis_set->len = cpu_to_le32(4);
	rndis_set->offset = cpu_to_le32(20);
	rndis_set->reserved = 0;
	rndis_set->request_id = 23;

}

void debug_rndis(rndis_data_hdr *rndis) {
	syslog(LOG_DEBUG, "STARGING RNDIS LOG");
	syslog(LOG_DEBUG, "RNDIS MsgType: %d", rndis->msg_type);
	syslog(LOG_DEBUG, "RNDIS MsgLen: %d", rndis->msg_len);
	syslog(LOG_DEBUG, "RNDIS Data Offset: %d", rndis->data_offset);
	syslog(LOG_DEBUG, "RNDIS Data Len: %d", rndis->data_len);
	syslog(LOG_DEBUG, "END OF RNDIS LOG");
}
