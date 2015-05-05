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

#ifndef RNDIS_H
#define RNDIS_H

#include <stdint.h>
#include "libusb.h"

#if defined(__BIG_ENDIAN__) || defined(_BIG_ENDIAN)

#define le32_to_cpu(x) (\
	(((x)>>24)&0xff)\
	|\
	(((x)>>8)&0xff00)\
	|\
	(((x)<<8)&0xff0000)\
	|\
	(((x)<<24)&0xff000000)\
)

#define le16_to_cpu(x) ( (((x)>>8)&0xff) | (((x)<<8)&0xff00) )

#else

#define le32_to_cpu(x) (x)
#define le16_to_cpu(x) (x)

#endif

#define cpu_to_le32(x) le32_to_cpu(x)
#define cpu_to_le16(x) le16_to_cpu(x)

#define RNDIS_MSG_INIT		0x00000002
#define USB_TYPE_CLASS                     (0x01 << 5)
#define USB_CDC_SEND_ENCAPSULATED_COMMAND  0x00
#define USB_RECIP_INTERFACE                0x01
#define USB_DIR_IN                 0x80
#define USB_CDC_GET_ENCAPSULATED_RESPONSE  0x01
#define USB_RECIP_INTERFACE                0x01

/* MS-Windows uses this strange size, but RNDIS spec says 1024 minimum */
#define	CONTROL_BUFFER_SIZE		1025
#define RNDIS_MSG_SET		0x00000005
/* RNDIS defines an (absurdly huge) 10 second control timeout,
* but ActiveSync seems to use a more usual 5 second timeout
* (which matches the USB 2.0 spec).
*/
#define	RNDIS_CONTROL_TIMEOUT_MS	(1000)
#define RNDIS_OID_GEN_CURRENT_PACKET_FILTER	0x0001010E
/* default filter used with RNDIS devices */
/* packet filter bits used by RNDIS_OID_GEN_CURRENT_PACKET_FILTER */
#define RNDIS_PACKET_TYPE_DIRECTED		0x00000001
#define RNDIS_PACKET_TYPE_MULTICAST		0x00000002
#define RNDIS_PACKET_TYPE_ALL_MULTICAST		0x00000004
#define RNDIS_PACKET_TYPE_BROADCAST		0x00000008
#define RNDIS_PACKET_TYPE_SOURCE_ROUTING	0x00000010
#define RNDIS_PACKET_TYPE_PROMISCUOUS		0x00000020
#define RNDIS_PACKET_TYPE_SMT			0x00000040
#define RNDIS_PACKET_TYPE_ALL_LOCAL		0x00000080
#define RNDIS_PACKET_TYPE_GROUP			0x00001000
#define RNDIS_PACKET_TYPE_ALL_FUNCTIONAL	0x00002000
#define RNDIS_PACKET_TYPE_FUNCTIONAL		0x00004000
#define RNDIS_PACKET_TYPE_MAC_FRAME		0x00008000
#define RNDIS_DEFAULT_FILTER ( \
	RNDIS_PACKET_TYPE_DIRECTED | \
	RNDIS_PACKET_TYPE_BROADCAST | \
	RNDIS_PACKET_TYPE_ALL_MULTICAST | \
	RNDIS_PACKET_TYPE_PROMISCUOUS)
typedef struct {
	uint32_t msg_type; /*!< Always 1*/
	uint32_t msg_len; /*!< Length of headear + data + payload*/
	uint32_t data_offset; /*!< Offset from data until payload*/
	uint32_t data_len; /*!< Length of payload*/
	uint32_t band_offset; /*!< Not used in this project*/
	uint32_t band_len; /*!< Not used in this project*/
	uint32_t out_band_elements; /*!< Not used in this project*/
	uint32_t packet_offset; /*!< Not used in this project*/
	uint32_t packet_info_len; /*!< Not used in this project*/
	uint32_t reserved_first; /*!< Not used in this project*/
	uint32_t reserved_second; /*!< Not used in this project*/
} rndis_data_hdr;

typedef struct {
	uint32_t msg_type;
	uint32_t msg_len;
	uint32_t request_id;
	uint32_t major_version;
	uint32_t minor_version;
	uint32_t max_transfer_size;
} rndis_init_hdr;

typedef struct {
	uint32_t msg_type;
	uint32_t msg_len;
	uint32_t request_id;
	uint32_t oid;
	uint32_t len;
	uint32_t offset;
	uint32_t reserved;
} rndis_set_hdr;

void make_rndis_data_hdr(rndis_data_hdr *rndis_data, uint32_t data_length);
void make_rndis_init_hdr(rndis_init_hdr *rndis_init);
void make_rndis_set_hdr(rndis_set_hdr *rndis_set);
int rndis_send_init(libusb_device_handle *dev_handle, unsigned char *buf, int buflen);
int rndis_send_filter(libusb_device_handle *dev_handle, unsigned char *buf, int buflen);

#endif
