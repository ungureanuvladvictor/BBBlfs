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

#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <libusb.h>
#include <inttypes.h>
#include <sys/types.h>
#include <sys/stat.h>

#if defined __linux__ || defined __APPLE__
#include <arpa/inet.h>
#endif

/*Used to suppressed the unused warnings*/
#ifdef __GNUC__
#define UNUSED __attribute__ ((unused))
#else
#define UNUSED
#endif

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

void hexDump (char *desc, void *addr, int len);

#endif

