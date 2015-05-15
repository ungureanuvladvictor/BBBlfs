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

#include <stdint.h>

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