#include "../../../compression_iot/archive/sprintz_huffman ver.1/huffman.h"

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>

#include "../../../compression_iot/archive/sprintz_huffman ver.1/bitstream.h"
#include "../../../compression_iot/archive/sprintz_huffman ver.1/circular_short_buffer.h"

#define NUMBERS 256
#define BITS 8

typedef struct node_t {
	uint16_t left;
	uint16_t right;
	uint16_t father;
	unsigned int freq;
	uint8_t symbol;
	unsigned int original : 1;
	unsigned int free : 1;
}node;

typedef struct {
	uint16_t symbol;
	unsigned int bits;
}lookup_element;

static int frequencies[NUMBERS] = {140,37,29,17,21,16,10,9,32,8,15,8,9,5,9,4,34,5,12,3,7,7,12,5,20,7,5,5,9,8,8,0,23,12,7,6,15,3,6,9,15,5,9,5,5,4,8,5,15,13,7,7,5,2,7,0,16,4,2,3,5,3,4,3,22,10,20,12,10,7,8,6,9,2,7,3,7,4,6,2,15,5,9,4,2,3,4,1,1,6,4,1,5,3,3,1,16,6,7,3,2,3,5,2,11,4,6,4,4,3,3,1,14,6,5,3,7,2,6,3,4,2,0,2,2,3,3,2,30,10,13,13,17,3,10,3,8,7,9,4,8,4,9,2,13,11,2,12,8,3,3,0,8,5,1,1,6,1,2,1,15,5,5,3,5,3,5,1,7,4,2,4,3,6,1,1,11,9,5,4,6,3,2,2,6,5,3,6,4,5,1,0,23,10,7,5,5,3,5,4,7,2,6,2,7,3,7,2,3,0,2,3,4,4,4,2,3,1,1,3,1,0,6,1,7,5,4,2,4,1,4,1,2,1,1,1,3,1,3,2,14,1,3,2,2,3,1,1,7,2,1,2,0,2,2,0};
static uint8_t numbers[NUMBERS];
static int size = 0;
static node tree[2*NUMBERS-1];
static lookup_element lookup_table[NUMBERS];

static uint16_t popFreeNode(void)
{
	volatile uint16_t index=0;
	volatile unsigned int lowest_val=10000;

	for(int i =0;i<size;i++)
		if(tree[i].free==1)
		{
			index = i;
			lowest_val = tree[i].freq;
			break;
		}		

	for(int i =index+1;i<size;i++)
	{
		if((tree[i].free==1)&(tree[i].freq<lowest_val))
		{
			index = i;
			lowest_val = tree[i].freq;
		}
	}
	tree[index].free=0;
	return index;
}

static uint16_t newFreeNode(uint16_t left, uint16_t right, uint16_t father, int freq, int symbol, unsigned int original)
{
	tree[size].father = father;
	tree[size].left = left;
	tree[size].right = right;
	tree[size].freq = freq;
	tree[size].symbol = symbol;
	tree[size].original = original;
	tree[size].free = 1;
	size++;
	return (uint16_t)(size-1);
}

void buildTree(void)
{
	for(int i =0;i<NUMBERS;i++)if(frequencies[i]==0)frequencies[i] = 1;
	for(int i =0;i<NUMBERS;i++)numbers[i] = i;

	for (int i = 0; i < NUMBERS; i++) newFreeNode(5555, 5555, 5555, frequencies[i], numbers[i], true);

	uint16_t left;
	uint16_t right;
	uint16_t new;

	while (size < 2*NUMBERS-1)
	{
		left = popFreeNode();
		right = popFreeNode();
		new = newFreeNode(left, right, 5555, tree[left].freq+tree[right].freq,0,0);
		tree[left].father = new;
		tree[right].father = new;
	}
	tree[2*NUMBERS-2].free=0;
}

void buildLookUpTable(void)
{
	for (int i = 0; i < NUMBERS; i++)
	{
		lookup_table[i].bits = 0;
		lookup_table[i].symbol = 0;
	}

	for (int i = 0; i < size; i++)
	{
		if (tree[i].original == 1)
		{
			uint16_t temp = i;
			int k = 0;
			while (tree[temp].father != 5555) 
			{	
				if (temp == tree[tree[temp].father].left)
				{
					lookup_table[tree[i].symbol].symbol |= (1<<k++);
					lookup_table[tree[i].symbol].bits++;
				}
				else if (temp == tree[tree[temp].father].right)
				{
					lookup_table[tree[i].symbol].symbol |= (0 << k++);
					lookup_table[tree[i].symbol].bits++;
				}

				temp = tree[temp].father;
			}
		}
	}
}

int huffmanByte(short_buffer* src, bitstream_state_t* dst)
{
	unsigned long long temp;
	temp = short_buffer_get_bits(src, 8);
	bitstream_append_bits(dst, lookup_table[temp].symbol, lookup_table[temp].bits);
	return lookup_table[temp].bits;
}

int huffmanStreamClose(short_buffer* src, bitstream_state_t* dst)
{
	int bit_count = 0;
	unsigned long long temp;
	while(src->bits>=8)
	{
		temp = short_buffer_get_bits(src, 8);
		bitstream_append_bits(dst, lookup_table[temp].symbol, lookup_table[temp].bits);
		bit_count += lookup_table[temp].bits;
	}

	if(src->bits!=0)
	{
		short_buffer_append_bits(src,0,8-src->bits);
		temp = short_buffer_get_bits(src, 8);
		bitstream_append_bits(dst, lookup_table[temp].symbol, lookup_table[temp].bits);
		bit_count += lookup_table[temp].bits;
	}
	return bit_count;
}

void huffmanByteDecode(bitstream_state_t* src,short_buffer* dst)
{
	int bit_count = 8;
	int i = 0;
	unsigned long long bit;
	static uint16_t temp = 5555;
	if(temp==5555)temp = 2*NUMBERS-2;

	while (i < bit_count)
	{
		bitstream_read_bits(src, &bit, 1);
		if (bit == 1)
		{
			temp = tree[temp].left;
		}
		else if (bit == 0)
		{
			temp = tree[temp].right;
		}

		if (tree[temp].original == 1)
		{
			short_buffer_append_bits(dst, tree[temp].symbol, BITS);
			temp = 2*NUMBERS-2;
		}
		i++;
	}
}


