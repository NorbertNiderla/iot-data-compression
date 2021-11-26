//by Norbert Niderla, 2020
//Sprintz by Blalock

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>

#include "include/sprintz.h"
#include "include/bitstream.h"
#include "include/fire.h"
#include "include/huffman.h"

#define DATA_WIDTH 32
#define DIM 1

void zigzag(int* data, int size){
	for(int idx=0;idx<size;idx++){
		if(data[idx]<0)data[idx] = -2*data[idx]-1;
		else data[idx] = 2*data[idx];
	}
}

static void zigzagDecode(int* data, int size){
	for(int idx=0;idx<size;idx++){
		if(data[idx]%2==1)data[idx]=(data[idx]+1)/(-2);
		else data[idx] = data[idx]/2;
	}
}

static int countSignificantBits(int* data, int size, int offset)
{
	int sb = 0;
	int temp = DATA_WIDTH;
	for (int idx = offset; idx < DIM*size; idx+=DIM){
		if(data[idx]==0) temp = 0;
		else temp = 32 - __builtin_clz(data[idx]);
		if(temp > sb) sb = temp;
	}
	return sb;
}

int sprintzEncode(const int* input, int size, unsigned char* output, size_t output_buffer_size, bool new_huff_table, fire_coder_t* fire_state){
	int* int_buffer = (int*)malloc(DIM*size*sizeof(int));
	unsigned char* char_buffer = (unsigned char*)malloc(DIM*3*size*sizeof(unsigned char));
	int* sb = (int*)malloc(DIM*sizeof(int));

	fireEncode(input,size,int_buffer, fire_state);
	zigzag(int_buffer,DIM*size);
	for(int k = 0; k<DIM; k++) sb[k] = countSignificantBits(int_buffer,size,k);

	bitstream_state_t state;
	bitstream_state_t* state_p = &state;
	bitstream_init(state_p, char_buffer, DIM*3*size);

	for(int k = 0; k<DIM; k++) bitstream_append(state_p, sb[k], 5);
	for(int k = 0; k<DIM; k++){
		for(int x = k; x<DIM*size; x+=DIM){
			bitstream_append(state_p, int_buffer[x],sb[k]);
		}
	}
	bitstream_write_close(state_p);

	int bytes = huffman_encode(char_buffer,state.stream_used_len,output,output_buffer_size, new_huff_table);

	free(int_buffer);
	free(char_buffer);
	free(sb);
	return bytes*8;
}

void sprintzDecode(unsigned char* input, size_t input_buffer_size, int* output, int size, fire_coder_t* fire_state){
	unsigned char* char_buffer = (unsigned char*)malloc(2*size*sizeof(unsigned char));

	huffman_decode(input, input_buffer_size, char_buffer);

	bitstream_state_t state;
	bitstream_state_t* state_p = &state;
	bitstream_init(state_p, char_buffer, 2*size);

	int* int_buffer = (int*)calloc(DIM*size,sizeof(int));
	int* sb = (int*)malloc(DIM*sizeof(int));

	for(int k = 0; k<DIM; k++) bitstream_read_bits_int(state_p,&sb[k],5);
	for(int k = 0; k<DIM; k++){
		for(int x = k; x<DIM*size; x+=DIM){
			bitstream_read_bits_int(state_p, &int_buffer[x],sb[k]);
		}
	}

	zigzagDecode(int_buffer, DIM*size);
	fireDecode(int_buffer,size,output, fire_state);
	free(int_buffer);
	free(char_buffer);
	free(sb);
}
