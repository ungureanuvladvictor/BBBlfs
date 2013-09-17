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

#include <string.h>
#include <stdio.h>

void hexDump(char *desc, void *addr, int len) {
    int i;
    unsigned char buff[17];
    unsigned char *pc = addr;

    if(desc != NULL)
        printf("%s:\n", desc);

    for(i=0; i<len; i++) {
        if(i!=0 && i%8==0)printf(" ");
        if((i % 16) == 0) {
            if(i != 0)
                printf("  %s\n", buff);

            printf("  %04x ", i);
        }

        printf(" %02x", pc[i]);

        if((pc[i] < 0x20) || (pc[i] > 0x7e))
            buff[i % 16] = '.';
        else
            buff[i % 16] = pc[i];
        buff[(i % 16) + 1] = '\0';
    }

    while((i % 16) != 0) {
        printf("   ");
        i++;
    }

    printf("  %s\n", buff);
}

const char *get_filename_ext(const char *filename) {
    const char *dot = strrchr(filename, '.');
    if(!dot || dot == filename) return "";
    return dot + 1;
}