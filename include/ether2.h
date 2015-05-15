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

#include <linux/if_ether.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>

void make_ether2(struct ethhdr *eth2, u_int8_t dstAddr[], u_int8_t srcAddr[]);
void update_proto_ether2(struct ethhdr *eth2, uint16_t protocol);
void debug_ether2(struct ethhdr *eth2);