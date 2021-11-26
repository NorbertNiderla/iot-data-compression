//by Norbert Niderla 2021
//SLEC by Liang, 2014

#include "include/slec.h"
#include "include/bitstream.h"

static inline int compute_binary_log2(int d) {
	int n = 0;
	d = d > 0 ? d : -d;
	while (d > 0) {
		d >>= 1;
		n++;
	}
	return n;
}

static inline void append_s_stamp(int current_s, int h, int else_code,
		int else_l, bitstream_state_t *stream) {
	if (current_s == h) {
		bitstream_append_bits(stream, 0, 2);
	} else if (current_s + 1 == h) {
		bitstream_append_bits(stream, 1, 2);
	} else if (current_s - 1 == h) {
		bitstream_append_bits(stream, 2, 2);
	} else {
		bitstream_append_bits(stream, 3, 2);
		bitstream_append_bits(stream, else_code, else_l);
	}
}

static inline void normalize_append_d(int d, int norm, int len,
		bitstream_state_t *stream) {
	d = d > 0 ? d : d + norm;
	bitstream_append_bits(stream, d, len);
}

int slec_encode(int *d, int size, unsigned char *output, int output_size, int dc_value) {

	bitstream_state_t stream;
	bitstream_init(&stream, output, output_size);
	int current_s = 0;

	for (int i = (size - 1); i > 0; i--)
		d[i] -= d[i - 1];
	d[0] -= dc_value;

	for (int i = 0; i < size; i++) {
		switch (compute_binary_log2(d[i])) {
		case 0:
			if (current_s == 0)
				bitstream_append_bits(&stream, 0, 2);
			else if (current_s - 1 == 0)
				bitstream_append_bits(&stream, 2, 2);
			else {
				bitstream_append_bits(&stream, 3, 2);
				bitstream_append_bits(&stream, 0, 2);
			}
			current_s = 0;
			break;
		case 1:
			append_s_stamp(current_s, 1, 2, 3, &stream);
			current_s = 1;
			normalize_append_d(d[i], 1, 1, &stream);
			break;
		case 2:
			append_s_stamp(current_s, 2, 3, 3, &stream);
			current_s = 2;
			normalize_append_d(d[i], 3, 2, &stream);
			break;
		case 3:
			append_s_stamp(current_s, 3, 4, 3, &stream);
			current_s = 3;
			normalize_append_d(d[i], 7, 3, &stream);
			break;
		case 4:
			append_s_stamp(current_s, 4, 5, 3, &stream);
			current_s = 4;
			normalize_append_d(d[i], 15, 4, &stream);
			break;
		case 5:
			append_s_stamp(current_s, 5, 6, 3, &stream);
			current_s = 5;
			normalize_append_d(d[i], 31, 5, &stream);
			break;
		case 6:
			append_s_stamp(current_s, 6, 14, 4, &stream);
			current_s = 6;
			normalize_append_d(d[i], 63, 6, &stream);
			break;
		case 7:
			append_s_stamp(current_s, 7, 30, 5, &stream);
			current_s = 7;
			normalize_append_d(d[i], 127, 7, &stream);
			break;
		case 8:
			append_s_stamp(current_s, 8, 62, 6, &stream);
			current_s = 8;
			normalize_append_d(d[i], 255, 8, &stream);
			break;
		case 9:
			append_s_stamp(current_s, 9, 126, 7, &stream);
			current_s = 9;
			normalize_append_d(d[i], 511, 9, &stream);
			break;
		case 10:
			append_s_stamp(current_s, 10, 254, 8, &stream);
			current_s = 10;
			normalize_append_d(d[i], 1023, 10, &stream);
			break;
		case 11:
			append_s_stamp(current_s, 11, 510, 9, &stream);
			current_s = 11;
			normalize_append_d(d[i], 2047, 11, &stream);
			break;
		case 12:
			append_s_stamp(current_s, 12, 1022, 10, &stream);
			current_s = 12;
			normalize_append_d(d[i], 4095, 12, &stream);
			break;
		case 13:
			append_s_stamp(current_s, 13, 2046, 11, &stream);
			current_s = 13;
			normalize_append_d(d[i], 8191, 13, &stream);
			break;
		case 14:
			append_s_stamp(current_s, 14, 4094, 12, &stream);
			current_s = 14;
			normalize_append_d(d[i], 16383, 14, &stream);
			break;
		}
	}
	bitstream_write_close(&stream);
	return stream.stream_used_len;
}

static inline int read_renormalize_val(int bits, int lim, int norm,
		bitstream_state_t *stream) {
	unsigned long long val;
	bitstream_read_bits(stream, &val, bits);
	return ((int)val < lim ? (int)val - norm : (int)val);
}

static inline void decode_current_s(int *d, int current_s,
		bitstream_state_t *stream) {

	switch (current_s) {
	case 0:
		*d = 0;
		break;
	case 1:
		*d = read_renormalize_val(1, 1, 1, stream);
		break;
	case 2:
		*d = read_renormalize_val(2, 2, 3, stream);
		break;
	case 3:
		*d = read_renormalize_val(3, 4, 7, stream);
		break;
	case 4:
		*d = read_renormalize_val(4, 8, 15, stream);
		break;
	case 5:
		*d = read_renormalize_val(5, 16, 31, stream);
		break;
	case 6:
		*d = read_renormalize_val(6, 32, 63, stream);
		break;
	case 7:
		*d = read_renormalize_val(7, 64, 127, stream);
		break;
	case 8:
		*d = read_renormalize_val(8, 128, 255, stream);
		break;
	case 9:
		*d = read_renormalize_val(9, 256, 511, stream);
		break;
	case 10:
		*d = read_renormalize_val(10, 512, 1023, stream);
		break;
	case 11:
		*d = read_renormalize_val(11, 1024, 2047, stream);
		break;
	case 12:
		*d = read_renormalize_val(12, 2048, 4095, stream);
		break;
	case 13:
		*d = read_renormalize_val(13, 4096, 8191, stream);
		break;
	case 14:
		*d = read_renormalize_val(14, 8192, 16383, stream);
		break;
	}
}

void slec_decode(unsigned char *input, int input_size, int *d, int size) {
	bitstream_state_t stream;
	bitstream_init(&stream, input, input_size);

	int i = 0;
	unsigned long long b;
	unsigned long long val;
	unsigned long long s;
	int current_s = 0;
	while (i < size) {
		bitstream_read_bits(&stream, &s, 2);
		if (s == 0) {
			decode_current_s(&d[i], current_s, &stream);
			i++;
		} else if (s == 1) {
			current_s++;
			decode_current_s(&d[i], current_s, &stream);
			i++;
		} else if (s == 2) {
			current_s--;
			decode_current_s(&d[i], current_s, &stream);
			i++;
		} else {

			b = 0;
			val = 0;
			bitstream_shift_read_bits(&stream, &b, 2);
			if (b == 0) {
				d[i++] = 0;
				current_s = 0;
			} else {
				bitstream_shift_read_bits(&stream, &b, 1);
				if (b < 7) {
					switch (b) {
					case 2:
						bitstream_read_bits(&stream, &val, 1);
						d[i] = val < 1 ? -1 : 1;
						current_s = 1;
						break;
					case 3:
						bitstream_read_bits(&stream, &val, 2);
						d[i] = val < 2 ? val - 3 : val;
						current_s = 2;
						break;
					case 4:
						bitstream_read_bits(&stream, &val, 3);
						d[i] = val < 4 ? val - 7 : val;
						current_s = 3;
						break;
					case 5:
						bitstream_read_bits(&stream, &val, 4);
						d[i] = val < 8 ? val - 15 : val;
						current_s = 4;
						break;
					case 6:
						bitstream_read_bits(&stream, &val, 5);
						d[i] = val < 16 ? val - 31 : val;
						current_s = 5;
						break;
					}

					i++;

				} else {
					bitstream_shift_read_bits(&stream, &b, 1);
					if (b == 14) {
						bitstream_read_bits(&stream, &val, 6);
						d[i++] = val < 32 ? val - 63 : val;
						current_s = 6;
					} else {
						bitstream_shift_read_bits(&stream, &b, 1);
						if (b == 30) {
							bitstream_read_bits(&stream, &val, 7);
							d[i++] = val < 64 ? val - 127 : val;
							current_s = 7;
						} else {
							bitstream_shift_read_bits(&stream, &b, 1);
							if (b == 62) {
								bitstream_read_bits(&stream, &val, 8);
								d[i++] = val < 128 ? val - 255 : val;
								current_s = 8;
							} else {
								bitstream_shift_read_bits(&stream, &b, 1);
								if (b == 126) {
									bitstream_read_bits(&stream, &val, 9);
									d[i++] = val < 256 ? val - 511 : val;
									current_s = 9;
								} else {
									bitstream_shift_read_bits(&stream, &b, 1);
									if (b == 254) {
										bitstream_read_bits(&stream, &val, 10);
										d[i++] = val < 512 ? val - 1023 : val;
										current_s = 10;
									} else {
										bitstream_shift_read_bits(&stream, &b,
												1);
										if (b == 510) {
											bitstream_read_bits(&stream, &val,
													11);
											d[i++] =
													val < 1024 ?
															val - 2047 : val;
											current_s = 11;
										} else {
											bitstream_shift_read_bits(&stream,
													&b, 1);
											if (b == 1022) {
												bitstream_read_bits(&stream,
														&val, 12);
												d[i++] =
														val < 2056 ?
																val - 4096 :
																val;
												current_s = 12;
											} else {
												bitstream_shift_read_bits(
														&stream, &b, 1);
												if (b == 2046) {
													bitstream_read_bits(&stream,
															&val, 13);
													d[i++] =
															val < 4096 ?
																	val - 8191 :
																	val;
													current_s = 13;
												} else {
													bitstream_shift_read_bits(
															&stream, &b, 1);
													bitstream_read_bits(&stream,
															&val, 14);
													d[i++] =
															val < 8192 ?
																	val
																			- 16383 :
																	val;
													current_s = 14;
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
	}

	for (int i = 1; i < size; i++)
		d[i] += d[i - 1];
}
