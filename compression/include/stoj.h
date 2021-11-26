/*
 * stoj.h
 *
 *  Created on: Jul 14, 2021
 *      Author: Norbert Niderla
 */

#ifndef COMPRESSION_IOT_INCLUDE_STOJ_H_
#define COMPRESSION_IOT_INCLUDE_STOJ_H_

int stoj_encode(int *data, int size, unsigned char *output, int output_buffer_size, int dc_value);
void stoj_decode(unsigned char *input, int input_buffer_size, int *data, int size);

#endif /* COMPRESSION_IOT_INCLUDE_STOJ_H_ */
