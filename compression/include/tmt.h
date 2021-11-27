/*
 * tmt.h
 *
 *  Created on: Jul 21, 2021
 *      Author: Norbert Niderla
 */

#ifndef COMPRESSION_IOT_INCLUDE_TMT_H_
#define COMPRESSION_IOT_INCLUDE_TMT_H_

#define ENCODER_DC_VALUE    (1)

#define SIGN_SYMBOL	(999)
#define	PLACEHOLDER	(998)
#define SIGN_SYMBOL_OFFSET	(0)
#define PLACEHOLDER_OFFSET	(1)

#define PLACEHOLDED_SYMBOLS_ARRAY_LEN	(10)

typedef struct tmt_data{
	unsigned R;
	unsigned M;
	unsigned bits;
	unsigned arithmetic_encoded_symbols;
	unsigned placeholded_symbols_nr;
} tmt_data_t;

typedef struct tmt_placeholded_symbols{
	int symbols[PLACEHOLDED_SYMBOLS_ARRAY_LEN];
	unsigned len; 
}tmt_placeholded_symbols_t;

#if ENCODER_DC_VALUE
tmt_data_t tmt_encode(int *data, int size, unsigned char* output, int output_buffer_size, int dc_value);
#else
#endif
void tmt_decode(unsigned char* input, int* output, int size, tmt_data_t parameters);

#endif /* COMPRESSION_IOT_INCLUDE_TMT_H_ */
