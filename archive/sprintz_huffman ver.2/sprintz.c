
#define TIME_SERIES_LENGTH 8 
#define D 14
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

#include "../../../compression_iot/archive/sprintz_huffman ver.2/bitstream.h"
#include "../../../compression_iot/archive/sprintz_huffman ver.2/sprintz.h"
#include "../../../compression_iot/archive/sprintz_huffman ver.2/huffman.h"
#include "../../../compression_iot/archive/sprintz_huffman ver.2/utility.h"

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
		if(temp[k]==0)sgn_bits[k] = 0;
		else sgn_bits[k] = 32 - __builtin_clz(temp[k]);
		if(sgn_bits[k] != 0) all_zeros=false;
	}

	//DEBUG
	//	for (int k = 0; k < D; k++) printf("%d,",sgn_bits[k]);
	//	printf("\n");

	return all_zeros;
}

static int sprintzCalcSet(uint16_t* data, bitstream_state_t* dst)
{
	autoregressiveForecasting(data, 2, -1);
	zigzag();
	int byte_count = 0;

	if(countSignificantBits())
	{
		rle_count++;
	}	
	else
	{	
		if(rle_count!=0)
		{
			for (int i = 0; i < D; i++)	byte_count += bitstream_append_bits(dst, (unsigned long long)0, HEADER_SECTION_BITS);
			byte_count += bitstream_write_close(dst);
			byte_count += bitstream_append_bits(dst,(unsigned long long)rle_count,RLE_COUNT_BITS);
			rle_count=0;
		}
		//pack header
		for (int i = 0; i < D; i++)	byte_count += bitstream_append_bits(dst, (unsigned long long)sgn_bits[i], HEADER_SECTION_BITS);
		byte_count += bitstream_write_close(dst);

		//pack payload
		for (int i = 0; i < TIME_SERIES_LENGTH; i++)
			for (int k = 0; k < D; k++)
				byte_count += bitstream_append_bits(dst, (unsigned long long)work_buff[i][k],sgn_bits[k]);
	}

	if(rle_count==255)
	{
		for (int i = 0; i < D; i++)	byte_count += bitstream_append_bits(dst, (unsigned long long)0, HEADER_SECTION_BITS);
		byte_count += bitstream_write_close(dst);
		byte_count += bitstream_append_bits(dst,(unsigned long long)rle_count,RLE_COUNT_BITS);
		rle_count=0;
	}

	return byte_count;
}

static int cleanSprintzRle(bitstream_state_t* dst)
{
	int byte_count = 0;
	if(rle_count!=0)
	{
		for (int i = 0; i < D; i++)	byte_count += bitstream_append_bits(dst, (unsigned long long)0, HEADER_SECTION_BITS);
		byte_count += bitstream_write_close(dst);
		byte_count += bitstream_append_bits(dst,(unsigned long long)rle_count,RLE_COUNT_BITS);
		rle_count=0;
	}
	return byte_count;
}

int sprintz(uint16_t* data, int sets, unsigned char* data_encoded)
{
	bitstream_state_t dst;
	bitstream_state_t* dst_p = &dst;
	bitstream_init(dst_p,data_encoded,D*TIME_SERIES_LENGTH*sets*2);

	int byte_count = 0;
	for (int i = 0; i < sets; i++)byte_count += sprintzCalcSet(data + i * D * TIME_SERIES_LENGTH, dst_p);
	byte_count += cleanSprintzRle(dst_p);
	byte_count += bitstream_write_close(dst_p);

	return byte_count;
}

void sprintzDecode(unsigned char* data, int sets, uint16_t* data_decoded)
{
	bitstream_state_t src;
	bitstream_init(&src,data, D*TIME_SERIES_LENGTH*sets*2);

	int i = 0;
	bool all_zeros = true;
	
	do
	{
		for (int i = 0; i < D; i++) bitstream_read_bits_int(&src, &sgn_bits[i], HEADER_SECTION_BITS);
		bitstream_read_panning_bits(&src);

		for(int k=0;k<D;k++)
		{
			if(sgn_bits[k]!=0) all_zeros=false;
			if(all_zeros==false)break;
		}

		if(all_zeros==true)
		{
			unsigned long long rle_count_de;
			bitstream_read_bits(&src,&rle_count_de,RLE_COUNT_BITS);
			for(int k=0;k<(int)rle_count_de;k++)autoregressiveForecastingDecode(data_decoded+(i+k)*D*TIME_SERIES_LENGTH, 2, -1);
			i+=rle_count_de;
		}
		else
		{
			for (int i = 0; i < TIME_SERIES_LENGTH; i++)
				for (int k = 0; k < D; k++) 
					bitstream_read_bits_int(&src, &work_buff[i][k], (unsigned)sgn_bits[k]);

			zigzagDecode();
			autoregressiveForecastingDecode(data_decoded+i*D*TIME_SERIES_LENGTH, 2, -1);
			i++;
			all_zeros=true;
		}
	}while(i<sets);
	
}



