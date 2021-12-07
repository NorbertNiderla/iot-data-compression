/*
 * stoj.c
 *
 *  Created on: Jul 13, 2021
 *      Author: Norbert Niderla
 *
 *      stoj by Stojkoska, 2017
 */

#include "bitstream.h"

//stoj definitions:
#define DELTA	(1)
#define _8_DELTA	(8)
#define N_8_DELTA	(-8)

#define FLAG	(0x2)
//end

#define ENABLE_DEBUG    (0)
#include "debug.h"

#if ENCODER_DC_VALUE
int stoj_encode(int *data, int size, unsigned char *output,
		int output_buffer_size, int dc_value) {

#if ENABLE_DEBUG
	DEBUG("DATA ENCODED: ");
	for (int i = 0; i < size; i++)
		DEBUG("%d, ", data[i]);
	DEBUG("\n");
#endif

	bitstream_state_t stream;
	bitstream_init(&stream, output, output_buffer_size);

	for (int i = (size - 1); i > 0; i--)
		data[i] -= data[i - 1];
	data[0] -= dc_value;

	/*
	 *	coding tree is written with an assumption that
	 *	delta can have effectively only one value: 1
	 */
	if (bitstream_append_bits(&stream, data[0], 16) == (-1))
		return (-1);
	for (int i = 1; i < size; i++) {
		if (data[i] == 0) {
			if (bitstream_append_bits(&stream, 0, 2) == (-1))
				return (-1);
		} else if (data[i] == DELTA) {
			if (bitstream_append_bits(&stream, 1, 2) == (-1))
				return (-1);
		} else if (data[i] == -DELTA) {
			if (bitstream_append_bits(&stream, 3, 2) == (-1))
				return (-1);
		} else if (data[i] > DELTA) {
			if (bitstream_append_bits(&stream, FLAG, 2) == (-1))
				return (-1);
			if (bitstream_append_bits(&stream, ~(0xFFFFFFFF << (data[i] - 1)),
					data[i] - 1) == (-1))
				return (-1);
			if (bitstream_append_bits(&stream, FLAG, 2) == (-1))
				return (-1);
		} else if (data[i] < -DELTA) {
			if (bitstream_append_bits(&stream, FLAG, 2) == (-1))
				return (-1);
			if (bitstream_append_bits(&stream, 0, -data[i] - 1) == (-1))
				return (-1);
			if (bitstream_append_bits(&stream, FLAG, 2) == (-1))
				return (-1);
		} else {
#if ENABLE_DEBUG
			DEBUG("stoj_encode: invalid value in main coding tree.");
#endif
		}
	}
	bitstream_write_close(&stream);
	return stream.stream_used_len;
}

#else
int stoj_encode(int *data, int size, unsigned char *output,
		int output_buffer_size) {

#if ENABLE_DEBUG
	DEBUG("DATA ENCODED: ");
	for (int i = 0; i < size; i++)
		DEBUG("%d, ", data[i]);
	DEBUG("\n");
#endif

	bitstream_state_t stream;
	bitstream_init(&stream, output, output_buffer_size);

	for (int i = (size - 1); i > 0; i--)
		data[i] -= data[i - 1];

	/*
	 *	coding tree is written with an assumption that
	 *	delta can have effectively only one value: 1
	 */
	if (bitstream_append_bits(&stream, data[0], 16) == (-1))
		return (-1);
	for (int i = 1; i < size; i++) {
		if (data[i] == 0) {
			if (bitstream_append_bits(&stream, 0, 2) == (-1))
				return (-1);
		} else if (data[i] == DELTA) {
			if (bitstream_append_bits(&stream, 1, 2) == (-1))
				return (-1);
		} else if (data[i] == -DELTA) {
			if (bitstream_append_bits(&stream, 3, 2) == (-1))
				return (-1);
		} else if (data[i] > DELTA) {
			if (bitstream_append_bits(&stream, FLAG, 2) == (-1))
				return (-1);
			if (bitstream_append_bits(&stream, ~(0xFFFFFFFF << (data[i] - 1)),
					data[i] - 1) == (-1))
				return (-1);
			if (bitstream_append_bits(&stream, FLAG, 2) == (-1))
				return (-1);
		} else if (data[i] < -DELTA) {
			if (bitstream_append_bits(&stream, FLAG, 2) == (-1))
				return (-1);
			if (bitstream_append_bits(&stream, 0, -data[i] - 1) == (-1))
				return (-1);
			if (bitstream_append_bits(&stream, FLAG, 2) == (-1))
				return (-1);
		} else {
#if ENABLE_DEBUG
			DEBUG("stoj_encode: invalid value in main coding tree.");
#endif
		}
	}
	bitstream_write_close(&stream);
	return stream.stream_used_len;
}
#endif
void stoj_decode(unsigned char *input, int input_buffer_size, int *data,
			int size) {

	bitstream_state_t stream;
	bitstream_init(&stream, input, input_buffer_size);

	unsigned long long val;
	bitstream_read_bits(&stream, &val, 16);
	data[0] = (int)val;

	unsigned long long b;
	int k;
	int sign;
	for (int i = 1; i < size; i++) {
		bitstream_read_bits(&stream, &b, 2);
		switch (b) {
		case 0:
			data[i] = 0;
			break;
		case 1:
			data[i] = DELTA;
			break;
		case 3:
			data[i] = -DELTA;
			break;
		case FLAG:
			bitstream_read_bits(&stream, &b, 3);
			k = 1;
			while((b & 3) != FLAG){
				bitstream_shift_read_bits(&stream, &b, 1);
				k++;
			}
			sign = (b >> 2) & 1;
			if(sign == 1){
				data[i] = k + 1;
			} else {
				data[i] = -(k + 1);
			}
			break;
		default:
#if ENABLE_DEBUG
			DEBUG("stoj_decode: invalid main switch statement input value.");
#endif
			break;
		}
	}

	for (int i = 1; i < size; i++)
			data[i] += data[i - 1];

#if ENABLE_DEBUG
	DEBUG("DATA DECODED: ");
	for (int i = 0; i < size; i++)
		DEBUG("%d, ", data[i]);
	DEBUG("\n");
#endif
}

