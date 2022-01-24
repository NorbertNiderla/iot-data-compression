#include "../../../compression_iot/archive/sprintz_huffman ver.1/rle.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

#include "../../../compression_iot/archive/sprintz_huffman ver.1/bitstream.h"

typedef struct {
	unsigned long long sign;
	unsigned long long cnt;
}rle_element;

uint8_t rle(unsigned char* src, unsigned char* dst, int bits, unsigned int depth)
{
	bitstream_state_t src_state;
	bitstream_state_t* src_state_p = &src_state;
	bitstream_init(src_state_p, src, 128);
		
	bitstream_state_t dst_state;
	bitstream_state_t* dst_state_p = &dst_state;
	bitstream_init(dst_state_p, dst, 128);

	bitstream_append_int8(dst_state_p, 0x00);

	unsigned long long next;

	rle_element elements[256];
	elements[0].cnt = 0;
	//for (int i = 0; i < 128; i++)elements[i].cnt = 0;
	bitstream_read_bits(src_state_p, &elements[0].sign, 1);

	int k = 0;
	for(int i = 1; i < bits; i++)
	{
		bitstream_read_bits(src_state_p, &next, 1);
		if (elements[k].sign == next) elements[k].cnt++;
		else
		{
			elements[++k].sign = next;
			elements[k].cnt = 0;
		}

		if (elements[k].cnt == (unsigned long long)((1 << depth) - 1))
		{
			k++;
			bitstream_read_bits(src_state_p, &elements[k].sign, 1);
			elements[k].cnt = 0;
			i++;
		}
	}

	for (int i = 0; i <= k; i++)
	{
		bitstream_append_bits(dst_state_p, elements[i].sign, 1);
		bitstream_append_bits(dst_state_p, elements[i].cnt, depth);
	}

	bitstream_write_close(dst_state_p);
	*dst = (unsigned char)(k + 1);
	return (uint8_t)(k+1);
}

void rleDecode(unsigned char* src, unsigned char* dst, const int depth)
{
	bitstream_state_t src_state;
	bitstream_state_t* src_state_p = &src_state;
	bitstream_init(src_state_p, src, 128);

	bitstream_state_t dst_state;
	bitstream_state_t* dst_state_p = &dst_state;
	bitstream_init(dst_state_p, dst, 128);

	unsigned long long cnt = 0;
	unsigned long long temp;
	int k = 0;
	int8_t period_count;
	bitstream_read_int8(src_state_p, &period_count);

	while(k < period_count)
	{
		bitstream_read_bits(src_state_p,&temp,1);
		bitstream_read_bits(src_state_p,&cnt,depth);
		if(temp==1)bitstream_append_bits(dst_state_p,~(0xFFFFFFFFFFFFFFFF<<(cnt+1)),cnt+1);
		else bitstream_append_bits(dst_state_p,0ULL,cnt+1);
		k++;
	}

	bitstream_write_close(dst_state_p);

}
