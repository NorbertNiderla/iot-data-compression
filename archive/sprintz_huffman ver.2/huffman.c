#include "../../../compression_iot/archive/sprintz_huffman ver.2/huffman.h"

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>

#include "../../../compression_iot/archive/sprintz_huffman ver.2/bitstream.h"

#define NUMBERS 256
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

static int frequencies[NUMBERS] = {67,20,18,7,6,2,10,10,13,7,5,3,11,1,6,1,13,4,10,4,5,5,2,0,6,3,7,4,3,1,2,1,17,6,4,3,8,1,2,2,6,4,3,2,5,0,4,3,13,9,1,1,4,5,4,1,4,8,1,2,3,2,6,0,18,5,8,4,6,3,5,1,4,2,7,3,2,1,2,1,5,0,2,0,5,1,3,2,1,3,2,2,2,0,1,1,5,3,1,0,2,1,4,1,8,1,7,0,2,2,2,0,2,5,1,1,2,3,3,1,3,2,1,1,4,1,0,1,15,5,3,4,11,4,7,2,6,1,6,0,4,0,0,1,5,5,1,3,3,0,1,3,2,2,3,2,2,1,2,1,6,6,1,1,4,2,0,1,1,2,0,3,4,1,2,1,8,1,3,1,2,2,1,0,4,2,3,1,2,2,1,0,9,2,5,2,3,3,5,2,2,1,2,1,5,1,2,2,4,6,2,0,4,1,0,0,3,3,1,4,0,4,1,2,1,4,4,1,3,0,1,0,2,1,1,0,3,1,5,1,3,0,1,0,0,0,1,1,1,0,1,2,1,1,1,0};
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
	for(int i =0;i<NUMBERS;i++)if(frequencies[i] == 0)frequencies[i]=1;
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
	//for (int i = 0 ;i<NUMBERS;i++)printf("%d\n",lookup_table[i].bits);
}

int huffman(unsigned char* src, unsigned char* dst,int size)
{
	bitstream_state_t src_state;
	bitstream_state_t dst_state;
	bitstream_state_t* src_p = &src_state;
	bitstream_state_t* dst_p = &dst_state;
	bitstream_init(src_p, src, size);
	bitstream_init(dst_p, dst, size);

	int bit_count = 0;
	unsigned long long temp;
	for (int i = 0; i < size; i++)
	{
		bitstream_read_bits(src_p, &temp, BITS);
		bitstream_append_bits(dst_p, lookup_table[temp].symbol, lookup_table[temp].bits);
		bit_count += lookup_table[temp].bits;
	}
	bitstream_write_close(dst_p);
	if(bit_count%8!=0) bit_count += (8-bit_count%8);
	int byte_count = bit_count/8;

	return byte_count;
}

void huffmanDecode(unsigned char* src, unsigned char* dst, int byte_count)
{
	bitstream_state_t src_state;
	bitstream_state_t dst_state;
	bitstream_state_t* src_p = &src_state;
	bitstream_state_t* dst_p = &dst_state;
	bitstream_init(src_p, src, byte_count);
	bitstream_init(dst_p, dst, byte_count*2);

	int bit_count = byte_count*8;
	int i = 0;
	node* temp = tree.array[tree.size - 1];
	
	while (i < bit_count)
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

