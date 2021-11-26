/*
 * lec.h
 *
 *  Created on: Jun 17, 2021
 *      Author: Norbert Niderla
 */

#ifndef COMPRESSION_IOT_INCLUDE_LEC_H_
#define COMPRESSION_IOT_INCLUDE_LEC_H_

int lec_encode(int *d, int size, unsigned char *output, int output_size, int dc_value);
int lec_table_encode(int* d, int size, unsigned char* output, int output_size, int dc_value);
void lec_decode(unsigned char *input, int input_size, int *d, int size);

#endif /* COMPRESSION_IOT_INCLUDE_LEC_H_ */
