//by Norbert Niderla 2021

//FA-LEC by Vecchio, 2014

#include <string.h>

#include "include/fa_lec.h"
#include "include/bitstream.h"

#define ENABLE_DEBUG    (0)
#if ENABLE_DEBUG
#include <stdio.h>
#pragma message "fa_lec: debug enabled"
#endif

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

static int codewords[DICTIONARY_L]= {0, 2, 4, 6, 0x1e, 0x7e, 0x1fe, 0x7fe, 0xffe, 0x3fe, 0xfe, 0x3e, 14, 5, 3};
static int len[DICTIONARY_L] = {2, 3, 3, 3, 5, 7, 9, 11, 12, 10, 8, 6, 4, 3, 3};
static int norm[DICTIONARY_L] = {0, 1, 3, 7, 15, 31, 63, 127, 255, 511, 1023, 2047, 4095, 8191, 16383};

static int rotary_ptr[DICTIONARY_L] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14};
static int group_freq[DICTIONARY_L] = { 0 };

static void reset_rotary_ptr(void){
	for(int i = 0; i < DICTIONARY_L; i++){
		rotary_ptr[i] = i;
	}
}

static void set_rotary_ptr(int target_group){
	for(int i = 0, k = target_group; i < DICTIONARY_L; i++){
		rotary_ptr[k++] = i;
		if(k >= DICTIONARY_L) k = 0;
	}
}

static void reset_group_freq(void){
	memset(group_freq, 0, DICTIONARY_L*sizeof(int));
}

int fa_lec_encode(int* d, int size, unsigned char* output, int output_size, int dc_value){

#if ENABLE_DEBUG
	printf("DATA ENCODED: ");
	for (int i = 0; i < size; i++)
		printf("%d, ", d[i]);
	printf("\n");
#endif

	reset_rotary_ptr();
	reset_group_freq();

	bitstream_state_t stream;
	bitstream_init(&stream, output, output_size);

	int log;
	int most_frequent_group = 0;

	for (int i = (size - 1); i > 0; i--) d[i] -= d[i - 1];
	d[0] -= dc_value;

	for (int i = 0; i < size; i++) {
		log = compute_binary_log2(d[i]);

		 bitstream_append_bits(&stream, codewords[rotary_ptr[log]], len[rotary_ptr[log]]);
		d[i] = d[i] > 0 ? d[i] : d[i] + norm[log];
		 bitstream_append_bits(&stream, d[i], log);

		group_freq[log]++;
		if(group_freq[log] > group_freq[most_frequent_group]){
			set_rotary_ptr(log);
			most_frequent_group = log;
		}
	}

	 bitstream_write_close(&stream);
	return(stream.stream_used_len);
}

static int fa_lec_decode_value(bitstream_state_t* stream, int group, int* d){
	int log = 0;
	while(log < DICTIONARY_L){
		if(group == rotary_ptr[log]){
			break;
		} else {
			log++;
		}
	}
	unsigned long long val;
	if(log == 0){
		*d = 0;
	}else{
		bitstream_read_bits(stream, &val, log);
		*d = val < (unsigned long long)(1 << (log - 1)) ?  val - norm[log] : val;
	}
	return log;
}

void fa_lec_decode(unsigned char *input, int input_size, int *d, int size) {
	bitstream_state_t stream;
	bitstream_init(&stream, input, input_size);

	reset_rotary_ptr();
	reset_group_freq();

	int most_frequent_group = 0;
	int log = 0;
	unsigned long long b;

	for(int i = 0; i < size; i++) {
		b = 0;
		bitstream_shift_read_bits(&stream, &b, 2);
		if (b == (unsigned long long )codewords[0]) {
			log = fa_lec_decode_value(&stream, 0, &d[i]);
		} else {
			bitstream_shift_read_bits(&stream, &b, 1);
			if (b < 7) {
				switch (b) {
				case 2:
					log = fa_lec_decode_value(&stream, 1, &d[i]);
					break;
				case 4:
					log = fa_lec_decode_value(&stream, 2, &d[i]);
					break;
				case 5:
					log = fa_lec_decode_value(&stream, 13, &d[i]);
					break;
				case 3:
					log = fa_lec_decode_value(&stream, 14, &d[i]);
					break;
				}
			} else {
				bitstream_shift_read_bits(&stream, &b, 1);
				if (b == 14) {
					log = fa_lec_decode_value(&stream, 12, &d[i]);
				} else {
					bitstream_shift_read_bits(&stream, &b, 1);
					if (b == 30) {
						log = fa_lec_decode_value(&stream, 4, &d[i]);
					} else {
						bitstream_shift_read_bits(&stream, &b, 1);
						if (b == 62) {
							log = fa_lec_decode_value(&stream, 11, &d[i]);
						} else {
							bitstream_shift_read_bits(&stream, &b, 1);
							if (b == 126) {
								log = fa_lec_decode_value(&stream, 5, &d[i]);
							} else {
								bitstream_shift_read_bits(&stream, &b, 1);
								if (b == 254) {
									log = fa_lec_decode_value(&stream, 10, &d[i]);
								} else {
									bitstream_shift_read_bits(&stream, &b, 1);
									if (b == 510) {
										log = fa_lec_decode_value(&stream, 6, &d[i]);
									} else {
										bitstream_shift_read_bits(&stream, &b, 1);
										if (b == 1022) {
											log = fa_lec_decode_value(&stream, 9, &d[i]);
										} else {
											bitstream_shift_read_bits(&stream, &b, 1);
											if (b == 2046) {
												log = fa_lec_decode_value(&stream, 7, &d[i]);
											} else {
												bitstream_shift_read_bits(&stream, &b, 1);
												log = fa_lec_decode_value(&stream, 8, &d[i]);
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

		group_freq[log]++;
		if(group_freq[log] > group_freq[most_frequent_group]){
			set_rotary_ptr(log);
			most_frequent_group = log;
		}
	}

    for(int i = 1; i < size; i++) d[i] += d[i-1];
#if ENABLE_DEBUG
	printf("DATA DECODED: ");
	for (int i = 0; i < size; i++)
		printf("%d, ", d[i]);
	printf("\n");
#endif
}

