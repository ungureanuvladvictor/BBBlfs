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

#include <linux/ip.h>
#include <syslog.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <arpa/inet.h>

#include "udp.h"
#include "bootp.h"

void make_ipv4(struct iphdr *ip, const char *src_addr, const char *dst_addr,
                uint8_t proto, uint16_t id, ssize_t total_len);
uint16_t ip_checksum(const void *buf, size_t hdr_len);;
void debug_ipv4(struct iphdr *ip);
