#pragma once

int riceEncode(int* n, int size, unsigned char* output, size_t output_buffer_size);
void riceDecode(unsigned char* input, int size, int* n);
void printRiceHist(void);
