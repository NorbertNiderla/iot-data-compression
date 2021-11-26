//by Norbert Niderla, 2021

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <math.h>

#include "include/arithmetic.h"

#define ENABLE_DEBUG	(0)
#if ENABLE_DEBUG
#include <stdio.h>
#pragma message "arithmetic: debug enabled"
#endif

#define code_value_bits	(32)
#define top_value 		(0xFFFFFFFF)
#define first_qtr		(0x40000000)
#define half			(0x80000000)
#define third_qtr		(0xC0000000)	

#define SQUARE_2	(1.4142135)

#define COUNT_COEFF	(100)
#define ADDITIONAL_SD_COEFF	(0)

typedef uint32_t code;

static unsigned* bounds;
static unsigned current_number_of_symbols;

void set_bounds_w_laplace_distribution(unsigned number_of_symbols, int sd){
	sd = sd + ADDITIONAL_SD_COEFF; //TEST
	bounds = (unsigned*)realloc(bounds, (number_of_symbols+3)*sizeof(unsigned));
	for (unsigned i = 1; i <= number_of_symbols; i++) {
		bounds[i] = (unsigned)round((double)LAPLACE_DIST_PRECISION * (double)SQUARE_2 * 0.5 / (double)sd * exp(-((double)i-1) * SQUARE_2 / (double)sd))*COUNT_COEFF;
		if (bounds[i] == 0) {
			for (unsigned k = i; k <= number_of_symbols; k++) {
				bounds[k] = 1*COUNT_COEFF;
			}
			break;
		}
	}
#if ENABLE_DEBUG
    printf("distribution:\n");
    for(unsigned i=1; i < number_of_symbols+1; i++){
    	printf("%d\n", bounds[i]);
    }
#endif
	bounds[number_of_symbols + 1] = bounds[2]; // sign symbol
	bounds[number_of_symbols + 2] = bounds[number_of_symbols]; // placeholder

	bounds[0] = 0;
    for(unsigned i = 1; i <= number_of_symbols + 2; i++){
        bounds[i] += bounds[i-1];
    }
    current_number_of_symbols = number_of_symbols + 2;

#if ENABLE_DEBUG
    printf("\nbounds:\n");
    for(unsigned i=0; i <= current_number_of_symbols; i++){
    	printf("%d\n", bounds[i]);
    }
#endif
}

void set_bounds(unsigned number_of_symbols, unsigned* counts){
    bounds = (unsigned*)realloc(bounds, (number_of_symbols+1)*sizeof(unsigned));
    bounds[0] = 0;
    for(unsigned i = 0; i < number_of_symbols; i++){
        bounds[i + 1] = bounds[i] + counts[i];
    }
    current_number_of_symbols = number_of_symbols;
}

void print_bounds(void){
    printf("Arithmetic coding bounds:\n");
    for(unsigned i = 0; i <= current_number_of_symbols; i++){
        printf("%d. %d\n", i, bounds[i]);
    }
    printf("\n");
}

static unsigned* output;
static int output_index;
static int bits_avail;
static int bits_to_follow;
static int output_size;

static inline unsigned output_bit(unsigned bit){
	unsigned bits = 0;
	output[output_index] = ((output[output_index] << 1) | bit);
	bits_avail--;
	bits++;
	if(bits_avail == 0){
		bits_avail=32;
		output_index++;
		if(output_index >= output_size){
			printf("arithemtic_encode: output_bit: output buffer overflow");
		}
	}
	while(bits_to_follow){
		bits_to_follow--;
		output[output_index] = ((output[output_index] << 1) | ((!bit)&1));
		bits_avail--;
		bits++;
		if(bits_avail == 0){
			bits_avail=32;
			output_index++;
			if(output_index >= output_size){
				printf("arithemtic_encode: output_bit: output buffer overflow");
			}
		}
	}
	return(bits);
}

unsigned arithmetic_encode(unsigned* data, int size, unsigned* output_ptr, int output_buffer_size){
	code high = top_value;
	code low = 0;
	code range;
	output = output_ptr;
    output_index = 0;
	output_size = output_buffer_size;
    bits_avail = 32;
    unsigned bits = 0;
	bits_to_follow = 0;

#if ENABLE_DEBUG
	printf("\n arithmetic coding input:\n");
	for(int i = 0; i < size;i++ ){
		printf("%d ", data[i]);
	}
	printf("\n\n");
#endif
	for(int i = 0; i < size; i++){
		range = high - low;
		high = low + (code)ceil((float)range * ((float)bounds[data[i] + 1]/(float)bounds[current_number_of_symbols]));
		low = low + (code)floor((float)range * ((float)bounds[data[i]]/(float)bounds[current_number_of_symbols]));
#if ENABLE_DEBUG
		if(high == low){
			printf("arithmetic_encode: low equals high - insufficient precision: high: %lx, low: %lx\n", high, low);
		}else if(low > high){
			printf("arithmetic_encode: low greater than high - insufficient precision: high: %lx, low: %lx\n", high, low);
		}
#endif
        while(1) {
            if ( high < half ){
				bits += output_bit(0);
            } else if ( low >= half ){
                bits += output_bit(1);
				low -= half;
				high -= half;
            } else if ((low >= first_qtr) & (high < third_qtr)){
                    bits_to_follow++;
					low -= first_qtr;
                    high -= first_qtr;
			} else
                break;
            low <<= 1;
            high <<= 1;
            high |= 1;
        }
	}

	bits_to_follow++;
	if(low<first_qtr){
		bits += output_bit(0);
	} else {
		bits += output_bit(1);
	}
	output[output_index] <<= bits_avail;

	return bits;
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
		return(ERROR_SYMBOL);
	}
}

void arithmetic_decode(unsigned* input, int bits, unsigned* data, int size){
	code low = 0;
	code high = top_value;
	
	code value = (code)input[0];
	code range;
	code cum;


	int input_index = 1;
	int input_internal_index = 31;
	int k = 32;
	for(int i = 0; i < size - 1; i++){
		range = high - low;
		cum = (code)floor(((float)(value - low + 1)*(float)bounds[current_number_of_symbols] - 1)/(float)range);
		data[i] = arithmetic_decode_symbol(cum);
		if(data[i] == (unsigned)ERROR_SYMBOL){
			printf("arithmetic_decode: error symbol while decoding");
		}
		high = low + (code)ceil((float)range * ((float)bounds[data[i] + 1]/(float)bounds[current_number_of_symbols]));
		low = low + (code)floor((float)range * ((float)bounds[data[i]]/(float)bounds[current_number_of_symbols]));
		while(1){
			if(high < half){

			}else if(low >= half){
				value -= half;
				low -= half;
				high -= half;
			}else if((low >= first_qtr) & (high <third_qtr)){
				value -= first_qtr;
				low -= first_qtr;
				high -= first_qtr;
			} else {
				break;
			}
			low <<= 1;
			high <<= 1;
			high++;
			value <<= 1;
			if((k++) <= bits){
				value |= ((input[input_index]>>(input_internal_index--))&1);
				if(input_internal_index == -1){
					input_internal_index = 31;
					input_index++;
				}
			}
		}
	}
	range = high - low;
	cum = (code)floor(((float)(value - low + 1)*(float)bounds[current_number_of_symbols] - 1)/(float)range);
	data[size - 1] = arithmetic_decode_symbol(cum);

#if ENABLE_DEBUG
	printf("\n arithmetic decoding output:\n");
	for(int i = 0; i < size;i++ ){
		printf("%d ", data[i]);
	}
	printf("\n\n");
#endif

}

