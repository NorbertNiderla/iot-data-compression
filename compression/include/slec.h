/*
 * lec.h
 *
 *  Created on: Jun 17, 2021
 *      Author: Norbert Niderla
 */

#ifndef COMPRESSION_IOT_INCLUDE_SLEC_H_
#define COMPRESSION_IOT_INCLUDE_SLEC_H_

int slec_encode(int *d, int size, unsigned char *output, int output_size, int dc_value);
void slec_decode(unsigned char *input, int input_size, int *d, int size);


#endif /* COMPRESSION_IOT_INCLUDE_SLEC_H_ */
