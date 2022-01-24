#include "../../../../compression_iot/archive/source/board_time_2_dim/my_lib.h"

#include <stdio.h>
#include <stdlib.h>

void printData(int* data, int size){
    for(int i = 0; i<size; i++) printf("%d ",data[i]);
    printf("\n");
}

void printCharData(unsigned char* data, int size){
	for(int i = 0; i<size; i++) printf("%d ",data[i]);
	printf("\n");
}

void convertCharToInt(unsigned char* input, int size, int* output){
	for(int idx = 0; idx<size; idx++){
		output[idx] = (int)input[idx];
	}
}

void convertIntToChar(int* input, int size, unsigned char* output){
	for(int idx = 0; idx<size; idx++){
		output[idx] = (unsigned char)input[idx];
	}
}

void checkResult(int* data, int* result, int size){
	for(int idx=0;idx<size;idx++){
		if(data[idx]!=result[idx]) printf("%d ",idx);
	}
	printf("\n");
}

void clearCharArray(unsigned char* arr, int size){
	for(int idx=0;idx<size;idx++){
		arr[idx]=0;
	}
}

void clearIntArray(int* arr, int size){
	for(int idx=0;idx<size;idx++){
		arr[idx]=0;
	}
}
