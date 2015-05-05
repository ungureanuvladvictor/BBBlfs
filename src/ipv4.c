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
#include <stdio.h>
#include <stddef.h>
#include "ipv4.h"

uint16_t ip_checksum(const void *buf, size_t hdr_len);

int
inet_aton(cp_arg, addr)
const char *cp_arg;
struct in_addr *addr;
{
	register const u_char *cp = cp_arg;
	register u_long val;
	register int base;
#ifdef WIN32
	register ULONG_PTR n;
#else
	register unsigned long n;
#endif
	register u_char c;
	u_int parts[4];
	register u_int *pp = parts;

	for (;;) {
		/*
		* Collect number up to ``.''.
		* Values are specified as for C:
		* 0x=hex, 0=octal, other=decimal.
		*/
		val = 0; base = 10;
		if (*cp == '0') {
			if (*++cp == 'x' || *cp == 'X')
				base = 16, cp++;
			else
				base = 8;
		}
		while ((c = *cp) != '\0') {
			if (isascii(c) && isdigit(c)) {
				val = (val * base) + (c - '0');
				cp++;
				continue;
			}
			if (base == 16 && isascii(c) && isxdigit(c)) {
				val = (val << 4) +
					(c + 10 - (islower(c) ? 'a' : 'A'));
				cp++;
				continue;
			}
			break;
		}
		if (*cp == '.') {
			/*
			* Internet format:
			*	a.b.c.d
			*	a.b.c	(with c treated as 16-bits)
			*	a.b	(with b treated as 24 bits)
			*/
			if (pp >= parts + 3 || val > 0xff)
				return (0);
			*pp++ = val, cp++;
		}
		else
			break;
	}
	/*
	* Check for trailing characters.
	*/
	if (*cp && (!isascii(*cp) || !isspace(*cp)))
		return (0);
	/*
	* Concoct the address according to
	* the number of parts specified.
	*/
	n = pp - parts + 1;
	switch (n) {

	case 1:				/* a -- 32 bits */
		break;

	case 2:				/* a.b -- 8.24 bits */
		if (val > 0xffffff)
			return (0);
		val |= parts[0] << 24;
		break;

	case 3:				/* a.b.c -- 8.8.16 bits */
		if (val > 0xffff)
			return (0);
		val |= (parts[0] << 24) | (parts[1] << 16);
		break;

	case 4:				/* a.b.c.d -- 8.8.8.8 bits */
		if (val > 0xff)
			return (0);
		val |= (parts[0] << 24) | (parts[1] << 16) | (parts[2] << 8);
		break;
	}
	if (addr)
		addr->s_addr = htonl(val);
	return (1);
}

void make_ipv4(struct iphdr *ip, const char *src_addr, const char *dst_addr,
	uint8_t proto, uint16_t id, size_t total_len) {
	ip->version = 4;
	ip->ihl = 5;
	ip->ttl = 64;
	ip->id = htons(id);
	ip->tot_len = htons(total_len);
	ip->protocol = proto;

	if (inet_aton(src_addr, (struct in_addr *)&ip->saddr) == 0) {
		printf("Cannot add IPv4 src address!\n");
	}

	if (inet_aton(dst_addr, (struct in_addr *)&ip->daddr) == 0) {
		printf("Cannot add IPv4 dst address!\n");
	}

	ip->check = ip_checksum(ip, sizeof(struct iphdr));
}

uint16_t ip_checksum(const void *buf, size_t hdr_len) {
	unsigned long sum = 0;
	const uint16_t *ip1;
	ip1 = buf;

	while (hdr_len > 1) {
		sum += *ip1++;
		if (sum & 0x80000000)
			sum = (sum & 0xFFFF) + (sum >> 16);
		hdr_len -= 2;
	}

	while (sum >> 16)
		sum = (sum & 0xFFFF) + (sum >> 16);

	return(~sum);
}
