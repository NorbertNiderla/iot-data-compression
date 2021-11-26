/*
 * aac.h
 *
 *  Created on: Aug 3, 2021
 *      Author: Norbert Niderla
 */

#ifndef COMPRESSION_IOT_INCLUDE_AAC_H_
#define COMPRESSION_IOT_INCLUDE_AAC_H_

void adaptive_arithmetic_decode(unsigned char* input, int bits, unsigned* data, int size);
unsigned adaptive_arithmetic_encode(unsigned* data, int size, unsigned char* output, int output_buffer_size);
void adaptive_arithmetic_set_number_of_symbols(unsigned number_of_symbols);

#endif /* COMPRESSION_IOT_INCLUDE_AAC_H_ */
