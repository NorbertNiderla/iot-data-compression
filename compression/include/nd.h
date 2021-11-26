/*
 * nd.h
 *
 *  Created on: Jun 23, 2021
 *      Author: Norbert Niderla
 */

#ifndef COMPRESSION_IOT_INCLUDE_ND_H_
#define COMPRESSION_IOT_INCLUDE_ND_H_

int nd_encode(int *data, int size, unsigned char *output, int output_buffer_size, int dc_value);
void nd_decode(unsigned char *input, int input_size, int *d, int size);


#endif /* COMPRESSION_IOT_INCLUDE_ND_H_ */
