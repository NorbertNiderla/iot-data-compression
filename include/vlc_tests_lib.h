/*
 * tests_lib.h
 *
 *  Created on: Jun 22, 2021
 *      Author: Norbert Niderla
 */

#ifndef COMPRESSION_IOT_INCLUDE_VLC_TESTS_LIB_H_
#define COMPRESSION_IOT_INCLUDE_VLC_TESTS_LIB_H_

#define FA_LEC_ENABLE	(1)
#define STOJ_ENABLE		(1)
#define SPRINTZ_VARIATIONS_ON	(0)

void print_r_api_coders_results(const int* data, char* data_label);
void print_r_api_tmt_results(const int* data, char* data_label);
void print_rake_positions(const int* data);

#endif /* COMPRESSION_IOT_INCLUDE_VLC_TESTS_LIB_H_ */
