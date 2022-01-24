#include "../../../../compression_iot/archive/source/board_2_dim_new/entropy.h"

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#define SYMBOLS 256
static int buffer[SYMBOLS] = {0};

void Entropy_AddToHist(unsigned char* symbols, int size){
	for(int k =0; k<size; k++)
		buffer[symbols[k]]++;
}

void Entropy_AddToHistInt(int* symbols, int size){
	for(int k =0; k<size; k++)
		buffer[symbols[k]]++;
}

void Entropy_ResetBuffer(){
	for(int k=0;k<SYMBOLS;k++)
		buffer[k]=0;
}

float Entropy_CalculateEntropy(){
	int sum = 0;
	for(int k=0; k<SYMBOLS;k++)
		sum += buffer[k];

	float sum_f = (float)sum;
	float entropy = 0;
	float prob;
	for(int k=0;k<SYMBOLS;k++){
		prob = (float)buffer[k]/sum;
		entropy += prob*log(prob);
	}
}

