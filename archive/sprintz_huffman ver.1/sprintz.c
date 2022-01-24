
#define TIME_SERIES_LENGTH 8 
#define D 14
#define HEADER_SECTION_BITS 5
#define NUMBER_OF_SETS_BITS 4
#define WORD_LENGTH 16
#define RLE_COUNT_BITS 8

#define PRED_CONST_A 2
#define PRED_CONST_B -1

#include <stdio.h> 
#include <stdlib.h> 
#include <math.h> 
#include <stdint.h> 
#include <string.h>
#include <stdbool.h>

#include "../../../compression_iot/archive/sprintz_huffman ver.1/bitstream.h"
#include "../../../compression_iot/archive/sprintz_huffman ver.1/sprintz.h"
#include "../../../compression_iot/archive/sprintz_huffman ver.1/huffman.h"
#include "../../../compression_iot/archive/sprintz_huffman ver.1/circular_short_buffer.h"

static int prev_sample[2][D] = {0};
static int prev_sample_decode[2][D] = {0};
static int work_buff[TIME_SERIES_LENGTH][D] = {0};
static int sgn_bits[D] = {0};
static int rle_count = 0;

static short_buffer huff_buffer = {.buffer = 0, .bits = 0};

static void autoregressiveForecasting(uint16_t* data, const float a, const float b)
{
	for (int i = 0; i < D; i++)
	{
		work_buff[0][i] = (int)round(prev_sample[1][i]*a + prev_sample[0][i]* b - data[i]);
		work_buff[1][i] = (int)round(data[i]*a + prev_sample[1][i] * b  - data[i+D]);
	}
	
	for (int i = 2; i < TIME_SERIES_LENGTH; i++)
		for (int k = 0; k < D; k++)
			work_buff[i][k] = (int)round(data[(i - 1)*D + k] * a + data[(i - 2) * D + k] * b - data[i*D+k]);
	
	//for (int i = 0; i < TIME_SERIES_LENGTH; i++)
		//for (int k = 0; k < D; k++)
			//work_buff[i][k] -= data[i*D+k];
	
	for(int k=0;k<D;k++)
	{
		prev_sample[0][k] = data[(TIME_SERIES_LENGTH-2)*D+k];
		prev_sample[1][k] = data[(TIME_SERIES_LENGTH-1)*D+k];
	}
}

static void autoregressiveForecastingDecode(uint16_t* output, const float a, const float b)
{
	for (int i = 0; i < D; i++)
	{
		output[i] = (uint16_t)round(prev_sample_decode[1][i] * a + prev_sample_decode[0][i] * b - work_buff[0][i]);
		output[D + i] = (uint16_t)round(output[i] * a + prev_sample_decode[1][i] * b - work_buff[1][i]);
	}
	
	for (int i = 2; i < TIME_SERIES_LENGTH; i++)
		for (int k = 0; k < D; k++)
			output[i * D + k] = (uint16_t)round(output[(i - 1) * D + k] * a + output[(i - 2) * D + k] * b - work_buff[i][k]);
	
	for(int k=0;k<D;k++)
	{
		prev_sample_decode[0][k] = (int)output[(TIME_SERIES_LENGTH-2)*D+k];
		prev_sample_decode[1][k] = (int)output[(TIME_SERIES_LENGTH-1)*D+k];
	}
}

static void zigzag(void)
{
	for (int i = 0; i < TIME_SERIES_LENGTH; i++)
		for (int k = 0; k < D; k++) 
			work_buff[i][k] = ((work_buff[i][k] >> (WORD_LENGTH-1)) ^ (work_buff[i][k] << 1));
}

static void zigzagDecode(void)
{
	for (int i = 0; i < TIME_SERIES_LENGTH; i++)
		for (int k = 0; k < D; k++)
			work_buff[i][k] = (work_buff[i][k] >> 1) ^ -(work_buff[i][k] & 1);
}

static bool countSignificantBits(void)
{
	int temp[D] = {0};
	bool all_zeros = true;
	
	for (int i = 0; i < TIME_SERIES_LENGTH; i++)
		for (int k = 0; k < D; k++)
			temp[k]|=work_buff[i][k];
	
	for (int k = 0; k < D; k++)	
	{
		if(temp[k]==0)sgn_bits[k]=0;
		else sgn_bits[k] = 32 - __builtin_clz(temp[k]);
		if(sgn_bits[k] != 0) all_zeros=false;
	}

	return all_zeros;
}

static int sprintzCalcSet(uint16_t* data, bitstream_state_t* dst)
{
	autoregressiveForecasting(data, PRED_CONST_A, PRED_CONST_B);
	zigzag();
	int bit_count = 0;

	if(countSignificantBits())
	{
		rle_count++;
	}	
	else
	{	
		if(rle_count!=0) //PACK RLE HEADER
		{
			for (int i = 0; i < D; i++)
			{
				while(huff_buffer.bits>=8) bit_count += huffmanByte(&huff_buffer, dst);
				short_buffer_append_bits(&huff_buffer, (unsigned long long)0, HEADER_SECTION_BITS);
			}

			while(huff_buffer.bits>=8) bit_count += huffmanByte(&huff_buffer, dst);
			short_buffer_append_bits(&huff_buffer,(unsigned long long)rle_count,RLE_COUNT_BITS);
			rle_count=0;
		}

		for (int i = 0; i < D; i++)	//PACK HEADER
		{
			while(huff_buffer.bits>=8) bit_count += huffmanByte(&huff_buffer, dst);
			short_buffer_append_bits(&huff_buffer, (unsigned long long)sgn_bits[i], HEADER_SECTION_BITS);
		}
		
		for (int i = 0; i < TIME_SERIES_LENGTH; i++) //PACK PAYLOAD
			for (int k = 0; k < D; k++)
			{
				while(huff_buffer.bits>=8) bit_count += huffmanByte(&huff_buffer, dst);
				short_buffer_append_bits(&huff_buffer, (unsigned long long)work_buff[i][k],sgn_bits[k]);
			}
	}

	if(rle_count==255) //PACK RLE HEADER IF COUNTER IS MAX
	{
		for (int i = 0; i < D; i++)
			{
				while(huff_buffer.bits>=8) bit_count += huffmanByte(&huff_buffer, dst);
				short_buffer_append_bits(&huff_buffer, (unsigned long long)0, HEADER_SECTION_BITS);
			}

			while(huff_buffer.bits>=8) bit_count += huffmanByte(&huff_buffer, dst);

			short_buffer_append_bits(&huff_buffer,(unsigned long long)rle_count,RLE_COUNT_BITS);
			rle_count=0;
	}

	return bit_count;
}

static int cleanSprintzRle(bitstream_state_t* dst)
{
	int bit_count = 0;
	if(rle_count!=0)
	{
		for (int i = 0; i < D; i++)
		{
			while(huff_buffer.bits>=8) bit_count += huffmanByte(&huff_buffer, dst);
			short_buffer_append_bits(&huff_buffer, (unsigned long long)0, HEADER_SECTION_BITS);
		}

		while(huff_buffer.bits>=8) bit_count += huffmanByte(&huff_buffer, dst);
		short_buffer_append_bits(&huff_buffer,(unsigned long long)rle_count,RLE_COUNT_BITS);
		rle_count=0;
	}
	return bit_count;
}

int sprintz(uint16_t* data, int sets, unsigned char* data_encoded)
{
	bitstream_state_t dst;
	bitstream_init(&dst,data_encoded,D*TIME_SERIES_LENGTH*sets*2);
	int bit_count=0;
	for (int i = 0; i < sets; i++)bit_count += sprintzCalcSet(data + i * D * TIME_SERIES_LENGTH, &dst);
	bit_count += cleanSprintzRle(&dst);
	bit_count += huffmanStreamClose(&huff_buffer, &dst);
	bitstream_write_close(&dst);

	return bit_count;	
}

void sprintzDecode(unsigned char* data, int sets, uint16_t* data_decoded)
{
	bitstream_state_t src;
	bitstream_init(&src,data,D*TIME_SERIES_LENGTH*sets*2);
	int i = 0;
	bool all_zeros = true;
	
	do
	{
		for (int i = 0; i < D; i++) 
		{
			while(huff_buffer.bits<16) huffmanByteDecode(&src,&huff_buffer);
			sgn_bits[i] = (int)short_buffer_get_bits(&huff_buffer, HEADER_SECTION_BITS);
		}

		for(int k=0;k<D;k++)
		{
			if(sgn_bits[k]!=0) all_zeros=false;
			if(all_zeros==false)break;
		}

		if(all_zeros==true)
		{
			unsigned long long rle_count_de;
			while(huff_buffer.bits<16)huffmanByteDecode(&src,&huff_buffer);
			rle_count_de = (int) short_buffer_get_bits(&huff_buffer, RLE_COUNT_BITS);
			for(int k=0;k<(int)rle_count_de;k++)autoregressiveForecastingDecode(data_decoded+(i+k)*D*TIME_SERIES_LENGTH, PRED_CONST_A, PRED_CONST_B);
			i+=rle_count_de;
		}
		else
		{
			for (int i = 0; i < TIME_SERIES_LENGTH; i++)
				for (int k = 0; k < D; k++) 
				{
					while(huff_buffer.bits<16)huffmanByteDecode(&src,&huff_buffer);
					work_buff[i][k] = (int)short_buffer_get_bits(&huff_buffer, sgn_bits[k]);
				}

			zigzagDecode();
			autoregressiveForecastingDecode(data_decoded+i*D*TIME_SERIES_LENGTH, PRED_CONST_A, PRED_CONST_B);
			i++;
			all_zeros=true;
		}
	}while(i<sets);
	
}



