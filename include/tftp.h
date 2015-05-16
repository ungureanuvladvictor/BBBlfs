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

#ifndef TFTP_H
#define TFTP_H

#include "utils.h"

#define TFTP_DATA 	3

typedef struct tftp {
	uint16_t opcode;
	uint16_t blk_number;
} tftp_data;

void make_tftp_data(tftp_data *tftp, uint16_t opcode, uint16_t blk_number);

#endif