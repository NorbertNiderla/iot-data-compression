//by Norbert Niderla, 2021

//ALDC by Kolo, 2012

#include <stdlib.h>

#include "bitstream.h"

#define ENABLE_DEBUG	(0)
#include "debug.h"

#define TYPE_3_CODE         (0)
#define TYPE_2_CODE         (1)
#define TYPE_BITS           (1)
#define TYPE_3_TABLE_A_CODE (0)
#define TYPE_3_TABLE_B_CODE (1)
#define TYPE_3_TABLE_C_CODE (2)
#define TYPE_3_TABLE_BITS   (2)
#define TYPE_2_TABLE_A_CODE (0)
#define TYPE_2_TABLE_B_CODE (1)
#define TYPE_2_TABLE_BITS   (1)

#define DICTIONARY_L    (15)

static int codewords_A[DICTIONARY_L] = { 0x0, 0x1, 0x3, 0x5, 0x9, 0x11, 0x21, 0x41, 0x81, 0x200, 0x402, 0x403, 0x404, 0x405, 0x406 };
static int codewords_B[DICTIONARY_L] = { 0x6f, 0x1a, 0xc, 0x3, 0x7, 0x2, 0x0, 0x2, 0x36, 0x1bb, 0x1b9, 0x375, 0x374, 0x370, 0x3e3 };
static int codewords_C[DICTIONARY_L] = { 9, 5, 0, 1, 3, 0x11, 0x21, 0x41, 0x81, 0x200, 0x402, 0x403, 0x404, 0x405, 0x406 };
static int len_A[DICTIONARY_L] = { 2, 2, 2, 3, 4, 5, 6, 7, 8, 10, 11, 11, 11, 11, 11 };
static int len_B[DICTIONARY_L] = { 7, 5, 4, 3, 3, 2, 2, 3, 6, 9, 9, 10, 10, 10, 11 };
static int len_C[DICTIONARY_L] = { 4, 3, 2, 2, 2, 5, 6, 7, 8, 10, 11, 11, 11, 11, 11 };

static int norm[DICTIONARY_L] = { 0, 1, 3, 7, 15, 31, 63, 127, 255, 511, 1023, 2047, 4095, 8191, 16383 };

#define LOGS_ARRAY_LEN	(40)
static int logs[LOGS_ARRAY_LEN];

static inline int compute_binary_log2(int d) {
	int n = 0;
	d = d > 0 ? d : -d;
	while (d > 0) {
		d >>= 1;
		n++;
	}
	return n;
}

static inline void normalize_write_val_A(int d, int log,
		bitstream_state_t *stream) {
	d = d > 0 ? d : d + norm[log];
	bitstream_append_bits(stream, codewords_A[log], len_A[log]);
	bitstream_append_bits(stream, d, log);
}

static inline void normalize_write_val_B(int d, int log,
		bitstream_state_t *stream) {
	d = d > 0 ? d : d + norm[log];
	bitstream_append_bits(stream, codewords_B[log], len_B[log]);
	bitstream_append_bits(stream, d, log);
}

static inline void normalize_write_val_C(int d, int log,
		bitstream_state_t *stream) {
	d = d > 0 ? d : d + norm[log];
	bitstream_append_bits(stream, codewords_C[log], len_C[log]);
	bitstream_append_bits(stream, d, log);
}
#if ENCODER_DC_VALUE
int aldc_encode(int *d, int size, unsigned char *output, int output_size, int dc_value) {
	bitstream_state_t stream;
	bitstream_init(&stream, output, output_size);


	if(size != LOGS_ARRAY_LEN){
		DEBUG("aldc_encode: size is not equal to logs array\n");
	}
	int F = 0;

	for (int i = (size - 1); i > 0; i--)
		d[i] -= d[i - 1];
	d[0] -= dc_value;

	for (int i = 0; i < size; i++)
		F += (d[i] >= 0 ? d[i] : -d[i]);

	for (int i = 0; i < size; i++)
		logs[i] = compute_binary_log2(d[i]);

	if ((F >= 3 * size) & (F < 12 * size)) {
		bitstream_append_bits(&stream, TYPE_3_CODE, TYPE_BITS);
		int bits_A = 0;
		int bits_B = 0;
		int bits_C = 0;
		for (int i = 0; i < size; i++) {
			bits_A += len_A[logs[i]];
			bits_B += len_B[logs[i]];
			bits_C += len_C[logs[i]];
		}

		if ((bits_A < bits_B) & (bits_A < bits_C)) {
			bitstream_append_bits(&stream, TYPE_3_TABLE_A_CODE,
			TYPE_3_TABLE_BITS);
			for (int i = 0; i < size; i++) {
				normalize_write_val_A(d[i], logs[i], &stream);
			}
		} else if ((bits_B < bits_A) & (bits_B < bits_C)) {
			bitstream_append_bits(&stream, TYPE_3_TABLE_B_CODE,
			TYPE_3_TABLE_BITS);
			for (int i = 0; i < size; i++) {
				normalize_write_val_B(d[i], logs[i], &stream);
			}
		} else {
			bitstream_append_bits(&stream, TYPE_3_TABLE_C_CODE,
			TYPE_3_TABLE_BITS);
			for (int i = 0; i < size; i++) {
				normalize_write_val_C(d[i], logs[i], &stream);
			}
		}
	} else {
		bitstream_append_bits(&stream, TYPE_2_CODE, TYPE_BITS);
		int bits_A = 0;
		int bits_B = 0;
		for (int i = 0; i < size; i++) {
			bits_A += len_A[logs[i]];
			bits_B += len_B[logs[i]];
		}

		if (bits_A < bits_B) {
			bitstream_append_bits(&stream, TYPE_2_TABLE_A_CODE,
			TYPE_2_TABLE_BITS);
			for (int i = 0; i < size; i++) {
				normalize_write_val_A(d[i], logs[i], &stream);
			}
		} else {
			bitstream_append_bits(&stream, TYPE_2_TABLE_B_CODE,
			TYPE_2_TABLE_BITS);
			for (int i = 0; i < size; i++) {
				normalize_write_val_B(d[i], logs[i], &stream);
			}
		}
	}

	bitstream_write_close(&stream);
	return stream.stream_used_len;
}

#else
int aldc_encode(int *d, int size, unsigned char *output, int output_size) {
	bitstream_state_t stream;
	bitstream_init(&stream, output, output_size);


	if(size != LOGS_ARRAY_LEN){
		DEBUG("aldc_encode: size is not equal to logs array\n");
	}
	int F = 0;

	for (int i = (size - 1); i > 0; i--)
		d[i] -= d[i - 1];

	for (int i = 0; i < size; i++)
		F += d[i];

	for (int i = 0; i < size; i++)
		logs[i] = compute_binary_log2(d[i]);
	if ((F >= 3 * size) & (F < 12 * size)) {
		 bitstream_append_bits(&stream, TYPE_3_CODE, TYPE_BITS);
		int bits_A = 0;
		int bits_B = 0;
		int bits_C = 0;
		for (int i = 0; i < size; i++) {
			bits_A += len_A[logs[i]];
			bits_B += len_B[logs[i]];
			bits_C += len_C[logs[i]];
		}

		if ((bits_A < bits_B) & (bits_A < bits_C)) {
			bitstream_append_bits(&stream, TYPE_3_TABLE_A_CODE,
			TYPE_3_TABLE_BITS);
			for (int i = 0; i < size; i++) {
				normalize_write_val_A(d[i], logs[i], &stream);
			}
		} else if ((bits_B < bits_A) & (bits_B < bits_C)) {
			bitstream_append_bits(&stream, TYPE_3_TABLE_B_CODE,
			TYPE_3_TABLE_BITS);
			for (int i = 0; i < size; i++) {
				normalize_write_val_B(d[i], logs[i], &stream);
			}
		} else {
			bitstream_append_bits(&stream, TYPE_3_TABLE_C_CODE,
			TYPE_3_TABLE_BITS);
			for (int i = 0; i < size; i++) {
				normalize_write_val_C(d[i], logs[i], &stream);
			}
		}
	} else {
		bitstream_append_bits(&stream, TYPE_2_CODE, TYPE_BITS);
		int bits_A = 0;
		int bits_B = 0;
		for (int i = 0; i < size; i++) {
			bits_A += len_A[logs[i]];
			bits_B += len_B[logs[i]];
		}

		if (bits_A < bits_B) {
			bitstream_append_bits(&stream, TYPE_2_TABLE_A_CODE,
			TYPE_2_TABLE_BITS);
			for (int i = 0; i < size; i++) {
				normalize_write_val_A(d[i], logs[i], &stream);
			}
		} else {
			bitstream_append_bits(&stream, TYPE_2_TABLE_B_CODE,
			TYPE_2_TABLE_BITS);
			for (int i = 0; i < size; i++) {
				normalize_write_val_B(d[i], logs[i], &stream);
			}
		}
	}

	bitstream_write_close(&stream);
	return stream.stream_used_len;
}
#endif
static int val_decode_A(bitstream_state_t* stream) {
	unsigned long long b = 0;
	int output = 0;
	unsigned long long val = 0;
	bitstream_read_bits(stream, &b, 2);
	switch (b) {
	case 0:
		output = 0;
		break;
	case 1:
		bitstream_read_bits(stream, &val, 1);
		output = val < 1 ? val - 1 : val;
		break;
	case 3:
		bitstream_read_bits(stream, &val, 2);
		output = val < 2 ? val - 3 : val;
		break;
	default:
		bitstream_shift_read_bits(stream, &b, 1);
		if (b == 5) {
			bitstream_read_bits(stream, &val, 3);
			output = val < 4 ? val - 7 : val;
		} else {
			bitstream_shift_read_bits(stream, &b, 1);
			if (b == 9) {
				bitstream_read_bits(stream, &val, 4);
				output = val < 8 ? val - 15 : val;
			} else {
				bitstream_shift_read_bits(stream, &b, 1);
				if (b == 17) {
					bitstream_read_bits(stream, &val, 5);
					output = val < 16 ? val - 31 : val;
				} else {
					bitstream_shift_read_bits(stream, &b, 1);
					if (b == 33) {
						bitstream_read_bits(stream, &val, 6);
						output = val < 32 ? val - 63 : val;
					} else {
						bitstream_shift_read_bits(stream, &b, 1);
						if (b == 65) {
							bitstream_read_bits(stream, &val, 7);
							output = val < 64 ? val - 127 : val;
						} else {
							bitstream_shift_read_bits(stream, &b, 1);
							if (b == 129) {
								bitstream_read_bits(stream, &val, 8);
								output = val < 128 ? val - 255 : val;
							} else {
								bitstream_shift_read_bits(stream, &b, 2);
								if (b == 0x200) {
									bitstream_read_bits(stream, &val, 9);
									output = val < 256 ? val - 511 : val;
								} else {
									bitstream_shift_read_bits(stream, &b, 1);
									switch (b) {
									case 0x402:
										bitstream_read_bits(stream, &val, 10);
										output = val < 512 ? val - 1023 : val;
										break;
									case 0x403:
										bitstream_read_bits(stream, &val, 11);
										output = val < 1024 ? val - 2047 : val;
										break;
									case 0x404:
										bitstream_read_bits(stream, &val, 12);
										output = val < 2048 ? val - 4095 : val;
										break;
									case 0x405:
										bitstream_read_bits(stream, &val, 13);
										output = val < 4096 ? val - 8191 : val;
										break;
									case 0x406:
										bitstream_read_bits(stream, &val, 14);
										output = val < 8192 ? val - 16383 : val;
										break;
									}
								}
							}
						}
					}
				}
			}
		}
		break;
	}
	return output;
}

static int val_decode_B(bitstream_state_t *stream) {
	unsigned long long b = 0;
	int output = 0;
	unsigned long long val = 0;
	bitstream_read_bits(stream, &b, 2);
	switch (b) {
	case 2:
		bitstream_read_bits(stream, &val, 5);
		output = val < 16 ? val - 31 : val;
		break;
	case 0:
		bitstream_read_bits(stream, &val, 6);
		output = val < 32 ? val - 63 : val;
		break;
	default:
		bitstream_shift_read_bits(stream, &b, 1);
		switch (b) {
		case 3:
			bitstream_read_bits(stream, &val, 3);
			output = val < 4 ? val - 7 : val;
			break;
		case 7:
			bitstream_read_bits(stream, &val, 4);
			output = val < 8 ? val - 15 : val;
			break;
		case 4:
			bitstream_read_bits(stream, &val, 7);
			output = val < 64 ? val - 127 : val;
			break;
		default:
			bitstream_shift_read_bits(stream, &b, 1);
			if (b == 12) {
				bitstream_read_bits(stream, &val, 2);
				output = val < 2 ? val - 3 : val;
			} else {
				bitstream_shift_read_bits(stream, &b, 1);
				if (b == 0x1c) {
					bitstream_read_bits(stream, &val, 1);
					output = val < 1 ? val - 1 : val;
				} else {
					bitstream_shift_read_bits(stream, &b, 1);
					if (b == 0x36) {
						bitstream_read_bits(stream, &val, 8);
						output = val < 128 ? val - 255 : val;
					} else {
						bitstream_shift_read_bits(stream, &b, 1);
						if (b == 0x36) {
							bitstream_read_bits(stream, &val, 8);
							output = val < 128 ? val - 255 : val;
						} else {
							bitstream_shift_read_bits(stream, &b, 1);
							if (b == 0x6f) {
								output = 0;
							} else {
								bitstream_shift_read_bits(stream, &b, 2);
								switch (b) {
								case 0x1bb:
									bitstream_read_bits(stream, &val, 9);
									output = val < 256 ? val - 511 : val;
									break;
								case 0x1b9:
									bitstream_read_bits(stream, &val, 10);
									output = val < 512 ? val - 1023 : val;

									break;
								default:
									bitstream_shift_read_bits(stream, &b, 2);
									switch (b) {
									case 0x375:
										bitstream_read_bits(stream, &val, 11);
										output = val < 1024 ? val - 2047 : val;
										break;
									case 0x374:
										bitstream_read_bits(stream, &val, 12);
										output = val < 2048 ? val - 4095 : val;
										break;
									case 0x370:
										bitstream_read_bits(stream, &val, 13);
										output = val < 4096 ? val - 8191 : val;
										break;
									default:
										bitstream_shift_read_bits(stream, &b,
												1);
										bitstream_read_bits(stream, &val, 14);
										output = val < 8192 ? val - 16383 : val;
										break;

									}
									break;
								}
							}
						}
					}
				}
			}
			break;
		}
	}
	return output;
}

static int val_decode_C(bitstream_state_t *stream) {
	unsigned long long b = 0;
	int output = 0;
	unsigned long long val = 0;
	bitstream_read_bits(stream, &b, 2);
	switch (b) {
	case 0:
		bitstream_read_bits(stream, &val, 2);
		output = val < 2 ? val - 3 : val;
		break;
	case 1:
		bitstream_read_bits(stream, &val, 3);
		output = val < 4 ? val - 7 : val;
		break;
	case 3:
		bitstream_read_bits(stream, &val, 4);
		output = val < 8 ? val - 15 : val;
		break;
	default:
		bitstream_shift_read_bits(stream, &b, 1);
		if (b == 5) {
			bitstream_read_bits(stream, &val, 1);
			output = val < 1 ? val - 1 : val;
		} else {
			bitstream_shift_read_bits(stream, &b, 1);
			if (b == 9) {
				output = 0;
			} else {
				bitstream_shift_read_bits(stream, &b, 1);
				if (b == 17) {
					bitstream_read_bits(stream, &val, 5);
					output = val < 16 ? val - 31 : val;
				} else {
					bitstream_shift_read_bits(stream, &b, 1);
					if (b == 33) {
						bitstream_read_bits(stream, &val, 6);
						output = val < 32 ? val - 63 : val;
					} else {
						bitstream_shift_read_bits(stream, &b, 1);
						if (b == 65) {
							bitstream_read_bits(stream, &val, 7);
							output = val < 64 ? val - 127 : val;
						} else {
							bitstream_shift_read_bits(stream, &b, 1);
							if (b == 129) {
								bitstream_read_bits(stream, &val, 8);
								output = val < 128 ? val - 255 : val;
							} else {
								bitstream_shift_read_bits(stream, &b, 2);
								if (b == 0x200) {
									bitstream_read_bits(stream, &val, 9);
									output = val < 256 ? val - 511 : val;
								} else {
									bitstream_shift_read_bits(stream, &b, 1);
									switch (b) {
									case 0x402:
										bitstream_read_bits(stream, &val, 10);
										output = val < 512 ? val - 1023 : val;
										break;
									case 0x403:
										bitstream_read_bits(stream, &val, 11);
										output = val < 1024 ? val - 2047 : val;
										break;
									case 0x404:
										bitstream_read_bits(stream, &val, 12);
										output = val < 2048 ? val - 4095 : val;
										break;
									case 0x405:
										bitstream_read_bits(stream, &val, 13);
										output = val < 4096 ? val - 8191 : val;
										break;
									case 0x406:
										bitstream_read_bits(stream, &val, 14);
										output = val < 8192 ? val - 16383 : val;
										break;
									}
								}
							}
						}
					}
				}
			}
		}
		break;
	}
	return output;
}

void aldc_decode(unsigned char *input, int input_size, int *output, int size) {
	bitstream_state_t stream;
	bitstream_init(&stream, input, input_size);

	unsigned long long type;
	unsigned long long table;
	bitstream_read_bits(&stream, &type, TYPE_BITS);
	switch (type) {
	case TYPE_3_CODE:
		bitstream_read_bits(&stream, &table, TYPE_3_TABLE_BITS);
		switch (table) {
		case TYPE_3_TABLE_A_CODE:
			for (int i = 0; i < size; i++)
				output[i++] = val_decode_A(&stream);
			break;
		case TYPE_3_TABLE_B_CODE:
            for (int i = 0; i < size; i++)
				output[i++] = val_decode_B(&stream);
			break;
		case TYPE_3_TABLE_C_CODE:
            for (int i = 0; i < size; i++)
				output[i++] = val_decode_C(&stream);
			break;
		default:
			DEBUG("aldc_decode: invalid table code");
			break;
		}
		break;
	case TYPE_2_CODE:
		bitstream_read_bits(&stream, &table, TYPE_2_TABLE_BITS);
		switch (table) {
		case TYPE_2_TABLE_A_CODE:
            for (int i = 0; i < size; i++)
				output[i++] = val_decode_A(&stream);
			break;
		case TYPE_2_TABLE_B_CODE:
            for (int i = 0; i < size; i++)
				output[i++] = val_decode_B(&stream);
			break;
		default:
			DEBUG("aldc_decode: invalid table code");
			break;
		}
		break;
	default:
		DEBUG("aldc_decode: invalid type code");
		break;
	}

	for(int i = 1; i < size; i++) output[i] += output[i-1];
}
