/*
 * nd.h
 *
 *  Created on: Jun 23, 2021
 *      Author: Norbert Niderla
 */

#ifndef COMPRESSION_IOT_INCLUDE_ND_H_
#define COMPRESSION_IOT_INCLUDE_ND_H_

#define ENCODER_DV_VALUE    (1)

#if ENCODER_DC_VALUE
int nd_encode(int *data, int size, unsigned char *output, int output_buffer_size, int dc_value);
#else
int nd_encode(int *data, int size, unsigned char *output, int output_buffer_size);
#endif
void nd_decode(unsigned char *input, int input_size, int *d, int size);


#endif /* COMPRESSION_IOT_INCLUDE_ND_H_ */
