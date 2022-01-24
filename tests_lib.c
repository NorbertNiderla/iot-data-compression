#include "include/frame_compression.h"
#include "include/tests_lib.h"
#include "include/data.h"
#include "xtimer.h"

#define ENABLE_DEBUG	(1)
#include "debug.h"

#define MAIN_BUFFER_SIZE        (220)
#define ADDITIONAL_BUFFER_SIZE  (250)

static unsigned char buffer[MAIN_BUFFER_SIZE] = { 0 };
static unsigned char output_buffer[ADDITIONAL_BUFFER_SIZE] = { 0 };

#define MANY_SAMPLES	(1)
#if MANY_SAMPLES
#define SAMPLES_LIMIT	(900)
#else
#define SAMPLES_LIMIT	(700)
#endif


#if ACK_TRANSMISSION_ENCODING
void get_sprintz_revised_results(unsigned bytes[][N_DATASETS],
		unsigned samples[][N_DATASETS], unsigned time[][N_DATASETS],
		int frame_low_limit, int frame_high_limit) {
	int data_idx = 0;
	int bytes_res = 0;
	xtimer_ticks32_t one, two;
	unsigned reset_state = 0;
#if MANY_SAMPLES
	unsigned starting_data_idx = 0;
#endif
	/* FIRE RICE*/
	DEBUG("fire_rice\n");
	for (int dataset_id = 0; dataset_id < N_DATASETS; dataset_id++) {
		reset_state = 1;
#if MANY_SAMPLES
		starting_data_idx = 0;
#endif
		while (data_idx < SAMPLES_LIMIT) {
			one = xtimer_now();
#if !MANY_SAMPLES
			data_idx += get_fire_rice_frame(buffer,
					get_data_ptr(data_idx, dataset_id), frame_low_limit,
					frame_high_limit, &bytes_res, reset_state);
#else
			data_idx += get_fire_rice_frame(buffer,
								get_data_ptr(starting_data_idx++, dataset_id), frame_low_limit,
								frame_high_limit, &bytes_res, reset_state);
#endif
			two = xtimer_now();
			time[0][dataset_id] += xtimer_diff(two, one).ticks32;
			bytes[0][dataset_id] += bytes_res;
			reset_state = 0;
		}
#if !MANY_SAMPLES
		if (data_idx > DATA_SIZE)
			DEBUG(
					"tests_lib.c: get_sprintz_revised_bytes: data_idx out of bounds\n");
#endif
		samples[0][dataset_id] = data_idx;
		data_idx = 0;
	}

	/* SPRINTZ */
	DEBUG("sprintz\n");
	for (int dataset_id = 0; dataset_id < N_DATASETS; dataset_id++) {
		reset_state = 1;
#if MANY_SAMPLES
		starting_data_idx = 0;
#endif
		while (data_idx < SAMPLES_LIMIT) {
			one = xtimer_now();
#if !MANY_SAMPLES
			data_idx += get_sprintz_frame(buffer,
					get_data_ptr(data_idx, dataset_id), frame_low_limit,
					frame_high_limit, &bytes_res, reset_state);
#else
			data_idx += get_sprintz_frame(buffer,
					get_data_ptr(starting_data_idx++, dataset_id), frame_low_limit,
					frame_high_limit, &bytes_res, reset_state);
#endif
			two = xtimer_now();
			time[1][dataset_id] += xtimer_diff(two, one).ticks32;
			bytes[1][dataset_id] += bytes_res;
			reset_state = 0;
		}
#if !MANY_SAMPLES
		if (data_idx > DATA_SIZE)
			DEBUG(
					"tests_lib.c: get_sprintz_revised_bytes: data_idx out of bounds\n");
#endif
		samples[1][dataset_id] = data_idx;
		data_idx = 0;
	}

	/*SPRINTZ HUFFMAN*/
	DEBUG("sprintz_huffman\n");
	for (int dataset_id = 0; dataset_id < N_DATASETS; dataset_id++) {
		reset_state = 1;
#if MANY_SAMPLES
		starting_data_idx = 1;
#endif
		data_idx += get_sprintz_huffman_frame(buffer,
				get_data_ptr(data_idx, dataset_id), frame_low_limit,
				frame_high_limit, &bytes_res, output_buffer,
				ADDITIONAL_BUFFER_SIZE, reset_state);
		bytes[2][dataset_id] += bytes_res;
		while (data_idx < SAMPLES_LIMIT) {
			one = xtimer_now();
#if !MANY_SAMPLES
			data_idx += get_sprintz_huffman_frame(buffer,
					get_data_ptr(data_idx, dataset_id), frame_low_limit,
					frame_high_limit, &bytes_res, output_buffer,
					ADDITIONAL_BUFFER_SIZE, reset_state);
#else
			data_idx += get_sprintz_huffman_frame(buffer,
					get_data_ptr(starting_data_idx++, dataset_id), frame_low_limit,
					frame_high_limit, &bytes_res, output_buffer,
					ADDITIONAL_BUFFER_SIZE, reset_state);
#endif
			two = xtimer_now();
			time[2][dataset_id] += xtimer_diff(two, one).ticks32;
			bytes[2][dataset_id] += bytes_res;
			reset_state = 0;
		}
#if !MANY_SAMPLES
		if (data_idx > DATA_SIZE)
			DEBUG(
					"tests_lib.c: get_sprintz_revised_bytes: data_idx out of bounds\n");
#endif
		samples[2][dataset_id] = data_idx;
		data_idx = 0;
	}

	/* SPRINTZ TANS*/
	DEBUG("sprintz_tans\n");
	for (int dataset_id = 0; dataset_id < N_DATASETS; dataset_id++) {
		DEBUG("--dataset_%d--\n", dataset_id);
		reset_state = 1;
#if MANY_SAMPLES
		starting_data_idx = 0;
#endif
		while (data_idx < SAMPLES_LIMIT) {
			one = xtimer_now();
#if !MANY_SAMPLES
			data_idx += get_sprintz_tans_frame(buffer,
					get_data_ptr(data_idx, dataset_id), frame_low_limit,
					frame_high_limit, &bytes_res, output_buffer,
					ADDITIONAL_BUFFER_SIZE, reset_state);
#else
			data_idx += get_sprintz_tans_frame(buffer,
					get_data_ptr(starting_data_idx++, dataset_id), frame_low_limit,
					frame_high_limit, &bytes_res, output_buffer,
					ADDITIONAL_BUFFER_SIZE, reset_state);
#endif
			two = xtimer_now();
			time[3][dataset_id] += xtimer_diff(two, one).ticks32;
			bytes[3][dataset_id] += bytes_res;
			reset_state = 0;
		}
#if !MANY_SAMPLES
		if (data_idx > DATA_SIZE)
			DEBUG(
					"tests_lib.c: get_sprintz_revised_bytes: data_idx out of bounds\n");
#endif
		samples[3][dataset_id] = data_idx;
		data_idx = 0;
	}
}

#endif
#if SINGLE_FRAME_ENCODING
void get_sprintz_revised_results(unsigned bytes[][N_DATASETS],
		unsigned samples[][N_DATASETS], unsigned time[][N_DATASETS],
		int frame_low_limit, int frame_high_limit) {
	int data_idx = 0;
	int bytes_res = 0;
	xtimer_ticks32_t one, two;

	/* FIRE RICE*/
	DEBUG("fire_rice\n");
	for (int dataset_id = 0; dataset_id < N_DATASETS; dataset_id++) {
		DEBUG("--dataset %d--\n", dataset_id);
		while (data_idx < SAMPLES_LIMIT) {
			one = xtimer_now();
#if !MANY_SAMPLES
			data_idx += get_fire_rice_frame(buffer,
					get_data_ptr(data_idx, dataset_id), frame_low_limit,
					frame_high_limit, &bytes_res);
#else
			data_idx += get_fire_rice_frame(buffer,
								get_data_ptr(0, dataset_id), frame_low_limit,
								frame_high_limit, &bytes_res);
#endif
			two = xtimer_now();
			time[0][dataset_id] += xtimer_diff(two, one).ticks32;
			bytes[0][dataset_id] += bytes_res;
		}
#if !MANY_SAMPLES
		if (data_idx > DATA_SIZE)
			DEBUG(
					"tests_lib.c: get_sprintz_revised_bytes: data_idx out of bounds\n");
#endif
		samples[0][dataset_id] = data_idx;
		data_idx = 0;
	}

	/* SPRINTZ */
	DEBUG("sprintz\n");
	for (int dataset_id = 0; dataset_id < N_DATASETS; dataset_id++) {
		while (data_idx < SAMPLES_LIMIT) {
			one = xtimer_now();
#if !MANY_SAMPLES
			data_idx += get_sprintz_frame(buffer,
					get_data_ptr(data_idx, dataset_id), frame_low_limit,
					frame_high_limit, &bytes_res);
#else
			data_idx += get_sprintz_frame(buffer,
					get_data_ptr(0, dataset_id), frame_low_limit,
					frame_high_limit, &bytes_res);
#endif
			two = xtimer_now();
			time[1][dataset_id] += xtimer_diff(two, one).ticks32;
			bytes[1][dataset_id] += bytes_res;
		}
#if !MANY_SAMPLES
		if (data_idx > DATA_SIZE)
			DEBUG(
					"tests_lib.c: get_sprintz_revised_bytes: data_idx out of bounds\n");
#endif
		samples[1][dataset_id] = data_idx;
		data_idx = 0;
	}

	/*SPRINTZ HUFFMAN*/
	DEBUG("sprintz_huffman\n");
	int frames = 0;
	for (int dataset_id = 0; dataset_id < N_DATASETS; dataset_id++) {
		data_idx += get_sprintz_huffman_frame(buffer,
				get_data_ptr(data_idx, dataset_id), frame_low_limit,
				frame_high_limit, &bytes_res, 1, output_buffer,
				ADDITIONAL_BUFFER_SIZE);
		bytes[2][dataset_id] += bytes_res;
		frames = 1;
		while (data_idx < SAMPLES_LIMIT) {
			one = xtimer_now();
#if !MANY_SAMPLES
			data_idx += get_sprintz_huffman_frame(buffer,
					get_data_ptr(data_idx, dataset_id), frame_low_limit,
					frame_high_limit, &bytes_res, 0, output_buffer,
					ADDITIONAL_BUFFER_SIZE);
#else
			data_idx += get_sprintz_huffman_frame(buffer,
					get_data_ptr(0, dataset_id), frame_low_limit,
					frame_high_limit, &bytes_res, 0, output_buffer,
					ADDITIONAL_BUFFER_SIZE);
#endif
			two = xtimer_now();
			time[2][dataset_id] += xtimer_diff(two, one).ticks32;
			if(frames == 1){
				time[2][dataset_id] <<= 1;
				frames++;
			}
			bytes[2][dataset_id] += bytes_res;
		}
#if !MANY_SAMPLES
		if (data_idx > DATA_SIZE)
			DEBUG(
					"tests_lib.c: get_sprintz_revised_bytes: data_idx out of bounds\n");
#endif
		samples[2][dataset_id] = data_idx;
		data_idx = 0;
	}

	/* SPRINTZ TANS*/
	DEBUG("sprintz_tans\n");
	for (int dataset_id = 0; dataset_id < N_DATASETS; dataset_id++) {
		while (data_idx < SAMPLES_LIMIT) {
			one = xtimer_now();
#if !MANY_SAMPLES
			data_idx += get_sprintz_tans_frame(buffer,
					get_data_ptr(data_idx, dataset_id), frame_low_limit,
					frame_high_limit, &bytes_res, output_buffer,
					ADDITIONAL_BUFFER_SIZE);
#else
			data_idx += get_sprintz_tans_frame(buffer,
					get_data_ptr(0, dataset_id), frame_low_limit,
					frame_high_limit, &bytes_res, output_buffer,
					ADDITIONAL_BUFFER_SIZE);
#endif
			two = xtimer_now();
			time[3][dataset_id] += xtimer_diff(two, one).ticks32;
			bytes[3][dataset_id] += bytes_res;
		}
#if !MANY_SAMPLES
		if (data_idx > DATA_SIZE)
			DEBUG(
					"tests_lib.c: get_sprintz_revised_bytes: data_idx out of bounds\n");
#endif
		samples[3][dataset_id] = data_idx;
		data_idx = 0;
	}
}
#endif
void print_r_api_sprintz_revised_results(unsigned results[][N_DATASETS], char* label) {
	printf("%s <- data.frame(\"Fire+Rice\"= c(", label);
	for (int i = 0; i < N_DATASETS; i++) {
		printf("%d", results[0][i]);
		if (i < N_DATASETS - 1) {
			printf(", ");
		}
	}
	printf("),\n");

	printf("\"Sprintz\"= c(");
	for (int i = 0; i < N_DATASETS; i++) {
		printf("%d", results[1][i]);
		if (i < N_DATASETS - 1) {
			printf(", ");
		}
	}
	printf("),\n");

	printf("\"Sprintz+Huffman\"= c(");
	for (int i = 0; i < N_DATASETS; i++) {
		printf("%d", results[2][i]);
		if (i < N_DATASETS - 1) {
			printf(", ");
		}
	}
	printf("),\n");

	printf("\"Sprintz+tANS\"= c(");
	for (int i = 0; i < N_DATASETS; i++) {
		printf("%d", results[3][i]);
		if (i < N_DATASETS - 1) {
			printf(", ");
		}
	}
	printf("),\n");
	printf("\"dataset\" = c(\"pH\", \"Temp\", \"Counductivity\", \"Heart\", \"Gas\", \"Voltage\", \"Water\", \"Humidity\", \"Light\"");
	printf("))\n");
}
