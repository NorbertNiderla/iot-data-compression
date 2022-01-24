//by Norbert Niderla, 2021

//MINDIFF by Campobello, 2015

#include "bitstream.h"
#include "compression_iot_definitions.h"

#define MINDIFF_MU_SHIFTED	(0)

#if MINDIFF_MU_SHIFTED
	#include "math.h"
#endif

#define ENABLE_DEBUG    (0)
#if ENABLE_DEBUG
#include <stdio.h>
#pragma message "mindiff: debug enabled"
#endif

static inline int compute_binary_log2(int d) {
	int n = 0;
	d = d > 0 ? d : -d;
	while (d > 0) {
		d >>= 1;
		n++;
	}
	return n;
}

static int get_required_bits(int *data, int size) {
	int rb = 0;
	int nb = 0;
	for (int i = 0; i < size; i++) {
		nb = compute_binary_log2(data[i]);
		rb = rb < nb ? nb : rb;
	}
	return rb;
}

#if MINDIFF_MU_SHIFTED
int mindiff_encode(int *data, int size, unsigned char *output,
		int output_buffer_size) {

	int mu = 0;
	for (int i = 0; i < size; i++)
		mu += data[i];
	mu >>= compute_binary_log2(size);

	for (int i = 0; i < size; i++)
		data[i] -= mu;
	int rb = get_required_bits(data, size);

	bitstream_state_t stream;
	bitstream_init(&stream, output, output_buffer_size);
	bitstream_append_bits(&stream, mu, 16);
	bitstream_append_bits(&stream, rb, 4);

	if (rb == 0) {
		bitstream_write_close(&stream);
	} else {
		for (int i = 0; i < size; i++) {
			if (data[i] < 0)
				bitstream_append_bit(&stream, 1);
			else
				bitstream_append_bit(&stream, 0);
			bitstream_append_bits(&stream, data[i], rb);
		}
		bitstream_write_close(&stream);
	}

	return (stream.stream_used_len);
}
#else

int mindiff_encode(int *data, int size, unsigned char *output,
		int output_buffer_size) {

#if ENABLE_DEBUG
	printf("DATA ENCODED: ");
	for (int i = 0; i < size; i++)
		printf("%d, ", data[i]);
	printf("\n");
#endif

	int u = data[0];
	for (int i = 1; i < size; i++)
		if(data[i] < u) u = data[i];

	for (int i = 0; i < size; i++) data[i] -= u;
	int rb = get_required_bits(data, size);

	bitstream_state_t stream;
	bitstream_init(&stream, output, output_buffer_size);
	if( u >= 0 ){
		bitstream_append_bits(&stream, 0, 1);
		bitstream_append_bits(&stream, u, 15);
	} else {
		bitstream_append_bits(&stream, 1, 1);
		bitstream_append_bits(&stream, -u, 15);
	}

	bitstream_append_bits(&stream, rb, 4);

	if (rb == 0) {
		bitstream_write_close(&stream);
	} else {
		for (int i = 0; i < size; i++) {
			bitstream_append_bits(&stream, data[i], rb);
		}
		bitstream_write_close(&stream);
	}

	return (stream.stream_used_len);
}
#endif

void mindiff_decode(unsigned char *input, int input_buffer_size, int *data,
		int size) {
	bitstream_state_t stream;
	bitstream_init(&stream, input, input_buffer_size);

	unsigned long long rb, mu, sign;
	bitstream_read_bits(&stream, &sign, 1);
	bitstream_read_bits(&stream, &mu, 15);
	if(sign == 1){
		mu = -mu;
	}
	bitstream_read_bits(&stream, &rb, 4);
	for(int i = 0; i < size; i++){
		bitstream_read_bits_int(&stream, &data[i], rb);
		data[i] = data[i] + mu;
	}

#if ENABLE_DEBUG
	printf("DATA DECODED: ");
	for (int i = 0; i < size; i++)
		printf("%d, ", data[i]);
	printf("\n");
#endif
}
