/*
 * circular_short_buffer.c
 *
 *  Created on: Aug 16, 2020
 *      Author: norbert
 */
#include "../../../compression_iot/archive/sprintz_huffman ver.1/circular_short_buffer.h"
void short_buffer_append_bits(short_buffer* buff, unsigned long long value, int bits)
{
	buff->buffer<<=bits;
	buff->buffer|=value;
	buff->bits+=bits;
}

unsigned long long short_buffer_get_bits(short_buffer* buff, int bits)
{
	unsigned long long output;
	output = (buff->buffer>>(buff->bits-bits));
	output&=(~(0xFFFFFFFFFFFFFFFF<<bits));
	buff->bits-=bits;
	return output;
}
