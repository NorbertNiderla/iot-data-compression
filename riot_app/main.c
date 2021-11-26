#include <stdio.h>
#include "bitstream.h"

int main(void){
    
    unsigned char buffer[7] = {0};
    bitstream_state_t stream;
    bitstream_init(&stream, buffer, 7);
    printf("Hello World!");
    return 0;
}