//by Norbert Niderla, 2021

#include "include/vlc_tests_lib.h"

#include <stdio.h>
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include "compression_iot_definitions.h"

//utility
#include "xtimer.h"
#include "bitstream.h"

//big coders
#include "fire.h"
#include "huffman.h"
#include "rice.h"
#include "sprintz.h"
#include "tans.h"

//data store
#include "data.h"

//small coders
#include "aldc.h"
#include "lec.h"
#include "slec.h"
#include "nd.h"
#include "mindiff.h"
#include "rake.h"
#include "rgc.h"
#include "fa_lec.h"
#include "stoj.h"
#include "tmt.h"

//array manipulation functions
#include "array_lib.h"

#define ENABLE_DEBUG (0)

#define INT_BUFFER_SIZE		(40)
#define CHAR_BUFFER_SIZE	(200)

#if ENCODER_DC_VALUE
static void get_small_coder_results(const int *data,
		int (*encode_fun)(int*, int, unsigned char*, int, int),
		int* result_bytes,
		int* result_time) {

	int int_buffer[INT_BUFFER_SIZE] = { 0 };
	unsigned char buffer[CHAR_BUFFER_SIZE] = { 0 };
	int idx = 0;
	xtimer_ticks32_t sum_time;
	sum_time.ticks32 = 0;
	int bytes = 0;
	int sum_bytes = 0;
	int encoder_dc_value = data[0];

	while (idx < DATA_SIZE) {
		for (int i = 0; i < INT_BUFFER_SIZE; i++) int_buffer[i] = data[idx + i];

		int irq_state = irq_disable();
		xtimer_ticks32_t one = xtimer_now();

		bytes = encode_fun(int_buffer, INT_BUFFER_SIZE, buffer, CHAR_BUFFER_SIZE, encoder_dc_value);

		xtimer_ticks32_t two = xtimer_now();
		irq_restore(irq_state);

		if(bytes == (-1)){
			*result_bytes = 0;
			*result_time = 0;
			return;
		}
		sum_bytes += bytes;
		sum_time.ticks32 += xtimer_diff(two, one).ticks32;

		clearCharArray(buffer, CHAR_BUFFER_SIZE);
		clearIntArray(int_buffer, INT_BUFFER_SIZE);
		encoder_dc_value = data[idx + INT_BUFFER_SIZE - 1];
		idx += INT_BUFFER_SIZE;

	}

	*result_bytes = sum_bytes;
	*result_time = sum_time.ticks32;
}

static void get_rake_results(const int* data, int* result_bytes, int* result_time){
	int16_t int_buffer[INT_BUFFER_SIZE] = { 0 };
	unsigned char buffer[CHAR_BUFFER_SIZE] = { 0 };
	int idx = 0;
	xtimer_ticks32_t sum_time;
	sum_time.ticks32 = 0;
	int bytes = 0;
	int encoder_dc_value = data[0];

	while (idx < DATA_SIZE) {
		for (int i = 0; i < INT_BUFFER_SIZE; i++)
			int_buffer[i] = (int16_t)data[idx + i];

		int irq_state = irq_disable();
		xtimer_ticks32_t one = xtimer_now();

		bytes += rake_encode(int_buffer, INT_BUFFER_SIZE, buffer, CHAR_BUFFER_SIZE, encoder_dc_value);

		xtimer_ticks32_t two = xtimer_now();
		irq_restore(irq_state);
		
		sum_time.ticks32 += xtimer_diff(two, one).ticks32;

		clearCharArray(buffer, CHAR_BUFFER_SIZE);
		for (int i = 0; i < INT_BUFFER_SIZE; i++)
			int_buffer[i] = 0;

		encoder_dc_value = data[idx + INT_BUFFER_SIZE - 1];
		idx += INT_BUFFER_SIZE;
	}

	*result_bytes = bytes;
	*result_time = sum_time.ticks32;
}

static void get_tmt_results(const int* data, int* result_bits, int* result_time){
	int int_buffer[INT_BUFFER_SIZE] = { 0 };
	unsigned char buffer[CHAR_BUFFER_SIZE] = { 0 };
	int idx = 0;
	xtimer_ticks32_t sum_time;
	sum_time.ticks32 = 0;
	int bits = 0;
	int encoder_dc_value = data[0];
	tmt_data_t encoder_output;

	while (idx < DATA_SIZE) {
		for (int i = 0; i < INT_BUFFER_SIZE; i++)
			int_buffer[i] = (int)data[idx + i];

		int irq_state = irq_disable();
		xtimer_ticks32_t one = xtimer_now();

		encoder_output = tmt_encode(int_buffer, INT_BUFFER_SIZE, buffer, CHAR_BUFFER_SIZE, encoder_dc_value);

		xtimer_ticks32_t two = xtimer_now();
		irq_restore(irq_state);

		bits += encoder_output.bits;
		bits += encoder_output.placeholded_symbols_nr*16;
		sum_time.ticks32 += xtimer_diff(two, one).ticks32;

		clearCharArray(buffer, CHAR_BUFFER_SIZE);
		for (int i = 0; i < INT_BUFFER_SIZE; i++)
			int_buffer[i] = 0;

		encoder_dc_value = data[idx + INT_BUFFER_SIZE - 1];
		idx += INT_BUFFER_SIZE;

	}

	*result_bits = bits;
	*result_time = sum_time.ticks32;
}

void print_rake_positions(const int* data){
	int16_t int_buffer[INT_BUFFER_SIZE] = { 0 };
	unsigned char buffer[CHAR_BUFFER_SIZE] = { 0 };
	int idx = 0;
	int encoder_dc_value = data[0];
	while (idx < DATA_SIZE) {
		for (int i = 0; i < INT_BUFFER_SIZE; i++)
			int_buffer[i] = (int16_t)data[idx + i];

		int irq_state = irq_disable();
		rake_encode(int_buffer, INT_BUFFER_SIZE, buffer, CHAR_BUFFER_SIZE, encoder_dc_value);
		irq_restore(irq_state);
		clearCharArray(buffer, CHAR_BUFFER_SIZE);
		for (int i = 0; i < INT_BUFFER_SIZE; i++)
			int_buffer[i] = 0;
		encoder_dc_value = data[idx + INT_BUFFER_SIZE - 1];
		idx += INT_BUFFER_SIZE;
	}
}
#else
static void get_small_coder_results(const int *data,
		int (*encode_fun)(int*, int, unsigned char*, int),
		int* result_bytes,
		int* result_time) {

	int int_buffer[INT_BUFFER_SIZE] = { 0 };
	unsigned char buffer[CHAR_BUFFER_SIZE] = { 0 };
	int idx = 0;
	xtimer_ticks32_t sum_time;
	sum_time.ticks32 = 0;
	int bytes = 0;
	int sum_bytes = 0;
	while (idx < DATA_SIZE) {
		for (int i = 0; i < INT_BUFFER_SIZE; i++)
			int_buffer[i] = data[idx + i];

		xtimer_ticks32_t one = xtimer_now();

		bytes = encode_fun(int_buffer, INT_BUFFER_SIZE, buffer, CHAR_BUFFER_SIZE);

		xtimer_ticks32_t two = xtimer_now();
		if(bytes == (-1)){
			*result_bytes = 0;
			*result_time = 0;
			return;
		}
		sum_bytes += bytes;
		sum_time.ticks32 += xtimer_diff(two, one).ticks32;

		clearCharArray(buffer, CHAR_BUFFER_SIZE);
		clearIntArray(int_buffer, INT_BUFFER_SIZE);
		idx += INT_BUFFER_SIZE;
	}

	*result_bytes = sum_bytes;
	*result_time = sum_time.ticks32;
}

static void get_rake_results(const int* data, int* result_bytes, int* result_time){
	int16_t int_buffer[INT_BUFFER_SIZE] = { 0 };
	unsigned char buffer[CHAR_BUFFER_SIZE] = { 0 };
	int idx = 0;
	xtimer_ticks32_t sum_time;
	sum_time.ticks32 = 0;
	int bytes = 0;
	while (idx < DATA_SIZE) {
		for (int i = 0; i < INT_BUFFER_SIZE; i++)
			int_buffer[i] = (int16_t)data[idx + i];

		xtimer_ticks32_t one = xtimer_now();

		bytes += rake_encode(int_buffer, INT_BUFFER_SIZE, buffer, CHAR_BUFFER_SIZE);

		xtimer_ticks32_t two = xtimer_now();
		
		sum_time.ticks32 += xtimer_diff(two, one).ticks32;

		clearCharArray(buffer, CHAR_BUFFER_SIZE);
		for (int i = 0; i < INT_BUFFER_SIZE; i++)
			int_buffer[i] = 0;

		idx += INT_BUFFER_SIZE;
	}

	*result_bytes = bytes;
	*result_time = sum_time.ticks32;
}

void print_rake_positions(const int* data){
	int16_t int_buffer[INT_BUFFER_SIZE] = { 0 };
	unsigned char buffer[CHAR_BUFFER_SIZE] = { 0 };
	int idx = 0;
	while (idx < DATA_SIZE) {
		for (int i = 0; i < INT_BUFFER_SIZE; i++)
			int_buffer[i] = (int16_t)data[idx + i];

		rake_encode(int_buffer, INT_BUFFER_SIZE, buffer, CHAR_BUFFER_SIZE);

		clearCharArray(buffer, CHAR_BUFFER_SIZE);
		for (int i = 0; i < INT_BUFFER_SIZE; i++)
			int_buffer[i] = 0;

		idx += INT_BUFFER_SIZE;
	}
}
#endif

static void get_mindiff_results(const int *data, int* result_bytes,
		int* result_time) {

	int int_buffer[INT_BUFFER_SIZE] = { 0 };
	unsigned char buffer[CHAR_BUFFER_SIZE] = { 0 };
	int idx = 0;
	xtimer_ticks32_t sum_time;
	sum_time.ticks32 = 0;
	int bytes = 0;
	int sum_bytes = 0;

	while (idx < DATA_SIZE) {
		for (int i = 0; i < INT_BUFFER_SIZE; i++)
			int_buffer[i] = data[idx + i];

		int irq_state = irq_disable();
		xtimer_ticks32_t one = xtimer_now();

		bytes = mindiff_encode(int_buffer, INT_BUFFER_SIZE, buffer, CHAR_BUFFER_SIZE);

		xtimer_ticks32_t two = xtimer_now();
		irq_restore(irq_state);

		if(bytes == (-1)){
			*result_bytes = 0;
			*result_time = 0;
			return;
		}
		sum_bytes += bytes;
		sum_time.ticks32 += xtimer_diff(two, one).ticks32;

		clearCharArray(buffer, CHAR_BUFFER_SIZE);
		clearIntArray(int_buffer, INT_BUFFER_SIZE);
		idx += INT_BUFFER_SIZE;

	}

	*result_bytes = sum_bytes;
	*result_time = sum_time.ticks32;
}

static fire_coder_t fire_state;

static void sprintz_tans_testing(const int *data, int stack_n,
		int table_build_coeff, int* result_bits, int* result_time) {
	xtimer_ticks32_t one;
	xtimer_ticks32_t two;
	xtimer_ticks32_t sum_time;

	int sum_bits = 0;
	sum_time.ticks32 = 0;
	int idx = 0;
	int int_buffer[INT_BUFFER_SIZE] = {0};
	unsigned char char_buffer[CHAR_BUFFER_SIZE] = { 0 };
	fire_coder_init(FIRE_LEARN_SHIFT, FIRE_BIT_WIDTH, &fire_state);
	fire_state.first = data[0];
	bool new_table;


	while (idx < DATA_SIZE) {
		for (int i = 0; i < INT_BUFFER_SIZE; i++)
			int_buffer[i] = data[idx + i];
		one = xtimer_now();

		if (idx % (stack_n * table_build_coeff) == 0) {
			new_table = true;
		} else
			new_table = false;

		int bits = sprintzEncode_tans(int_buffer, stack_n, char_buffer,
				CHAR_BUFFER_SIZE, new_table, &fire_state);
		two = xtimer_now();

		clearCharArray(char_buffer, CHAR_BUFFER_SIZE);
		idx += stack_n;
		sum_bits += bits;
		sum_time.ticks32 += xtimer_diff(two, one).ticks32;
	}

	*result_time = sum_time.ticks32;
	*result_bits = sum_bits;
}

static void sprintz_testing(const int *data, int stack_n, int table_build_coeff, int* result_bits, int* result_time) {
	xtimer_ticks32_t one;
	xtimer_ticks32_t two;
	xtimer_ticks32_t sum_time;

	int sum_bits = 0;
	sum_time.ticks32 = 0;
	int idx = 0;
	int int_buffer[INT_BUFFER_SIZE] = {0};
	unsigned char char_buffer[CHAR_BUFFER_SIZE] = { 0 };
	fire_init(FIRE_LEARN_SHIFT, FIRE_BIT_WIDTH, &fire_state);
	fire_state.first = data[0];
	bool new_table;
	while (idx < DATA_SIZE) {
		for (int i = 0; i < INT_BUFFER_SIZE; i++)
			int_buffer[i] = data[idx + i];

		int irq_state = irq_disable();
		one = xtimer_now();

		if (idx % (stack_n * table_build_coeff) == 0) {
			new_table = true;
		} else
			new_table = false;

		int bits = sprintzEncode(int_buffer, stack_n, char_buffer, CHAR_BUFFER_SIZE,
				new_table, &fire_state);
		two = xtimer_now();
		irq_restore(irq_state);

		clearCharArray(char_buffer, CHAR_BUFFER_SIZE);
		idx += stack_n;
		sum_bits += bits;
		sum_time.ticks32 += xtimer_diff(two, one).ticks32;
	}
	*result_bits = sum_bits;
	*result_time = sum_time.ticks32;

}
*/
/*
static void fire_rice_testing(const int *data, int stack_n, int* result_bits, int* result_time) {
	xtimer_ticks32_t one;
	xtimer_ticks32_t two;
	xtimer_ticks32_t sum_time;

	int int_buffer[INT_BUFFER_SIZE] = { 0 };
	unsigned char char_buffer[CHAR_BUFFER_SIZE] = { 0 };

	int sum_bits = 0;
	sum_time.ticks32 = 0;
	int idx = 0;
	fire_coder_init(FIRE_LEARN_SHIFT, FIRE_BIT_WIDTH, &fire_state);
	fire_state.first = data[0];

	while (idx < DATA_SIZE) {
		for (int i = 0; i < INT_BUFFER_SIZE; i++)
			int_buffer[i] = data[idx + i];
		one = xtimer_now();

		fireEncode(&data[idx], stack_n, int_buffer, &fire_state);
		int bits = riceEncodeStream(int_buffer, stack_n, char_buffer,
				CHAR_BUFFER_SIZE);

		two = xtimer_now();

		sum_bits += bits;
		sum_time.ticks32 += xtimer_diff(two, one).ticks32;

		clearCharArray(char_buffer, CHAR_BUFFER_SIZE);
		clearIntArray(int_buffer, INT_BUFFER_SIZE);
		idx += stack_n;
	}
	*result_bits = sum_bits;
	*result_time = sum_time.ticks32;
}

static void get_placebo_results(const int* data, int* result_bytes, int* result_time){
	int int_buffer[INT_BUFFER_SIZE] = { 0 };
	unsigned char buffer[CHAR_BUFFER_SIZE] = { 0 };
	int idx = 0;
	xtimer_ticks32_t sum_time;
	sum_time.ticks32 = 0;
	int bytes = 0;
	int sum_bytes = 0;
	int encoder_dc_value = data[0];

	while (idx < DATA_SIZE) {
		for (int i = 0; i < INT_BUFFER_SIZE; i++) int_buffer[i] = data[idx + i];

		xtimer_ticks32_t one = xtimer_now();

		bytes = 0;
		for(int i = 0; i < INT_BUFFER_SIZE; i++){
			bytes += int_buffer[i];
		}

		bytes -= encoder_dc_value;

		xtimer_ticks32_t two = xtimer_now();
			if(bytes == (-1)){
				*result_bytes = 0;
				*result_time = 0;
				return;
			}
		sum_bytes += bytes;
		sum_time.ticks32 += xtimer_diff(two, one).ticks32;

		clearCharArray(buffer, CHAR_BUFFER_SIZE);
		clearIntArray(int_buffer, INT_BUFFER_SIZE);
		encoder_dc_value = data[idx + INT_BUFFER_SIZE - 1];
		idx += INT_BUFFER_SIZE;
	}
		*result_bytes = sum_bytes;
		*result_time = sum_time.ticks32;
}

void print_r_api_coders_results(const int* data, char* data_label){
	int nd_bytes = 0, lec_bytes = 0, slec_bytes = 0, aldc_bytes = 0, mindiff_bytes = 0, rake_bytes = 0, rgc_bytes = 0, sprintz_bits = 0;
	int nd_time = 0, lec_time = 0, slec_time = 0, aldc_time = 0, mindiff_time = 0, rake_time = 0, rgc_time = 0, sprintz_time = 0;
	int tmt_bits = 0;
	int tmt_time = 0;

	get_placebo_results(data, &nd_bytes, &nd_time);
	get_small_coder_results(data, &nd_encode, &nd_bytes, &nd_time);
	get_small_coder_results(data, &lec_encode, &lec_bytes, &lec_time);
	get_small_coder_results(data, &slec_encode, &slec_bytes, &slec_time);
	get_small_coder_results(data, &aldc_encode, &aldc_bytes, &aldc_time);
	get_mindiff_results(data, &mindiff_bytes, &mindiff_time);
	get_rake_results(data, &rake_bytes, &rake_time);
	get_small_coder_results(data, &rgc_encode, &rgc_bytes, &rgc_time);
#if FA_LEC_ENABLE
	int fa_lec_bytes = 0;
	int fa_lec_time = 0;
	get_small_coder_results(data, &fa_lec_encode, &fa_lec_bytes, &fa_lec_time);
#endif
#if STOJ_ENABLE
	int stoj_bytes = 0;
	int stoj_time = 0;
	get_small_coder_results(data, &stoj_encode, &stoj_bytes, &stoj_time);
#endif
	sprintz_testing(data, INT_BUFFER_SIZE, 1, &sprintz_bits, &sprintz_time);
#if SPRINTZ_VARIATIONS_ON
	int sprintz_tans_bits = 0, fire_rice_bits = 0;
	int sprintz_tans_time = 0, fire_rice_time = 0;
	sprintz_tans_testing(data, INT_BUFFER_SIZE, 1, &sprintz_tans_bits, &sprintz_tans_time);
	fire_rice_testing(data, INT_BUFFER_SIZE, &fire_rice_bits, &fire_rice_time);
#endif
	get_tmt_results(data, &tmt_bits, &tmt_time);

	printf("\"%s_bits\" = c(c(%d, %d, %d, %d, %d, %d, %d",
			data_label, nd_bytes, lec_bytes, slec_bytes, aldc_bytes, mindiff_bytes, rake_bytes, rgc_bytes);
#if FA_LEC_ENABLE
	printf(", %d", fa_lec_bytes);
#endif
#if STOJ_ENABLE
	printf(", %d", stoj_bytes);
#endif
	printf(")*8, %d, ", sprintz_bits);
#if SPRINTZ_VARIATIONS_ON
	printf("%d, %d, ", sprintz_tans_bits, fire_rice_bits);
#endif
	printf("%d)/%d,\n", tmt_bits, DATA_SIZE);


	printf("\"%s_time\" = c(%d, %d, %d, %d, %d, %d, %d",
			data_label, nd_time, lec_time, slec_time, aldc_time, mindiff_time, rake_time, rgc_time);
#if FA_LEC_ENABLE
	printf(", %d", fa_lec_time);
#endif
#if STOJ_ENABLE
	printf(", %d", stoj_time);
#endif
	printf(", %d, ",  sprintz_time);
#if SPRINTZ_VARIATIONS_ON
	printf("%d, %d, ", sprintz_tans_time, fire_rice_time);
#endif
	printf("%d)/%d", tmt_time, DATA_SIZE);
}

void print_r_api_tmt_results(const int* data, char* data_label){
	int tmt_bits = 0;
	int tmt_time = 0;
	get_tmt_results(data, &tmt_bits, &tmt_time);

	printf("\"%s_bits\" = c(", data_label);
	printf("%d)/%d,\n", tmt_bits, DATA_SIZE);
	printf("\"%s_time\" = c(",data_label);
	printf("%d)/%d", tmt_time, DATA_SIZE);
}

