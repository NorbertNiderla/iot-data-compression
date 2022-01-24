#include "../../../compression_iot/archive/sprintz ver.1/huffman.h"

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>

#include "../../../compression_iot/archive/sprintz ver.1/bitstream.h"

#define NUMBERS 258
#define BITS 8

typedef struct node_t {
	struct node_t* left;
	struct node_t* right;
	struct node_t* father;
	int freq;
	int sign;
	bool original;
}node;

typedef struct {
	node* array[2*NUMBERS-1];
	int size;
} nodes_list;

typedef struct {
	uint16_t symbol;
	int bits;
}lookup_element;

static int frequencies[NUMBERS];
static uint16_t numbers[NUMBERS];
static nodes_list tree = { .array = {NULL}, .size = 0 };
static nodes_list free_nodes = { .array = {NULL}, .size = 0 };
static lookup_element lookup_table[NUMBERS];

static void appendNode(nodes_list* list, node* node_a)
{
	list->size++;
	list->array[list->size - 1] = node_a;
}

static node* newNode(nodes_list* list, int frequency, int sign, node* left, node* right, node* father, bool original)
{
	int i = 0;

	list->size++;
	node* new = malloc(sizeof(node));
	list->array[list->size-1] = new;
	list->array[list->size-1]->freq = frequency;
	list->array[list->size-1]->sign = sign;
	list->array[list->size-1]->left = left;
	list->array[list->size-1]->right = right;
	list->array[list->size-1]->father = father;
	list->array[list->size-1]->original = original;

	while (i++ < list->size-1)
	{
		if (frequency <= list->array[i]->freq)
		{
			node* temp = list->array[list->size-1];
			memmove(&(list->array[i]) + 1, &(list->array[i]), (list->size-i-1) * sizeof(node*));
			list->array[i] = temp;
			break;
		}
	}

	return new;
}

static node* popNode(nodes_list* list)
{
	node* temp = list->array[0];
	memmove(&list->array[0], &list->array[1], (list->size - 1) * sizeof(node*));
	list->size--;
	return temp;
}

void buildTree(void)
{
	for(int i =0;i<NUMBERS;i++)frequencies[i] = 1;
	for(int i =0;i<NUMBERS;i++)numbers[i] = i;

	for (int i = 0; i < NUMBERS; i++) newNode(&free_nodes, frequencies[i], numbers[i], NULL, NULL, NULL, true);

	while (free_nodes.size > 1)
	{
		node* node_left = popNode(&free_nodes);
		node* node_right = popNode(&free_nodes);
		node* new = newNode(&free_nodes, node_left->freq + node_right->freq, 0, node_left, node_right, NULL, false);
		node_left->father = new;
		node_right->father = new;
		appendNode(&tree, node_left);
		appendNode(&tree, node_right);
	}
	appendNode(&tree, popNode(&free_nodes));
}

void buildLookUpTable(void)
{
	for (int i = 0; i < NUMBERS; i++)
	{
		lookup_table[i].bits = 0;
		lookup_table[i].symbol = 0;
	}

	for (int i = 0; i < tree.size; i++)
	{
		if (tree.array[i]->original == true)
		{
			node* temp = tree.array[i];
			int k = 0;
			while (temp->father != NULL) 
			{	
				if (temp == temp->father->left)
				{
					lookup_table[tree.array[i]->sign].symbol |= (1<<k++);
					lookup_table[tree.array[i]->sign].bits++;
				}
				else if (temp == temp->father->right)
				{
					lookup_table[tree.array[i]->sign].symbol |= (0 << k++);
					lookup_table[tree.array[i]->sign].bits++;
				}

				temp = temp->father;
			}
		}
	}
}

void huffman(unsigned char* data, unsigned char* output,int size)
{
	//size = size/BITS;
	bitstream_state_t src;
	bitstream_state_t* src_p = &src;
	bitstream_init(src_p, data, 48);

	bitstream_state_t dst;
	bitstream_state_t* dst_p = &dst;
	bitstream_init(dst_p, output, 50);

	bitstream_append_int16(dst_p, 0x0000);
	int bit_count = 0;

	unsigned long long temp;
	for (int i = 0; i < size; i++)
	{
		bitstream_read_bits(src_p, &temp, BITS);
		bitstream_append_bits(dst_p, lookup_table[temp].symbol, lookup_table[temp].bits);
		bit_count += lookup_table[temp].bits;
	}
	bitstream_write_close(dst_p);

	memcpy(output, &bit_count, 2 * sizeof(unsigned char));
	unsigned char tmp = *output;
	*output = *(output + 1);
	*(output+1) = tmp;
}

void huffmanDecode(unsigned char* data, unsigned char* output)
{
	bitstream_state_t src;
	bitstream_state_t* src_p = &src;
	bitstream_init(src_p, data, 128);

	bitstream_state_t dst;
	bitstream_state_t* dst_p = &dst;
	bitstream_init(dst_p, output, 128);
	
	unsigned long long bit_count = 0;
	bitstream_read_bits(src_p, &bit_count, 16);

	int i = 0;
	node* temp = tree.array[tree.size - 1];
	
	while (i < (int)bit_count)
	{
		unsigned long long bit;
		bitstream_read_bits(src_p, &bit, 1);
		if (bit == 1)
		{
			temp = temp->left;
		}
		else if (bit == 0)
		{
			temp = temp->right;
		}

		if (temp->original == true)
		{
			bitstream_append_bits(dst_p, temp->sign, BITS);
			temp = tree.array[tree.size - 1];
		}

		i++;
	}

	bitstream_write_close(dst_p);
}

void freeHuffmanObjects(void)
{
	for(int i =0;i<tree.size;i++)free(tree.array[i]);
}

