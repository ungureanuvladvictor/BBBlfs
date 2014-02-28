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