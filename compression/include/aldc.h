/*
 * aldc.h
 *
 *  Created on: Jun 22, 2021
 *      Author: Norbert Niderla
 */

#ifndef COMPRESSION_IOT_INCLUDE_ALDC_H_
#define COMPRESSION_IOT_INCLUDE_ALDC_H_

#define ENCODER_DV_VALUE    (1)

#if ENCODER_DC_VALUE
int aldc_encode(int *d, int size, unsigned char *output, int output_size, int dc_value);
#else
int aldc_encode(int *d, int size, unsigned char *output, int output_size);
#endif
void aldc_decode(unsigned char *input, int input_size, int *output, int size);

#endif /* COMPRESSION_IOT_INCLUDE_ALDC_H_ */
