/*
 * fa_lec.h
 *
 *  Created on: Jul 13, 2021
 *      Author: Norbert Niderla
 */

#ifndef COMPRESSION_IOT_INCLUDE_FA_LEC_H_
#define COMPRESSION_IOT_INCLUDE_FA_LEC_H_

#define ENCODER_DV_VALUE    (1)

#if ENCODER_DC_VALUE
int fa_lec_encode(int* d, int size, unsigned char* output, int output_size, int dc_value);
#else
int fa_lec_encode(int* d, int size, unsigned char* output, int output_size);
#endif
void fa_lec_decode(unsigned char *input, int input_size, int *d, int size);

#endif /* COMPRESSION_IOT_INCLUDE_FA_LEC_H_ */
