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

int rndis_send_init(libusb_device_handle *dev_handle, unsigned char *buf,
		    int buflen)
{
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
				int buflen)
{
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

void make_rndis_data_hdr(rndis_data_hdr *rndis_data, uint32_t data_length)
{
	rndis_data->data_len = data_length;
	rndis_data->data_offset = 0x24;
	rndis_data->msg_type = 0x00000001;
	rndis_data->msg_len = data_length + 44;
}

void make_rndis_init_hdr(rndis_init_hdr *rndis_init)
{
	rndis_init->msg_type = cpu_to_le32(RNDIS_MSG_INIT);
	rndis_init->msg_len = cpu_to_le32(0x00000018);
	rndis_init->major_version = cpu_to_le32(1);
	rndis_init->request_id = cpu_to_le32(1);
	rndis_init->minor_version = cpu_to_le32(0);
	rndis_init->max_transfer_size = cpu_to_le32(64);
}

void make_rndis_set_hdr(rndis_set_hdr *rndis_set)
{
	memset(rndis_set, 0, sizeof(rndis_set_hdr));
	rndis_set->msg_type = cpu_to_le32(RNDIS_MSG_SET);
	rndis_set->msg_len = cpu_to_le32(4 + sizeof(rndis_set_hdr));
	rndis_set->oid = cpu_to_le32(RNDIS_OID_GEN_CURRENT_PACKET_FILTER);
	rndis_set->len = cpu_to_le32(4);
	rndis_set->offset = cpu_to_le32(20);
	rndis_set->reserved = 0;
	rndis_set->request_id = 23;

}
