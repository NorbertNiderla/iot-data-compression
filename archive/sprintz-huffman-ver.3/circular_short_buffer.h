/*
 * circular_short_buffer.h
 *
 *  Created on: Aug 16, 2020
 *      Author: Norbert Niderla
 */
#pragma once

typedef struct{
	unsigned long long buffer;
	int bits;
}short_buffer;

void short_buffer_append_bits(short_buffer* buff, unsigned long long value, int bits);
unsigned long long short_buffer_get_bits(short_buffer* buff, int bits);
