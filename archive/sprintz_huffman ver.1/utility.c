/*
 * utility.c
 *
 *  Created on: Aug 14, 2020
 *      Author: norbert
 */
#include "../../../compression_iot/archive/sprintz_huffman ver.1/utility.h"

#include <stdio.h>

static int histogram_arr[256] = {0};

void histogram(unsigned char* buff, int size)
{
	int histogram[256] = {0};
	for(int i=0;i<size;i++)
		histogram[buff[i]]++;

	for(int i=0;i<256;i++)
		printf("%d,",histogram[i]);

	printf("\n");
}

void histogramStream(unsigned long long value)
{
		histogram_arr[value]++;
}

void printHistogram(void)
{
		for(int i=0;i<256;i++)
			printf("%d,",histogram_arr[i]);
		printf("\n");
}
