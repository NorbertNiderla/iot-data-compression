#include "../../../../compression_iot/archive/source/board_bits_2_dim/sprintz.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>

#include "../../../../compression_iot/archive/source/board_bits_2_dim/bitstream.h"
#include "../../../../compression_iot/archive/source/board_bits_2_dim/fire.h"
#include "../../../../compression_iot/archive/source/board_bits_2_dim/huffman.h"
#include "../../../../compression_iot/archive/source/board_bits_2_dim/my_lib.h"

#define DATA_WIDTH 32
#define DIM 2

static int fire_bits = 0;
static int bit_packing_bits = 0;

static int hist[256] = {0};

int getFireBits(void){
	int temp = fire_bits;
	fire_bits = 0;
	return temp;
}

int getBitPackingBits(void){
	int temp = bit_packing_bits;
	bit_packing_bits = 0;
	return temp;
}

static void zigzag(int* data, int size){
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

int sprintzEncode(const int* input, int size, unsigned char* output, size_t output_buffer_size, bool new_huff_table){
	int* int_buffer = (int*)malloc(DIM*size*sizeof(int));
	unsigned char* char_buffer = (unsigned char*)malloc(DIM*2*size*sizeof(unsigned char));
	int* sb = (int*)malloc(DIM*sizeof(int));

	fireEncode(input,size,int_buffer);

	//liczenie bitów
		for(int i = 0;i<DIM*size;i++) fire_bits += (int)(32 - __builtin_clz(fabs(int_buffer[i]))+1);

	zigzag(int_buffer,DIM*size);



	for(int k = 0; k<DIM; k++) sb[k] = countSignificantBits(int_buffer,size,k);

	bitstream_state_t state;
	bitstream_state_t* state_p = &state;
	bitstream_init(state_p, char_buffer, DIM*2*size);

	int bytes = 0;

	for(int k = 0; k<DIM; k++) bytes += bitstream_append_bits(state_p,(unsigned long long)sb[k], 5);
	for(int k = 0; k<DIM; k++){
		for(int x = k; x<DIM*size; x+=DIM){
			bytes += bitstream_append_bits(state_p, (unsigned long long)int_buffer[x],sb[k]);
		}
	}
	bytes += bitstream_write_close(state_p);

	bit_packing_bits += bytes*8; //liczenie bitow
	for(int i =0;i<bytes;i++) hist[char_buffer[i]]++;


	bytes = huffmanEncodeWithTable(char_buffer,bytes,output,output_buffer_size, new_huff_table);
	free(int_buffer);
	free(char_buffer);
	free(sb);
	return bytes;
}

void sprintzDecode(unsigned char* input, size_t input_buffer_size, int* output, int size, int bytes){
	unsigned char* char_buffer = (unsigned char*)malloc(bytes*sizeof(unsigned char));

	huffmanDecode(input, input_buffer_size, char_buffer, bytes);

	bitstream_state_t state;
	bitstream_state_t* state_p = &state;
	bitstream_init(state_p, char_buffer, bytes);

	int* int_buffer = (int*)calloc(DIM*size,sizeof(int));
	int* sb = (int*)malloc(DIM*sizeof(int));

	for(int k = 0; k<DIM; k++) bitstream_read_bits_int(state_p,&sb[k],5);
	for(int k = 0; k<DIM; k++){
		for(int x = k; x<DIM*size; x+=DIM){
			bitstream_read_bits_int(state_p, &int_buffer[x],sb[k]);
		}
	}

	zigzagDecode(int_buffer, DIM*size);
	fireDecode(int_buffer,size,output);
	free(int_buffer);
	free(char_buffer);
	free(sb);
}

void printSprintzHist(void){
	for(int i =0;i<256;i++)printf("%d\n", hist[i]);
}
