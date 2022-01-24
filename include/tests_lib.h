/*
 * tests_lib.h
 *
 *  Created on: Sep 22, 2021
 *      Author: Norbert Niderla
 *      		Warsaw University of Technology
 */

#ifndef COMPRESSION_IOT_INCLUDE_TESTS_LIB_H_
#define COMPRESSION_IOT_INCLUDE_TESTS_LIB_H_

#include "data.h"

void get_sprintz_revised_results(unsigned bytes[][N_DATASETS], unsigned samples[][N_DATASETS], unsigned time[][N_DATASETS], int frame_low_limit, int frame_high_limit);
void print_r_api_sprintz_revised_results(unsigned results[][N_DATASETS], char* label);
#endif /* COMPRESSION_IOT_INCLUDE_TESTS_LIB_H_ */
