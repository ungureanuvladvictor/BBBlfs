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

#ifndef RNDIS_H
#define RNDIS_H

#include "utils.h"

#define RNDIS_MSG_INIT				0x00000002
#define USB_TYPE_CLASS                     	(0x01 << 5)
#define USB_CDC_SEND_ENCAPSULATED_COMMAND  	0x00
#define USB_RECIP_INTERFACE                	0x01
#define USB_DIR_IN				0x80
#define USB_CDC_GET_ENCAPSULATED_RESPONSE  	0x01
#define USB_RECIP_INTERFACE                	0x01

#define	CONTROL_BUFFER_SIZE			1025
#define RNDIS_MSG_SET				0x00000005
#define	RNDIS_CONTROL_TIMEOUT_MS		1000
#define RNDIS_OID_GEN_CURRENT_PACKET_FILTER	0x0001010E
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
} rndis_hdr;

void make_rndis(rndis_hdr *rndishdr, uint32_t data_length);
void debug_rndis(rndis_hdr *rndis);

#endif