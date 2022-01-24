#ifndef COMPRESSION_IOT_ARRAY_LIB_H
#define COMPRESSION_IOT_ARRAY_LIB_H

void print_data(int* data, int size);
void print_data_w_label(int* data, int size, char* label);
void print_char_data_w_label(unsigned char* data, int size, char* label);

/*
void convertCharToInt(unsigned char* input, int size, int* output);
void convertIntToChar(int* input, int size, unsigned char* output);
void checkResult(int* data, int* result, int size);
*/

void clearCharArray(unsigned char* arr, int size);
void clearIntArray(int* arr, int size);


#endif
