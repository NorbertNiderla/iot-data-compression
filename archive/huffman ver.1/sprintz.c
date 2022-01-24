
#define TIME_SERIES_LENGTH 8 
#define D 3 
#define RLE_DEPTH 2
#define HEADER_SECTION_BITS 5
#define NUMBER_OF_SETS_BITS 4

#include <stdio.h> 
#include <stdlib.h> 
#include <math.h> 
#include <stdint.h> 
#include <string.h>

#include "../../../compression_iot/archive/huffman ver.1/bitstream.h"
#include "../../../compression_iot/archive/huffman ver.1/sprintz.h"
#include "../../../compression_iot/archive/huffman ver.1/huffman.h"

static int* simpleForecasting(uint32_t* data, const float a, const float b)
{
	int* output = (int*)calloc(D * TIME_SERIES_LENGTH, sizeof(int));
	static unsigned int* prev_sample = NULL;
	if (prev_sample == NULL) prev_sample = (unsigned int*)calloc(2*D,sizeof(int));

	for (int i = 0; i < D; i++)
	{
		*(output + i) = (int)(*(prev_sample + i + D) * a + *(prev_sample + i) * b);
		*(output + D + i) = (int)(*(data + i) * a + *(prev_sample + D + i) * b);
	}
	
	for (int i = 2; i < TIME_SERIES_LENGTH; i++)
		for (int k = 0; k < D; k++)
			*(output + i * D + k) = (int)(*(data + (i - 1) * D + k) * a + *(data + (i - 2) * D + k) * b);
	
	for (int i = 0; i < D * TIME_SERIES_LENGTH; i++) *(output + i) -= *(data + i);
	
	for (int i = D * (TIME_SERIES_LENGTH - 2), k = 0; i < D * TIME_SERIES_LENGTH; i++, k++) 
		*(prev_sample + k) = *(data + i);
	
	return output;
}

static int* simpleForecastingDecode(int* error, const float a, const float b)
{
	int* output = (int*)calloc(D * TIME_SERIES_LENGTH, sizeof(int));
	static int* prev_sample = NULL;
	if (prev_sample == NULL) prev_sample = (int*)calloc(2*D,sizeof(int));
	
	for (int i = 0; i < D; i++)
	{
		*(output + i) = (int)(*(prev_sample + i + D) * a + *(prev_sample + i) * b) - *(error+i);
		*(output + D + i) = (int)(*(output + i) * a + *(prev_sample + D + i) * b) - *(error+D+i);
	}
	
	for (int i = 2; i < TIME_SERIES_LENGTH; i++)
		for (int k = 0; k < D; k++)
			*(output + i * D + k) = (int)(*(output + (i - 1) * D + k) * a + *(output + (i - 2) * D + k) * b) - *(error+ i * D + k);
	
	for (int i = D * (TIME_SERIES_LENGTH - 2), k = 0; i < D * TIME_SERIES_LENGTH; i++, k++) *(prev_sample + k) = *(output + i);
	
	return output;
}

static void zigzag(int* data)
{
	for (int i = 0; i < D * TIME_SERIES_LENGTH; i++) *(data + i) = (*(data + i) >> 31) ^ (*(data + i) << 1);
}

static void zigzagDecod(int* data)
{
	for (int i = 0; i < D * TIME_SERIES_LENGTH; i++) *(data + i) = (*(data + i) >> 1) ^ -(*(data + i) & 1);
}

static int* countSignificantBits(uint32_t* data)
{
	uint32_t* temp = calloc(D,sizeof(uint32_t));
	int* significant_bits = calloc(D,sizeof(int));

	for (int i = 0; i < TIME_SERIES_LENGTH; i++)
		for (int k = 0; k < D; k++)
			temp[k]|=data[i*D+k];
	for (int k = 0; k < D; k++)	significant_bits[k] = 32 - __builtin_clz(temp[k]);

	//for(int i =0;i<D;i++)printf("%d ",significant_bits[i]);
	//printf("\n");

	return significant_bits;
}

int sprintzCalcSet(uint32_t* data, bitstream_state_t* dst)
{
	int* work_data = simpleForecasting(data, 2, -1);
	zigzag(work_data);
	int* sgn_bits = countSignificantBits((uint32_t*)work_data);

	unsigned int bit_count = 0;

	//pack header
	for (int i = 0; i < D; i++)
	{
		bitstream_append_bits(dst, (unsigned long long)*(sgn_bits+i), HEADER_SECTION_BITS);
		bit_count += HEADER_SECTION_BITS;
	}
	bit_count += bitstream_write_close(dst);

	//pack payload
	for (int i = 0; i < TIME_SERIES_LENGTH; i++)
	{
		for (int k = 0; k < D; k++)
		{
			bitstream_append_bits(dst, (unsigned long long)work_data[i * D + k],sgn_bits[k]);
			bit_count += sgn_bits[k];
		}
		//bit_count+=bitstream_write_close(dst);
	}

	free(work_data);
	free(sgn_bits);
	return bit_count;
}

uint32_t* sprintzDecodeCalcSet(bitstream_state_t* src)
{
	//unpack header
	unsigned long long* sgn_bits = malloc(D * sizeof(unsigned long long));
	for (int i = 0; i < D; i++) bitstream_read_bits(src, sgn_bits + i, HEADER_SECTION_BITS);
	bitstream_read_panning_bits(src);

	//unpack payload
	unsigned long long* data = malloc(D * TIME_SERIES_LENGTH * sizeof(unsigned long long));
	for (int i = 0; i < TIME_SERIES_LENGTH; i++)
	{
		for (int k = 0; k < D; k++) bitstream_read_bits(src, data + i * D + k, (unsigned)*(sgn_bits + k));
	}

	int* data_int = malloc(D * TIME_SERIES_LENGTH * sizeof(int));
	for (int i = 0; i < D * TIME_SERIES_LENGTH; i++)*(data_int + i) = (int)*(data + i);
	zigzagDecod(data_int);
	int* output = simpleForecastingDecode(data_int, 2, -1);
	
	free(sgn_bits);
	free(data);
	free(data_int);
	return (uint32_t*)output;
}

void sprintz(uint32_t* data, int sets, unsigned char* data_encoded)
{
	unsigned char temp[128] = {0};
	bitstream_state_t dst;
	bitstream_state_t* dst_p = &dst;
	bitstream_init(dst_p, temp, 128);
	bitstream_append_int8(dst_p, sets);
	unsigned int bit_count = 8;
	for (int i = 0; i < sets; i++)bit_count += sprintzCalcSet(data + i * D * TIME_SERIES_LENGTH, dst_p);
	bit_count += bitstream_write_close(dst_p);
	bit_count += (8 - bit_count%8);
	//HUFFMAN

	huffman(temp, data_encoded, bit_count);

}

void sprintzDecode(unsigned char* data, uint32_t* data_decoded)
{
	unsigned char temp[128] = {0};
	//HUFFMAN
	huffmanDecode(data, temp);

	bitstream_state_t src;
	bitstream_state_t* src_p = &src;
	bitstream_init(src_p,temp, 128);
	int8_t sets;
	bitstream_read_int8(src_p, &sets);

	uint32_t* temp_data_decoded;
	for (int i = 0; i < sets; i++)
	{
		temp_data_decoded = sprintzDecodeCalcSet(src_p);
		for (int k = 0; k < D * TIME_SERIES_LENGTH; k++)
		{
			*(data_decoded + k + i * D * TIME_SERIES_LENGTH) = *(temp_data_decoded + k);
		}
		free(temp_data_decoded);
	}
}
