/*
 * mindiff.h
 *
 *  Created on: Jun 24, 2021
 *      Author: Norbert Niderla
 */

#ifndef COMPRESSION_IOT_INCLUDE_MINDIFF_H_
#define COMPRESSION_IOT_INCLUDE_MINDIFF_H_

void mindiff_decode(unsigned char *input, int input_buffer_size, int *data, int size);
int mindiff_encode(int *data, int size, unsigned char *output, int output_buffer_size);

#endif /* COMPRESSION_IOT_INCLUDE_MINDIFF_H_ */
