#pragma once


uint8_t rle(unsigned char* src, unsigned char* dst, int bits, unsigned int depth);

void rleDecode(unsigned char* src, unsigned char* dst, const int depth);
