/*
 * arithmetic.h
 *
 *  Created on: Jul 23, 2021
 *      Author: Norbet Niderla
 */

#ifndef COMPRESSION_IOT_INCLUDE_ARITHMETIC_H_
#define COMPRESSION_IOT_INCLUDE_ARITHMETIC_H_

void set_bounds(unsigned number_of_symbols, unsigned* counts);
void print_bounds(void);
unsigned arithmetic_encode(unsigned* data, int size, unsigned* output_ptr, int output_buffer_size);
void arithmetic_decode(unsigned* input, int bits, unsigned* data, int size);
void set_bounds_w_laplace_distribution(unsigned number_of_symbols, int sd);

#define ERROR_SYMBOL	(-9999)
#define LAPLACE_DIST_PRECISION  (1000)
#endif /* COMPRESSION_IOT_INCLUDE_ARITHMETIC_H_ */
