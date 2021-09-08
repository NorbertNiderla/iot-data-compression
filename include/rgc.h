/*
 * rgc.h
 *
 *  Created on: Jul 1, 2021
 *      Author: norbert
 */

#ifndef COMPRESSION_IOT_INCLUDE_RGC_H_
#define COMPRESSION_IOT_INCLUDE_RGC_H_

int rgc_encode(int *d, int size, unsigned char *output, int output_buffer_size, int dc_value);
void rgc_decode(unsigned char *input, int input_size, int *d, int size);

#endif /* COMPRESSION_IOT_INCLUDE_RGC_H_ */
