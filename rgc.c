//by Norbert Niderla, 2021

//RGC by Kalaivani, 2020

#include "bitstream.h"

#include <math.h>

#define R_ENCODE_BITS   (4)

#define ENABLE_DEBUG    (0)
#if ENABLE_DEBUG
#include <stdio.h>
#pragma message "rgc: debug enabled"
#endif

#define PROB_COUNT_LEN	(257)
static int prob_count[PROB_COUNT_LEN] = { 0 };

static int rice_parameter_estimate(int parameter) {
	if (parameter == 0)
		return 0;
	float r = ceil(log2((parameter)));
	if (r < 0)
		return 1;
	return r;
}

int rgc_encode(int *d, int size, unsigned char *output, int output_buffer_size, int dc_value) {

#if ENABLE_DEBUG
	printf("DATA ENCODED: ");
	for (int i = 0; i < size; i++)
		printf("%d, ", d[i]);
	printf("\n");
#endif

	for (int i = 0; i < PROB_COUNT_LEN; i++)
		prob_count[i] = 0;
	for (int i = (size - 1); i > 0; i--)
		d[i] -= d[i - 1];
	d[0] -= dc_value;

	int min = 0;
	for (int i = 0; i < size; i++)
		if (d[i] < min)
			min = d[i];

	for (int i = 0; i < size; i++) {
		d[i] -= min;
		if (d[i] < PROB_COUNT_LEN - 1)
			prob_count[d[i]]++;
		else
			prob_count[PROB_COUNT_LEN - 1]++;
	}

	int prob = 0;
	int idx = 0;
	for (int i = 0; i < PROB_COUNT_LEN; i++) {
		if (prob_count[i] > prob) {
			prob = prob_count[i];
			idx = i;
		}
	}
	int r = rice_parameter_estimate(idx);

	bitstream_state_t stream;
	bitstream_init(&stream, output, output_buffer_size);
	if (min < 0) {
		bitstream_append_bits(&stream, 1, 1);
		bitstream_append_bits(&stream, -min, 15);
	} else {
		bitstream_append_bits(&stream, min, 16);
	}

	bitstream_append_bits(&stream, r, R_ENCODE_BITS);
	for (int i = 0; i < size; i++) {
		int s = d[i] >> r;
		if (s > 64){
			for(int j = 0; j < s; j++){
				if(bitstream_append_bit(&stream, 1) == (-1)) return(-1);
			}
#if ENABLE_DEBUG
			printf("s is out of reasonable bound\n");
#endif
		} else {
			if(bitstream_append_bits(&stream, ~(0xFFFFFFFFFFFFFFFF << s),s) == (-1)) return(-1);
		}
		if(bitstream_append_bits(&stream, 0, 1) == (-1)) return(-1);
		if(bitstream_append_bits(&stream, d[i] & (~(0xFFFFFFFF << r)), r) == (-1)) return(-1);
	}
	if(bitstream_write_close(&stream) == (-1)) return(-1);
	return stream.stream_used_len;
}

void rgc_decode(unsigned char *input, int input_size, int *d, int size) {
	bitstream_state_t stream;
	bitstream_init(&stream, input, input_size);

	unsigned long long m, bit, r, min, min_sign;
	int real_min;
	bitstream_read_bits(&stream, &min_sign, 1);
	bitstream_read_bits(&stream, &min, 15);
	if (min_sign == 1)
		real_min = -(int) min;
	else
		real_min = (int) min;
	bitstream_read_bits(&stream, &r, R_ENCODE_BITS);

	for (int i = 0; i < size; i++) {
		int s = 0;
		bitstream_read_bits(&stream, &bit, 1);
		while (bit != 0) {
			s++;
			bitstream_read_bits(&stream, &bit, 1);
		}
		bitstream_read_bits(&stream, &m, r);
		d[i] = (s << r) + m + real_min;
	}

	for (int i = 1; i < size; i++)
		d[i] += d[i - 1];

#if ENABLE_DEBUG
	printf("DATA DECODED: ");
	for (int i = 0; i < size; i++)
		printf("%d, ", d[i]);
	printf("\n");
#endif 
}
