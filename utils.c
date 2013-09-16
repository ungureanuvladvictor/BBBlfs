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