#ifndef TFTP_H
#define TFTP_H

#include <inttypes.h>

typedef struct tftp {
	uint16_t opcode;
	uint16_t blk_number;
} tftp_data;

void make_tftp_data(tftp_data *tftp, uint16_t opcode, uint16_t blk_number);

#endif