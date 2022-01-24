
#define TIME_SERIES_LENGTH 8 
#define D 3 
#define HEADER_SECTION_BITS 5
#define NUMBER_OF_SETS_BITS 4
#define WORD_LENGTH 16
#define RLE_COUNT_BITS 8

#include <stdio.h> 
#include <stdlib.h> 
#include <math.h> 
#include <stdint.h> 
#include <string.h>
#include <stdbool.h>

#include "../../../compression_iot/archive/sprintz ver.1/bitstream.h"
#include "../../../compression_iot/archive/sprintz ver.1/sprintz.h"
#include "../../../compression_iot/archive/sprintz ver.1/huffman.h"

static int prev_sample[2][D] = {0};
static int prev_sample_decode[2][D] = {0};
static int work_buff[TIME_SERIES_LENGTH][D] = {0};
static int sgn_bits[D] = {0};
static int rle_count = 0;

static void autoregressiveForecasting(uint16_t* data, const float a, const float b)
{
	for (int i = 0; i < D; i++)
	{
		work_buff[0][i] = (int)(prev_sample[1][i]*a + prev_sample[0][i]* b);
		work_buff[1][i] = (int)(data[i]*a + prev_sample[1][i] * b);
	}
	
	for (int i = 2; i < TIME_SERIES_LENGTH; i++)
		for (int k = 0; k < D; k++)
			work_buff[i][k] = (int)(data[(i - 1)*D + k] * a + data[(i - 2) * D + k] * b);
	
	for (int i = 0; i < TIME_SERIES_LENGTH; i++)
		for (int k = 0; k < D; k++)
			work_buff[i][k] -= data[i*D+k];
	
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
		output[i] = (uint16_t)((prev_sample_decode[1][i] * a + prev_sample_decode[0][i] * b) - work_buff[0][i]);
		output[D + i] = (uint16_t)((output[i] * a + prev_sample_decode[1][i] * b) - work_buff[1][i]);
	}
	
	for (int i = 2; i < TIME_SERIES_LENGTH; i++)
		for (int k = 0; k < D; k++)
			output[i * D + k] = (uint16_t)((output[(i - 1) * D + k] * a + output[(i - 2) * D + k] * b) - work_buff[i][k]);
	
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
		sgn_bits[k] = 32 - __builtin_clz(temp[k]);
		if(sgn_bits[k] != 0) all_zeros=false;
	}

	return all_zeros;
}

static int sprintzCalcSet(uint16_t* data, bitstream_state_t* dst)
{
	autoregressiveForecasting(data, 2, -1);
	zigzag();
	unsigned int bit_count = 0;

	if(countSignificantBits())
	{
		rle_count++;
	}	
	else
	{	
		if(rle_count!=0)
		{
			for (int i = 0; i < D; i++)	bit_count += bitstream_append_bits(dst, (unsigned long long)0, HEADER_SECTION_BITS);
			bit_count += bitstream_write_close(dst);
			bit_count += bitstream_append_bits(dst,(unsigned long long)rle_count,RLE_COUNT_BITS);
			rle_count=0;
		}
		//pack header
		for (int i = 0; i < D; i++)	bit_count += bitstream_append_bits(dst, (unsigned long long)sgn_bits[i], HEADER_SECTION_BITS);
		bit_count += bitstream_write_close(dst);

		//pack payload
		for (int i = 0; i < TIME_SERIES_LENGTH; i++)
			for (int k = 0; k < D; k++)
				bit_count += bitstream_append_bits(dst, (unsigned long long)work_buff[i][k],sgn_bits[k]);
	}

	if(rle_count==255)
	{
		for (int i = 0; i < D; i++)	bit_count += bitstream_append_bits(dst, (unsigned long long)0, HEADER_SECTION_BITS);
		bit_count += bitstream_write_close(dst);
		bit_count += bitstream_append_bits(dst,(unsigned long long)rle_count,RLE_COUNT_BITS);
		rle_count=0;
	}

	return bit_count;
}

void sprintz(uint16_t* data, int sets, unsigned char* data_encoded)
{
	unsigned char temp[128] = {0};
	bitstream_state_t dst;
	bitstream_state_t* dst_p = &dst;
	bitstream_init(dst_p, temp, 128);
	bitstream_append_int8(dst_p, sets);
	unsigned int byte_count = 1;
	for (int i = 0; i < sets; i++)byte_count += sprintzCalcSet(data + i * D * TIME_SERIES_LENGTH, dst_p);
	byte_count += bitstream_write_close(dst_p);
	//bit_count += (8 - bit_count%8);
	huffman(temp, data_encoded, byte_count);
}

void sprintzDecode(unsigned char* data, uint16_t* data_decoded)
{
	unsigned char temp[128] = {0};
	huffmanDecode(data, temp);

	bitstream_state_t src;
	bitstream_state_t* src_p = &src;
	bitstream_init(src_p,temp, 128);
	int8_t sets;
	bitstream_read_int8(src_p, &sets);

	int i = 0;
	bool all_zeros = true;
	
	do
	{
		for (int i = 0; i < D; i++) bitstream_read_bits_int(src_p, &sgn_bits[i], HEADER_SECTION_BITS);
		bitstream_read_panning_bits(src_p);

		for(int k=0;k<D;k++) if(sgn_bits[k]!=0) all_zeros=false;
		if(all_zeros==true)
		{
			unsigned long long rle_count_de;
			bitstream_read_bits(src_p,&rle_count_de,RLE_COUNT_BITS);
			for(int k=0;k<(int)rle_count_de;k++)autoregressiveForecastingDecode(data_decoded+(i+k)*D*TIME_SERIES_LENGTH, 2, -1);
			i+=rle_count_de;
		}
		else
		{
			for (int i = 0; i < TIME_SERIES_LENGTH; i++)
				for (int k = 0; k < D; k++) 
					bitstream_read_bits_int(src_p, &work_buff[i][k], (unsigned)sgn_bits[k]);

			zigzagDecode();
			autoregressiveForecastingDecode(data_decoded+i*D*TIME_SERIES_LENGTH, 2, -1);
			i++;
			all_zeros=true;
		}
	}while(i<sets);
	
}
/*
static int cleanSprintzRle(bitstream_state_t* dst)
{
	int bit_count = 0;
	if(rle_count!=0)
	{
		for (int i = 0; i < D; i++)	bit_count += bitstream_append_bits(dst, (unsigned long long)0, HEADER_SECTION_BITS);
		bit_count += bitstream_write_close(dst);
		bit_count += bitstream_append_bits(dst,(unsigned long long)rle_count,RLE_COUNT_BITS);
		rle_count=0;
	}
	return bit_count;
}
*/
