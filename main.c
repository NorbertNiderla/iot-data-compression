//by Norbert Niderla /2021

#include <stdio.h>

#include "data.h"
#include "array_lib.h"

#define STANDARD		(0)
#define STANDARD_FRAME_COMPRESSION	(1)
#define TMT_STANDARD	(0)

#define FIRE_DIFF	(0)

#define RAKE_POS_PRINT	(0)
#define RGC_TESTING		(0)
#define ND_TESTING		(0)
#define MINDIFF_TESTING	(0)
#define FA_LEC_TESTING	(0)
#define STOJ_TESTING	(0)
#define ARITH_TESTING	(0)
#define TMT_TESTING		(0)
#define AAC_TESTING		(0)
#define SPRINTZ_TESTING	(0)
#define HUFFMAN_TESTING	(1)
#define TANS_TESTING 	(0)
#define RICE_TESTING	(0)

#if STANDARD | TMT_STANDARD
#include "tests_lib.h"
#elif STANDARD_FRAME_COMPRESSION
#include "include/tests_lib.h"
#define ENABLE_DEBUG	(1)
#include "debug.h"
#define BOARD 1
//#define DATA_FNC	get_cond_data_ptr(0)
//#define FRAME_LOW_LIMIT		55
#elif FIRE_DIFF
#include "include/fire.h"
#elif RAKE_POS_PRINT
#include "rake.h"
#elif RGC_TESTING
#include "rgc.h"
#elif ND_TESTING
#include "nd.h"
#elif MINDIFF_TESTING
#include "mindiff.h"
#elif FA_LEC_TESTING
#include "fa_lec.h"
#elif STOJ_TESTING
#include "stoj.h"
#elif ARITH_TESTING
#include "arithmetic.h"
#elif TMT_TESTING
#include "tmt.h"
#include <stdlib.h>
#elif AAC_TESTING
#include "aac.h"
#elif SPRINTZ_TESTING
#include "fire.h"
#include "sprintz.h"
#include "bitstream.h"
#include <string.h>
#elif HUFFMAN_TESTING
#include "huffman.h"
#include <string.h>
#elif TANS_TESTING
#include "tans.h"
#include <string.h>
#include "bitstream.h"
#elif RICE_TESTING
#include "rice.h"
#include <string.h>
#endif



int main(void) {
#if STANDARD
	char ph_label[] = "ph";
	char cond_label[] = "conduct";
	char temp_label[] = "temp";
	char gas_label[] = "gas";
	char vol_label[] = "vol";
	char water_label[] = "water";
	char heart_label[] = "heart";
	char hum_label[] = "hum";
	char light_label[] = "light";

	
	printf("data <- data.frame(\"Coder\" = c(\"ND\", \"LEC\", \"SLEC\", \"ALDC\",\"MinDiff\", \"RAKE\", \"RGC\",");
#if FA_LEC_ENABLE
	printf("\"FA-LEC\", ");
#endif
#if STOJ_ENABLE
	printf("\"STOJ\", ");
#endif
	printf("\"Sprintz\", ");
#if SPRINTZ_VARIATIONS_ON
	printf("\"SprintANS\", \"FiRice\", ");
#endif
	printf("\"TMT\"),\n");

	print_r_api_coders_results(get_heart_data_ptr(0), heart_label); printf(",\n");
	print_r_api_coders_results(get_light_data_ptr(0), light_label); printf(",\n");
	print_r_api_coders_results(get_hum_data_ptr(0), hum_label); printf(",\n");
	print_r_api_coders_results(get_water_data_ptr(0), water_label); printf(",\n");
	print_r_api_coders_results(get_vol_data_ptr(0), vol_label); printf(",\n");
	print_r_api_coders_results(get_gas_data_ptr(0), gas_label); printf(",\n");
	print_r_api_coders_results(get_temp_data_ptr(0), temp_label); printf(",\n");
	print_r_api_coders_results(get_cond_data_ptr(0), cond_label); printf(",\n");
	print_r_api_coders_results(get_ph_data_ptr(0), ph_label); printf(")\n");

#elif FIRE_DIFF
	unsigned long long  sum[N_DATASETS] = { 0 };
	int buffer[250] = {0};
	fire_coder_t fire_state;
	fire_init(FIRE_LEARN_SHIFT, FIRE_BIT_WIDTH, &fire_state);
	for(int i = 0; i < N_DATASETS; i++){
		const int* data = get_data_ptr(0, i);
		for(int x = 0; x < 250; x++) buffer[x] = data[x];
		fire_encode(buffer, 250, &fire_state);
		fire_reset(&fire_state);
		for(int x = 1; x < 250; x++) sum[i] += (buffer[x]*buffer[x]);

		printf("dataset_%d <- c(", i);
		for(int k = 1; k < 250; k++){
			printf("%d", buffer[k]);
			if(k < 250-1) printf(", ");
			if((k%25)==0) printf("\n");
		}
		printf(")\n");

	}
	printf("sum <- c(");
	for(int i = 0; i < N_DATASETS; i++){
		printf("%lld", sum[i]);
		if(i < N_DATASETS-1)
			printf(", ");
	}
	printf(")\n");

#elif STANDARD_FRAME_COMPRESSION
	unsigned bytes[4][N_DATASETS] = { 0 };
	unsigned samples[4][N_DATASETS] = { 0 };
	unsigned time[4][N_DATASETS] = { 0 };

#if BOARD == 0
	get_sprintz_revised_results(bytes, samples, time, 64, 81);
	print_r_api_sprintz_revised_results(samples, "samples");
	print_r_api_sprintz_revised_results(bytes, "bytes");
	print_r_api_sprintz_revised_results(time, "time");
#elif BOARD == 1
	get_sprintz_revised_results(bytes, samples, time, 180, 210);
	print_r_api_sprintz_revised_results(samples, "samples");
	print_r_api_sprintz_revised_results(bytes, "bytes");
	print_r_api_sprintz_revised_results(time, "time");
#endif

#elif RICE_TESTING
	int int_buffer[17] = {0};
	const int* data = get_heart_data_ptr(0);
	for(int i = 0; i<17; i++) int_buffer[i] = data[i];
	unsigned char stream[40] = {0};
	print_data_w_label(int_buffer, 17, "Data before encoding:");

	bitstream_state_t stream_state;
	bitstream_init(&stream_state, stream, 40);
	rice_encode(int_buffer, 17, &stream_state);
	bitstream_write_close(&stream_state);
	memset(int_buffer, 0, 17*sizeof(int));
	unsigned n_samples = rice_decode(stream, stream_state.stream_used_len, int_buffer);
	print_data_w_label(int_buffer, n_samples, "Data after_decoding:");

#elif TANS_TESTING
	int int_buffer[8] = {0};
	for(int x = 0; x < 9; x++){
		const int* data = get_heart_data_ptr(0);
		for(int i = 0; i< 8; i++) int_buffer[i] = data[i];
		unsigned char stream[64] = {0};
		print_data_w_label(int_buffer, 8, "Data before encoding:");
		//printf("ENCODING:\n");
		unsigned bytes = tans_encode((unsigned char*)int_buffer, 8*sizeof(int), stream, 64);
		memset(int_buffer, 0, 8*sizeof(int));
		//printf("DECODING:\n");
		tans_decode(stream, bytes, (unsigned char*)int_buffer);
		print_data_w_label(int_buffer, 8, "Data after_decoding:");
		printf("bytes: %d\n", bytes);
	}
#elif HUFFMAN_TESTING
	int int_buffer[8] = {0};
		for(int x = 0; x < 9; x++){
			const int* data = get_heart_data_ptr(0);
			for(int i = 0; i< 8; i++) int_buffer[i] = data[i];
			unsigned char stream[64] = {0};
			print_data_w_label(int_buffer, 8, "Data before encoding:");
			//printf("ENCODING:\n");
			unsigned bytes = huffman_encode((unsigned char*)int_buffer, 8*sizeof(int), stream, 64);
			memset(int_buffer, 0, 8*sizeof(int));
			//printf("DECODING:\n");
			huffman_decode(stream, bytes, (unsigned char*)int_buffer);
			print_data_w_label(int_buffer, 8, "Data after_decoding:");
			printf("bytes: %d\n", bytes);
		}
#elif SPRINTZ_TESTING
	fire_coder_t fire_state;
	fire_init(FIRE_LEARN_SHIFT, FIRE_BIT_WIDTH, &fire_state);
	int int_buffer[64] = {0};
	const int* data = get_water_data_ptr(0);
	for(int i = 0; i< 64; i++) int_buffer[i] = data[i];

	unsigned char stream[50] = {0};
	bitstream_state_t stream_state;
	bitstream_init(&stream_state, stream, 100);

	print_data_w_label(int_buffer, 64, "Data before encoding:");
	sprintz_encode(int_buffer, 64, &stream_state, 1);
	bitstream_write_close(&stream_state);

	unsigned bytes = stream_state.stream_used_len;
	memset(int_buffer, 0, 64*sizeof(int));
	bitstream_init(&stream_state, stream, 100);

	print_char_data_w_label(stream, bytes, "Encoded stream:");
	sprintz_decode(&stream_state, bytes, int_buffer, 1);
	print_data_w_label(int_buffer, 64, "Data after decoding:");

	printf("\nbytes:%d\n", bytes);

#elif TMT_STANDARD
	char ph_label[] = "ph";
	char cond_label[] = "conduct";
	char temp_label[] = "temp";
	char gas_label[] = "gas";
	char vol_label[] = "vol";
	char water_label[] = "water";
	char heart_label[] = "heart";
	char hum_label[] = "hum";
	char light_label[] = "light";


		printf("data <- data.frame(\"Coder\" = c(\"TMT\"),\n");
		print_r_api_tmt_results(get_heart_data_ptr(0), heart_label); printf(",\n");
		print_r_api_tmt_results(get_light_data_ptr(0), light_label); printf(",\n");
		print_r_api_tmt_results(get_hum_data_ptr(0), hum_label); printf(",\n");
		print_r_api_tmt_results(get_water_data_ptr(0), water_label); printf(",\n");
		print_r_api_tmt_results(get_vol_data_ptr(0), vol_label); printf(",\n");
		print_r_api_tmt_results(get_gas_data_ptr(0), gas_label); printf(",\n");
		print_r_api_tmt_results(get_temp_data_ptr(0), temp_label); printf(",\n");
		print_r_api_tmt_results(get_cond_data_ptr(0), cond_label); printf(",\n");
		print_r_api_tmt_results(get_ph_data_ptr(0), ph_label); printf(")\n");

#elif RAKE_POS_PRINT
	printf("rake_pos <- data.frame(");
	printf("\"ph\" = c(");
	print_rake_positions(get_ph_data_ptr(0));
	printf("),\n");

	printf("\"conduct\" = c(");
	print_rake_positions(get_cond_data_ptr(0));
	printf("),\n");

	printf("\"temp\" = c(");
	print_rake_positions(get_temp_data_ptr(0));
	printf("),\n");

	printf("\"vol\" = c(");
	print_rake_positions(get_vol_data_ptr(0));
	printf("),\n");

	printf("\"gas\" = c(");
	print_rake_positions(get_gas_data_ptr(0));
	printf("),\n");

	printf("\"water\" = c(");
	print_rake_positions(get_water_data_ptr(0));
	printf("),\n");

	printf("\"hum\" = c(");
	print_rake_positions(get_hum_data_ptr(0));
	printf("),\n");

	printf("\"light\" = c(");
	print_rake_positions(get_light_data_ptr(0));
	printf("),\n");

	printf("\"heart\" = c(");
	print_rake_positions(get_heart_data_ptr(0));
	printf("),\n");

#elif RGC_TESTING

	int int_buffer[32] = {0};
	unsigned char buffer[1000] = {0};
	const int* data = get_heart_data_ptr(0);
	for(int x = 0; x < 768; x+=32){
	for(int i = 0; i< 32; i++) int_buffer[i] = data[x + i];
		rgc_encode(int_buffer, 32, buffer, 1000);
		clearIntArray(int_buffer, 32);
		rgc_decode(buffer, 1000, int_buffer, 32);
		clearCharArray(buffer, 1000);
		printf("\n");
	}

#elif ND_TESTING
	int int_buffer[32] = {0};
	unsigned char buffer[100] = {0};
	const int* data = get_vol_data_ptr(0);
	for(int x = 0; x < 768; x+=32){
	for(int i = 0; i< 32; i++) int_buffer[i] = data[x + i];
		nd_encode(int_buffer, 32, buffer, 100);
		clearIntArray(int_buffer, 32);
		nd_decode(buffer, 100, int_buffer, 32);
		clearCharArray(buffer, 100);
		printf("\n");
	}

#elif MINDIFF_TESTING
	int int_buffer[32] = {0};
	unsigned char buffer[100] = {0};
	const int* data = get_temp_data_ptr(0);
	for(int x = 0; x < 768; x+=32){
	for(int i = 0; i< 32; i++) int_buffer[i] = data[x + i];
		mindiff_encode(int_buffer, 32, buffer, 100);
		clearIntArray(int_buffer, 32);
		mindiff_decode(buffer, 100, int_buffer, 32);
		clearCharArray(buffer, 100);
		printf("\n");
	}

#elif FA_LEC_TESTING
	int int_buffer[32] = {0};
	unsigned char buffer[100] = {0};
	const int* data = get_heart_data_ptr(0);
	for(int x = 0; x < 768; x+=32){
	for(int i = 0; i< 32; i++) int_buffer[i] = data[x + i];
		fa_lec_encode(int_buffer, 32, buffer, 100);
		clearIntArray(int_buffer, 32);
		fa_lec_decode(buffer, 100, int_buffer, 32);
		clearCharArray(buffer, 100);
		printf("\n");
	}

#elif STOJ_TESTING
	int int_buffer[32] = {0};
	unsigned char buffer[100] = {0};
	const int* data = get_temp_data_ptr(0);
	for(int x = 0; x < 768; x+=32){
	for(int i = 0; i< 32; i++) int_buffer[i] = data[x + i];
		stoj_encode(int_buffer, 32, buffer, 100);
		clearIntArray(int_buffer, 32);
		stoj_decode(buffer, 100, int_buffer, 32);
		clearCharArray(buffer, 100);
		printf("\n");
	}
#elif ARITH_TESTING
/*
	#define SIZE (30)

	unsigned counts[] = {10, 3, 2, 5, 1, 1, 4, 3, 1, 2};
	set_bounds(10, counts);
	print_bounds();

	unsigned data[SIZE] = { 1, 3, 0, 0, 0, 2, 0, 1, 5,9,2,6,1,5,2,9,8,3,1,0,0,0,0,5,1,2,5,3,7,6 };
	unsigned decoded_data[SIZE] = {0};
	unsigned stream[50] = {0};
	for(int i = 0; i<SIZE;i++) printf("%d ", data[i]);
	printf("\n\n");
	unsigned bits = arithmetic_encode(data, SIZE, stream);
	

	arithmetic_decode(stream, bits, decoded_data, SIZE);
	for(int i = 0; i<SIZE;i++)printf("%d ", decoded_data[i]);
	printf("\n\n");
	printf("bits on symbol: %f", (float)bits/(float)SIZE);
*/
#define SIZE (6)

set_bounds_w_laplace_distribution(11, 9);
/*
unsigned counts[] = {1000, 300, 200, 500};
set_bounds(4, counts);
print_bounds();

unsigned data[SIZE] = { 1, 3, 0, 0, 0, 2};
unsigned decoded_data[SIZE] = {0};
unsigned stream[50] = {0};
for(int i = 0; i<SIZE;i++) printf("%d ", data[i]);
printf("\n\n");
unsigned bits = arithmetic_encode(data, SIZE, stream);


arithmetic_decode(stream, bits, decoded_data, SIZE);
for(int i = 0; i<SIZE;i++)printf("%d ", decoded_data[i]);
printf("\n\n");
printf("bits on symbol: %f", (float)bits/(float)SIZE);

*/
#elif TMT_TESTING

int data[9] = {700, 693, 699, 700,701,698,710,709,707};
int decode_data[20] = {0};
unsigned char buffer[30] = {0};
tmt_data_t encoder_output = tmt_encode(data, 9, buffer, 20, 700);
printf("\nR: %d, M: %d, bits: %d, ps: %d\n", encoder_output.R,
											encoder_output.M,
											encoder_output.bits,
											encoder_output.placeholded_symbols_nr);

tmt_decode(buffer, decode_data, 9, encoder_output);
printf("\n");
for(int i = 0; i < 9; i++) printf("%d ", data[i]);
printf("\n\n");
for(int i = 0; i < 9; i++) printf("%d ", decode_data[i]);
printf("\n\n");

/*
	int int_buffer[40] = {0};
	const int * data[9];
	data[0] = get_ph_data_ptr(0);
	data[1] = get_temp_data_ptr(0);
	data[2] = get_cond_data_ptr(0);
	data[3] = get_gas_data_ptr(0);
	data[4] = get_vol_data_ptr(0);
	data[5] = get_hum_data_ptr(0);
	data[6] = get_light_data_ptr(0);
	data[7] = get_heart_data_ptr(0);
	data[8] = get_water_data_ptr(0);

	char ph_label[] = "ph";
	char cond_label[] = "conduct";
	char temp_label[] = "temp";
	char gas_label[] = "gas";
	char vol_label[] = "vol";
	char water_label[] = "water";
	char heart_label[] = "heart";
	char hum_label[] = "hum";
	char light_label[] = "light";

	char* label_ptr[] = {ph_label, temp_label, cond_label, gas_label, vol_label, hum_label, light_label, heart_label, water_label};
	unsigned char stream[120] = {0};
	for(int p = 0; p < 9; p++){
		printf("%s:\n", label_ptr[p]);
		int encoder_dc_value = (data[p])[0];	
		for(int x = 0; x < DATA_SIZE; x+=40){
			for(int i = 0; i< 40; i++) int_buffer[i] = (data[p])[x + i];
			tmt_data_t encoder_output = tmt_encode(int_buffer, 40, stream, 120, encoder_dc_value);
			encoder_dc_value = (data[p])[x + 40 - 1];
			printf("R: %d, M: %d, bits: %d, ps: %d\n", encoder_output.R,
														encoder_output.M,
														encoder_output.bits,
														encoder_output.placeholded_symbols_nr);
			if(encoder_output.placeholded_symbols_nr != 0){
				free(encoder_output.placeholded_symbols);
			}
		}
		printf("\n\n");

	}
*/
#elif AAC_TESTING
	#define SIZE (30)

	adaptive_arithmetic_set_number_of_symbols(10);

	unsigned data[SIZE] = { 0, 3, 0, 0, 0, 2, 0, 1, 5,0,2,6,1,5,2,9,0,0,1,0,0,0,0,5,1,2,5,0,7,6 };
	unsigned decoded_data[SIZE] = {0};
	unsigned char stream[50] = {0};
	for(int i = 0; i<SIZE;i++) printf("%d ", data[i]);
	printf("\n\n");
	unsigned bytes = adaptive_arithmetic_encode(data, SIZE, stream, 50);


	adaptive_arithmetic_decode(stream, bytes<<3, decoded_data, SIZE);
	for(int i = 0; i<SIZE;i++)printf("%d ", decoded_data[i]);
	printf("\n\n");
	printf("bits on symbol: %f", (float)(bytes << 3)/(float)SIZE);
#endif

	return 0;
}
