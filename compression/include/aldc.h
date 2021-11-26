/*
 * aldc.h
 *
 *  Created on: Jun 22, 2021
 *      Author: Norbert Niderla
 */

#ifndef COMPRESSION_IOT_INCLUDE_ALDC_H_
#define COMPRESSION_IOT_INCLUDE_ALDC_H_

int aldc_encode(int *d, int size, unsigned char *output, int output_size, int dc_value);
void aldc_decode(unsigned char *input, int input_size, int *output, int size);

#endif /* COMPRESSION_IOT_INCLUDE_ALDC_H_ */
