/*
 * rake.h
 *
 *  Created on: Jun 29, 2021
 *      Author: Norbert Niderla
 */

#ifndef COMPRESSION_IOT_INCLUDE_RAKE_H_
#define COMPRESSION_IOT_INCLUDE_RAKE_H_

int rake_encode(int16_t* data, int size, unsigned char* output, int output_buffer_size, int dc_value);
void rake_decode(unsigned char* input, int input_size, int16_t* output, int output_size);

#endif /* COMPRESSION_IOT_INCLUDE_RAKE_H_ */
