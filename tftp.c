#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <arpa/inet.h>

#include "tftp.h"

void make_tftp_data(tftp_data *tftp, uint16_t opcode, uint16_t blk_number) {
	tftp->opcode = ntohs(opcode);
	tftp->blk_number = ntohs(blk_number);
}

