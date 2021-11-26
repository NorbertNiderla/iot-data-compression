//by Norbert Niderla, 2021

#include <stdlib.h>
#include <stdint.h>
#include <math.h>

#include "include/aac.h"
#include "include/bitstream.h"

#define ENABLE_DEBUG	(0)
#if ENABLE_DEBUG
#include <stdio.h>
#pragma message "adaptive arithmetic: debug enabled"
#endif

#define top_value 		(0x7FFFFFFF)
#define first_qtr		(0x20000000)
#define half			(0x40000000)
#define third_qtr		(0x60000000)

typedef uint32_t code;

static unsigned* bounds;
static unsigned total;
static unsigned current_number_of_symbols;
static unsigned scale;

/*
static void adaptive_arithmetic_set_bounds(unsigned number_of_symbols, unsigned* counts){
	bounds = (unsigned*)realloc(bounds, (number_of_symbols+1)*sizeof(unsigned));
	bounds[0] = 0;
	for(unsigned i = 0; i < number_of_symbols; i++){
		bounds[i + 1] = bounds[i] + counts[i];
	}
	current_number_of_symbols = number_of_symbols;
	total = bounds[current_number_of_symbols];
}
*/

void adaptive_arithmetic_set_number_of_symbols(unsigned number_of_symbols){
	current_number_of_symbols = number_of_symbols;
	bounds = (unsigned*)realloc(bounds, (number_of_symbols+1)*sizeof(unsigned));
}

unsigned adaptive_arithmetic_encode(unsigned* data, int size, unsigned char* output, int output_buffer_size){
	code high = top_value;
	code low = 0;
	code step;
	scale = 0;

#if ENABLE_DEBUG
	printf("aac: starting, %d\n", size);
#endif

	bitstream_state_t stream;
	bitstream_init(&stream, output, output_buffer_size);

	for(unsigned i = 0; i<=current_number_of_symbols; i++) bounds[i] = i;
	total = bounds[current_number_of_symbols];

	for(int i = 0; i < size; i++){
		step = (code)ceilf((float)(high - low + 1) / (float)total);
		high = low + (step * bounds[data[i] + 1]) - 1;
		low = low + (step * bounds[data[i]]);
#if ENABLE_DEBUG
		if(high == low){
			printf("arithmetic_encode: low equals high - insufficient precision: high: %lx, low: %lx\n", high, low);
		}else if(low > high){
			printf("arithmetic_encode: low greater than high - insufficient precision: high: %lx, low: %lx\n", high, low);
		}
#endif
		while((high < half) | (low >= half)) {
			if( high < half ){
				bitstream_append_bits(&stream, 0, 1);
				low <<= 1;
				high <<= 1;
				high |= 1;
				for(; scale > 0; scale-- )
					bitstream_append_bits(&stream, 1, 1);
			} else if( low >= half ) {
				bitstream_append_bits(&stream, 1, 1);
				low = ( low - half ) << 1;
				high = (( high - half ) << 1) | 1;
				for(; scale > 0; scale-- )
					bitstream_append_bits(&stream, 0, 1);
			}
        }

		while( ( low >= first_qtr ) & ( high < third_qtr ) ){
			scale++;
			low = ( low - first_qtr ) << 1;
			high = (( high - first_qtr ) << 1) + 1;
		}

		for(unsigned c = data[i] + 1; c <= current_number_of_symbols; c++){
			bounds[c]++;
		}	
		total++;
	}

	if( low < first_qtr ) {
		bitstream_append_bits(&stream, 0, 1);
		for(unsigned i=0; i < (scale + 1); i++)
			bitstream_append_bits(&stream, 1, 1);
	} else {
		bitstream_append_bits(&stream, 1, 1);
	}

	bitstream_write_close(&stream);
	return stream.stream_used_len;
}

static inline int arithmetic_decode_symbol(code cum){
	if(cum == 0){
		return 0;
	} else {
		for(unsigned i = 0; i < current_number_of_symbols; i++){
			if((bounds[i]<cum) & (bounds[i+1] >= cum)){
				return i;
			}
		}
#if ENABLE_DEBUG
		printf("arithmetic_decode: error symbol while decoding\n");
#endif
		return(999);
	}
}

void adaptive_arithmetic_decode(unsigned char* input, int bits, unsigned* data, int size){
	code low = 0;
	code high = top_value;

	unsigned long long temp = 0;
	unsigned long long bit;
	bitstream_state_t stream;
	bitstream_init(&stream, input, (bits >> 3) + 5);
	bitstream_read_bits(&stream, &temp, 31);
	code value = temp;
	code step;
	code cum;

	for(unsigned i = 0; i<=current_number_of_symbols; i++) bounds[i] = i;
	total = bounds[current_number_of_symbols];

	for(int i = 0; i < size; i++){
		step = (code)ceilf((float)(high - low + 1)/(float)total);
		cum = (code)ceilf((float)(value - low) / (float)step);
		data[i] = arithmetic_decode_symbol(cum);

		high = low + (step * bounds[data[i] + 1]) - 1;
		low = low + (step * bounds[data[i]]);

		for(unsigned c = data[i] + 1; c <= current_number_of_symbols; c++){
			bounds[c]++;
		}
		total++;

		while((high < half) | (low >= half)) {
			if( high < half ){
				low <<= 1;
				high <<= 1;
				high |= 1;
				bitstream_read_bits(&stream, &bit, 1);
				value = (value << 1) + bit;
			} else if( low >= half ) {
				low = ( low - half ) << 1;
				high = (( high - half ) << 1) | 1;
				bitstream_read_bits(&stream, &bit, 1);
				value = ((value - half) << 1) + bit;
			}
		}

		while( ( low >= first_qtr ) & ( high < third_qtr ) ){
			low = ( low - first_qtr ) << 1;
			high = (( high - first_qtr ) << 1) | 1;
			bitstream_read_bits(&stream, &bit, 1);
			value = ((value - first_qtr) << 1) + bit;
		}
	}
}

