/*
 * fa_lec.h
 *
 *  Created on: Jul 13, 2021
 *      Author: Norbert Niderla
 */

#ifndef COMPRESSION_IOT_INCLUDE_FA_LEC_H_
#define COMPRESSION_IOT_INCLUDE_FA_LEC_H_

int fa_lec_encode(int* d, int size, unsigned char* output, int output_size, int dc_value);
void fa_lec_decode(unsigned char *input, int input_size, int *d, int size);

#endif /* COMPRESSION_IOT_INCLUDE_FA_LEC_H_ */
