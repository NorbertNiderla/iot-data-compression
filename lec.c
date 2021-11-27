//by Norbert Niderla 2021

//LEC by Marcelloni, 2009

#include "lec.h"
#include "bitstream.h"

#define DICTIONARY_L    (15)

static inline int compute_binary_log2(int d) {
	int n = 0;
	d = d > 0 ? d : -d;
	while (d > 0) {
		d >>= 1;
		n++;
	}
	return n;
}

static int codewords[DICTIONARY_L]= {0, 2, 3, 4, 5, 6, 14, 30, 62, 126, 254, 510, 1022, 2046, 4094};
static int len[DICTIONARY_L] = {2, 3, 3, 3, 3, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
static int norm[DICTIONARY_L] = {0, 1, 3, 7, 15, 31, 63, 127, 255, 511, 1023, 2047, 4095, 8191, 16383};

#if ENCODER_DC_VALUE
int lec_table_encode(int* d, int size, unsigned char* output, int output_size, int dc_value){
	bitstream_state_t stream;
	bitstream_init(&stream, output, output_size);
	int bytes = 0;
	int log;

	for (int i = (size - 1); i > 0; i--) d[i] -= d[i - 1];
	d[0] -= dc_value;
	for (int i = 0; i < size; i++) {
		log = compute_binary_log2(d[i]);
		bitstream_append_bits(&stream, codewords[log], len[log]);
		d[i] = d[i] > 0 ? d[i] : d[i] + norm[log];
		bitstream_append_bits(&stream, d[i], log);
	}

	bitstream_write_close(&stream);
	return(bytes);
}

int lec_encode(int *d, int size, unsigned char *output, int output_size, int dc_value) {

	bitstream_state_t stream;
	bitstream_init(&stream, output, output_size);

	for (int i = (size - 1); i > 0; i--) d[i] -= d[i - 1];
	d[0] -= dc_value;

	for (int i = 0; i < size; i++) {
		switch (compute_binary_log2(d[i])) {
		case 0:
			bitstream_append_bits(&stream, 0, 2);
			break;
		case 1:
			bitstream_append_bits(&stream, 2, 3);
			d[i] = d[i] > 0 ? d[i] : d[i] + 1;
			bitstream_append_bits(&stream, d[i], 1);
			break;
		case 2:
			bitstream_append_bits(&stream, 3, 3);
			d[i] = d[i] > 0 ? d[i] : d[i] + 3;
			bitstream_append_bits(&stream, d[i], 2);
			break;
		case 3:
			bitstream_append_bits(&stream, 4, 3);
			d[i] = d[i] > 0 ? d[i] : d[i] + 7;
			bitstream_append_bits(&stream, d[i], 3);
			break;
		case 4:
			bitstream_append_bits(&stream, 5, 3);
			d[i] = d[i] > 0 ? d[i] : d[i] + 15;
			bitstream_append_bits(&stream, d[i], 4);
			break;
		case 5:
			bitstream_append_bits(&stream, 6, 3);
			d[i] = d[i] > 0 ? d[i] : d[i] + 31;
			bitstream_append_bits(&stream, d[i], 5);
			break;
		case 6:
			bitstream_append_bits(&stream, 14, 4);
			d[i] = d[i] > 0 ? d[i] : d[i] + 63;
			bitstream_append_bits(&stream, d[i], 6);
			break;
		case 7:
			bitstream_append_bits(&stream, 30, 5);
			d[i] = d[i] > 0 ? d[i] : d[i] + 127;
			bitstream_append_bits(&stream, d[i], 7);
			break;
		case 8:
			bitstream_append_bits(&stream, 62, 6);
			d[i] = d[i] > 0 ? d[i] : d[i] + 255;
			bitstream_append_bits(&stream, d[i], 8);
			break;
		case 9:
			bitstream_append_bits(&stream, 126, 7);
			d[i] = d[i] > 0 ? d[i] : d[i] + 511;
			bitstream_append_bits(&stream, d[i], 9);
			break;
		case 10:
			bitstream_append_bits(&stream, 254, 8);
			d[i] = d[i] > 0 ? d[i] : d[i] + 1023;
			bitstream_append_bits(&stream, d[i], 10);
			break;
		case 11:
			bitstream_append_bits(&stream, 510, 9);
			d[i] = d[i] > 0 ? d[i] : d[i] + 2047;
			bitstream_append_bits(&stream, d[i], 11);
			break;
		case 12:
			bitstream_append_bits(&stream, 1022, 10);
			d[i] = d[i] > 0 ? d[i] : d[i] + 4095;
			bitstream_append_bits(&stream, d[i], 12);
			break;
		case 13:
			bitstream_append_bits(&stream, 2046, 11);
			d[i] = d[i] > 0 ? d[i] : d[i] + 8191;
			bitstream_append_bits(&stream, d[i], 13);
			break;
		case 14:
			bitstream_append_bits(&stream, 4094, 12);
			d[i] = d[i] > 0 ? d[i] : d[i] + 16383;
			bitstream_append_bits(&stream, d[i], 14);
			break;
		}
	}
	bitstream_write_close(&stream);
	return stream.stream_used_len;
}

#else
int lec_table_encode(int* d, int size, unsigned char* output, int output_size){
	bitstream_state_t stream;
	bitstream_init(&stream, output, output_size);
	int bytes = 0;
	int log;

	for (int i = (size - 1); i > 0; i--) d[i] -= d[i - 1];
	for (int i = 0; i < size; i++) {
		log = compute_binary_log2(d[i]);
		bitstream_append_bits(&stream, codewords[log], len[log]);
		d[i] = d[i] > 0 ? d[i] : d[i] + norm[log];
		bitstream_append_bits(&stream, d[i], log);
	}

	bitstream_write_close(&stream);
	return(stream.stream_used_len);
}

int lec_encode(int *d, int size, unsigned char *output, int output_size) {

	bitstream_state_t stream;
	bitstream_init(&stream, output, output_size);
	int bytes = 0;

	for (int i = (size - 1); i > 0; i--) d[i] -= d[i - 1];

	for (int i = 0; i < size; i++) {
		switch (compute_binary_log2(d[i])) {
		case 0:
			bitstream_append_bits(&stream, 0, 2);
			break;
		case 1:
			bitstream_append_bits(&stream, 2, 3);
			d[i] = d[i] > 0 ? d[i] : d[i] + 1;
			bitstream_append_bits(&stream, d[i], 1);
			break;
		case 2:
			bitstream_append_bits(&stream, 3, 3);
			d[i] = d[i] > 0 ? d[i] : d[i] + 3;
			bitstream_append_bits(&stream, d[i], 2);
			break;
		case 3:
			bitstream_append_bits(&stream, 4, 3);
			d[i] = d[i] > 0 ? d[i] : d[i] + 7;
			bitstream_append_bits(&stream, d[i], 3);
			break;
		case 4:
			bitstream_append_bits(&stream, 5, 3);
			d[i] = d[i] > 0 ? d[i] : d[i] + 15;
			bitstream_append_bits(&stream, d[i], 4);
			break;
		case 5:
			bitstream_append_bits(&stream, 6, 3);
			d[i] = d[i] > 0 ? d[i] : d[i] + 31;
			bitstream_append_bits(&stream, d[i], 5);
			break;
		case 6:
			bitstream_append_bits(&stream, 14, 4);
			d[i] = d[i] > 0 ? d[i] : d[i] + 63;
			bitstream_append_bits(&stream, d[i], 6);
			break;
		case 7:
			bitstream_append_bits(&stream, 30, 5);
			d[i] = d[i] > 0 ? d[i] : d[i] + 127;
			bitstream_append_bits(&stream, d[i], 7);
			break;
		case 8:
			bitstream_append_bits(&stream, 62, 6);
			d[i] = d[i] > 0 ? d[i] : d[i] + 255;
			bitstream_append_bits(&stream, d[i], 8);
			break;
		case 9:
			bitstream_append_bits(&stream, 126, 7);
			d[i] = d[i] > 0 ? d[i] : d[i] + 511;
			bitstream_append_bits(&stream, d[i], 9);
			break;
		case 10:
			bitstream_append_bits(&stream, 254, 8);
			d[i] = d[i] > 0 ? d[i] : d[i] + 1023;
			bitstream_append_bits(&stream, d[i], 10);
			break;
		case 11:
			bitstream_append_bits(&stream, 510, 9);
			d[i] = d[i] > 0 ? d[i] : d[i] + 2047;
			bitstream_append_bits(&stream, d[i], 11);
			break;
		case 12:
			bitstream_append_bits(&stream, 1022, 10);
			d[i] = d[i] > 0 ? d[i] : d[i] + 4095;
			bitstream_append_bits(&stream, d[i], 12);
			break;
		case 13:
			bitstream_append_bits(&stream, 2046, 11);
			d[i] = d[i] > 0 ? d[i] : d[i] + 8191;
			bitstream_append_bits(&stream, d[i], 13);
			break;
		case 14:
			bitstream_append_bits(&stream, 4094, 12);
			d[i] = d[i] > 0 ? d[i] : d[i] + 16383;
			bitstream_append_bits(&stream, d[i], 14);
			break;
		}
	}
	bitstream_write_close(&stream);
	return stream.stream_used_len;
}
#endif
void lec_decode(unsigned char *input, int input_size, int *d, int size) {
	bitstream_state_t stream;
	bitstream_init(&stream, input, input_size);

	int i = 0;
	unsigned long long b;
	unsigned long long val;
	while (i < size) {
		//s part
		b = 0;
		val = 0;
		bitstream_shift_read_bits(&stream, &b, 2);
		if (b == 0) {
			d[i++] = 0;
		} else {
			bitstream_shift_read_bits(&stream, &b, 1);
			if (b < 7) {
				switch (b) {
				case 2:
					bitstream_read_bits(&stream, &val, 1);
					d[i] = val < 1 ? -1 : 1;
					break;
				case 3:
					bitstream_read_bits(&stream, &val, 2);
					d[i] = val < 2 ? val - 3 : val;
					break;
				case 4:
					bitstream_read_bits(&stream, &val, 3);
					d[i] = val < 4 ? val - 7 : val;
					break;
				case 5:
					bitstream_read_bits(&stream, &val, 4);
					d[i] = val < 8 ? val - 15 : val;
					break;
				case 6:
					bitstream_read_bits(&stream, &val, 5);
					d[i] = val < 16 ? val - 31 : val;
					break;
				}

				i++;

			} else {
				bitstream_shift_read_bits(&stream, &b, 1);
				if (b == 14) {
					bitstream_read_bits(&stream, &val, 6);
					d[i++] = val < 32 ? val - 63 : val;
				} else {
					bitstream_shift_read_bits(&stream, &b, 1);
					if (b == 30) {
						bitstream_read_bits(&stream, &val, 7);
						d[i++] = val < 64 ? val - 127 : val;
					} else {
						bitstream_shift_read_bits(&stream, &b, 1);
						if (b == 62) {
							bitstream_read_bits(&stream, &val, 8);
							d[i++] = val < 128 ? val - 255 : val;
						} else {
							bitstream_shift_read_bits(&stream, &b, 1);
							if (b == 126) {
								bitstream_read_bits(&stream, &val, 9);
								d[i++] = val < 256 ? val - 511 : val;
							} else {
								bitstream_shift_read_bits(&stream, &b, 1);
								if (b == 254) {
									bitstream_read_bits(&stream, &val, 10);
									d[i++] = val < 512 ? val - 1023 : val;
								} else {
									bitstream_shift_read_bits(&stream, &b, 1);
									if (b == 510) {
										bitstream_read_bits(&stream, &val, 11);
										d[i++] = val < 1024 ? val - 2047 : val;
									} else {
										bitstream_shift_read_bits(&stream, &b, 1);
										if (b == 1022) {
											bitstream_read_bits(&stream, &val,
													12);
											d[i++] =
													val < 2056 ?
															val - 4096 : val;
										} else {
											bitstream_shift_read_bits(&stream, &b, 1);
											if (b == 2046) {
												bitstream_read_bits(&stream,
														&val, 13);
												d[i++] =
														val < 4096 ?
																val - 8191 :
																val;
											} else {
												bitstream_shift_read_bits(&stream, &b,
														1);
												bitstream_read_bits(&stream,
														&val, 14);
												d[i++] =
														val < 8192 ?
																val - 16383 :
																val;
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}

    for(int i = 1; i < size; i++) d[i] += d[i-1];
}

