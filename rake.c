//by Norbert Niderla, 2021

//RAKE by Campobello, 2017

#include <stdlib.h>
#include <string.h>

#include "bitstream.h"
#include "include/compression_iot_definitions.h"

#if RAKE_POS_PRINT
#include <stdio.h>
#endif

static inline int compute_binary_log2(int d) {
	int n = 0;
	d = d > 0 ? d : -d;
	while (d > 0) {
		d >>= 1;
		n++;
	}
	return n;
}

static int rake_core_encode(unsigned char *data, int size, unsigned char *output, int output_buffer_size){
    bitstream_state_t stream;
    bitstream_init(&stream, data, size+2);

    bitstream_state_t output_stream;
    bitstream_init(&output_stream, output, output_buffer_size);
    int i = 0;
    int log = 0;
    int pos = 0;

    unsigned long long val = 0;
    while( i < size){
        if(log == 0) {
            i += bitstream_shift_read_bits(&stream, &val, 16);
        } else {
            i += bitstream_shift_read_bits(&stream, &val, 16 - log + 1);
        }
        val &= 0xFFFF;
        log = compute_binary_log2(val);
        pos = 16 - log;
#if RAKE_POS_PRINT
        printf("%d,",pos);
#endif
        if(pos == 16){
            bitstream_append_bits(&output_stream, 0, 1);
        } else {
            bitstream_append_bits(&output_stream, 1, 1);
            bitstream_append_bits(&output_stream, pos, 4);
        }
    }
    bitstream_write_close(&output_stream);
    return(output_stream.stream_used_len);
}

#if ENCODER_DC_VALUE
int rake_encode(int16_t* data, int size, unsigned char* output, int output_buffer_size, int dc_value){
    for (int i = (size - 1); i > 0; i--) data[i] -= data[i - 1];
    data[0] -= dc_value;
    for(int i=0;i<size;i++) data[i] = data[i]<0 ? -2*data[i]-1 : 2*data[i];
    int bytes = rake_core_encode((unsigned char*)data, size*2, output, output_buffer_size);
    return(bytes);
}
#else
int rake_encode(int16_t* data, int size, unsigned char* output, int output_buffer_size){
	for (int i = (size - 1); i > 0; i--) data[i] -= data[i - 1];
	    for(int i=0;i<size;i++) data[i] = data[i]<0 ? -2*data[i]-1 : 2*data[i];
	    int bytes = rake_core_encode((unsigned char*)data, size*2, output, output_buffer_size);
	    return(bytes);
}
#endif

static int rake_core_decode(unsigned char *input, int input_size, unsigned char* output, int output_buffer_size) {
    bitstream_state_t stream;
    bitstream_init(&stream, input, input_size);

    bitstream_state_t output_stream;
    bitstream_init(&output_stream, output, output_buffer_size);

    int i = 0;
    unsigned long long val = 0;
    while(i < input_size){
        i += bitstream_read_bits(&stream, &val, 1);
        if(val == 1){
            i += bitstream_read_bits(&stream, &val, 4);
            bitstream_append_bits(&output_stream, 0, val);
            bitstream_append_bits(&output_stream, 1, 1);
        } else {
            bitstream_append_bits(&output_stream, 0, 16);
        }
    }

    return(output_stream.stream_used_len);
}

void rake_decode(unsigned char* input, int input_size, int16_t* output, int output_size){
    rake_core_decode(input, input_size, (unsigned char*)output, 2*output_size);
    for(int i=0;i<output_size;i++) output[i]= output[i]%2==1 ? (output[i]+1)/(-2) : output[i]/2;
    for(int i = 1; i < output_size; i++) output[i] += output[i-1];
}
