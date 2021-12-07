//by Norbert Niderla

//ND by Xuejung,2010

#include <stdint.h>

#include "bitstream.h"

#define ENABLE_DEBUG    (0)
#include "debug.h"

#if ENCODER_DC_VALUE
int nd_encode(int *data, int size, unsigned char *output,
		int output_buffer_size, int dc_value) {

#if ENABLE_DEBUG
	printf("DATA ENCODED: ");
	for (int i = 0; i < size; i++)
		printf("%d, ", data[i]);
	printf("\n");
#endif

	bitstream_state_t stream;
	bitstream_init(&stream, output, output_buffer_size);
	for (int i = (size - 1); i > 0; i--)
		data[i] -= data[i - 1];
	data[0] -= dc_value;

	for (int i = 0; i < size; i++) {
		switch (data[i]) {
		case 0:
			bitstream_append_bits(&stream, 0, 2);
			break;
		case -1:
			bitstream_append_bits(&stream, 2, 3);
			break;
		case 1:
			bitstream_append_bits(&stream, 3, 3);
			break;
		case -2:
			bitstream_append_bits(&stream, 8, 4);
			break;
		case 2:
			bitstream_append_bits(&stream, 9, 4);
			break;
		case -3:
			bitstream_append_bits(&stream, 10, 4);
			break;
		case 3:
			bitstream_append_bits(&stream, 11, 4);
			break;
		case -4:
			bitstream_append_bits(&stream, 24, 5);
			break;
		case 4:
			bitstream_append_bits(&stream, 25, 5);
			break;
		case -5:
			bitstream_append_bits(&stream, 26, 5);
			break;
		case 5:
			bitstream_append_bits(&stream, 27, 5);
			break;
		case -6:
			bitstream_append_bits(&stream, 56, 6);
			break;
		case 6:
			bitstream_append_bits(&stream, 57, 6);
			break;
		case -7:
			bitstream_append_bits(&stream, 58, 6);
			break;
		case 7:
			bitstream_append_bits(&stream, 59, 6);
			break;
		default:
			bitstream_append_bits(&stream, 15, 4);
			if (data[i] >= 0) {
				bitstream_append_bits(&stream, 0, 1);
				bitstream_append_bits(&stream, data[i], 15);
			} else {
				bitstream_append_bits(&stream, 1, 1);
				bitstream_append_bits(&stream, -data[i], 15);
			}
			break;
		}
	}
	bitstream_write_close(&stream);
	return stream.stream_used_len;
}

#else
int nd_encode(int *data, int size, unsigned char *output,
		int output_buffer_size) {

#if ENABLE_DEBUG
	printf("DATA ENCODED: ");
	for (int i = 0; i < size; i++)
		printf("%d, ", data[i]);
	printf("\n");
#endif

	bitstream_state_t stream;
	bitstream_init(&stream, output, output_buffer_size);
	for (int i = (size - 1); i > 0; i--)
		data[i] -= data[i - 1];

	for (int i = 0; i < size; i++) {
		switch (data[i]) {
		case 0:
			bitstream_append_bits(&stream, 0, 2);
			break;
		case -1:
			bitstream_append_bits(&stream, 2, 3);
			break;
		case 1:
			bitstream_append_bits(&stream, 3, 3);
			break;
		case -2:
			bitstream_append_bits(&stream, 8, 4);
			break;
		case 2:
			bitstream_append_bits(&stream, 9, 4);
			break;
		case -3:
			bitstream_append_bits(&stream, 10, 4);
			break;
		case 3:
			bitstream_append_bits(&stream, 11, 4);
			break;
		case -4:
			bitstream_append_bits(&stream, 24, 5);
			break;
		case 4:
			bitstream_append_bits(&stream, 25, 5);
			break;
		case -5:
			bitstream_append_bits(&stream, 26, 5);
			break;
		case 5:
			bitstream_append_bits(&stream, 27, 5);
			break;
		case -6:
			bitstream_append_bits(&stream, 56, 6);
			break;
		case 6:
			bitstream_append_bits(&stream, 57, 6);
			break;
		case -7:
			bitstream_append_bits(&stream, 58, 6);
			break;
		case 7:
			bitstream_append_bits(&stream, 59, 6);
			break;
		default:
			bitstream_append_bits(&stream, 15, 4);
			if (data[i] >= 0) {
				bitstream_append_bits(&stream, 0, 1);
				bitstream_append_bits(&stream, data[i], 15);
			} else {
				bitstream_append_bits(&stream, 1, 1);
				bitstream_append_bits(&stream, -data[i], 15);
			}
			break;
		}
	}
	bitstream_write_close(&stream);
	return stream.stream_used_len;
}
#endif
void nd_decode(unsigned char *input, int input_size, int *d, int size) {

	bitstream_state_t stream;
	bitstream_init(&stream, input, input_size);

	unsigned long long val = 0;
	unsigned long long sign = 0;
	for (int i = 0; i < size; i++) {
		val = 0;
		sign = 0;
		bitstream_read_bits(&stream, &val, 2);
		if (val == 0) {
			d[i] = 0;
		} else if (val == 1) {
			bitstream_shift_read_bits(&stream, &val, 1);
			if (val == 2) {
				d[i] = -1;
			} else if (val == 3) {
				d[i] = 1;
			}
		} else if (val == 2) {
			bitstream_shift_read_bits(&stream, &val, 2);
			switch (val) {
			case 8:
				d[i] = -2;
				break;
			case 9:
				d[i] = 2;
				break;
			case 10:
				d[i] = -3;
				break;
			case 11:
				d[i] = 3;
				break;
			default:
#if ENABLE_DEBUG
				printf("nd_decode: symbol not recognized\n");
#endif
				break;

			}
		} else {
			bitstream_shift_read_bits(&stream, &val, 1);
			if (val == 6) {
				bitstream_shift_read_bits(&stream, &val, 2);
				switch (val) {
				case 24:
					d[i] = -4;
					break;
				case 25:
					d[i] = 4;
					break;
				case 26:
					d[i] = -5;
					break;
				case 27:
					d[i] = 5;
					break;
				default:
#if ENABLE_DEBUG
					printf("nd_decode: symbol not recognized\n");
#endif
					break;
				}
			} else {
				bitstream_shift_read_bits(&stream, &val, 1);
				if (val == 14) {
					bitstream_shift_read_bits(&stream, &val, 2);
					switch (val) {
					case 56:
						d[i] = -6;
						break;
					case 57:
						d[i] = 6;
						break;
					case 58:
						d[i] = -7;
						break;
					case 59:
						d[i] = 7;
						break;
					default:
#if ENABLE_DEBUG
						printf("nd_decode: symbol not recognized\n");
#endif
						break;
					}
				} else {
					bitstream_read_bits(&stream, &sign, 1);
					bitstream_read_bits(&stream, &val, 15);
					d[i] = (sign == 0) ? val : -val;
				}
			}
		}
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

