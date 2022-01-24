#include "../../../compression_iot/archive/huffman ver.2/huffman.h"

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>

#include "../../../compression_iot/archive/huffman ver.1/bitstream.h"

#define NUMBERS 258
#define BITS 8

typedef struct node_t {
	struct node_t* left;
	struct node_t* right;
	struct node_t* father;
	struct node_t* next;
	int freq;
	int sign;
	bool original;
}node;

typedef struct nodes_list_t{
	node* start;
	int size;
	node* top;
}nodes_list;

typedef struct {
	uint16_t symbol;
	int bits;
}lookup_element;

static int frequencies[NUMBERS];
static nodes_list tree = {.start = NULL, .size = 0, .top = NULL};
static nodes_list free_nodes = {.start = NULL, .size = 0, .top = NULL};
//static nodes_list tree = { .array = {NULL}, .size = 0 };
//static nodes_list free_nodes = { .array = {NULL}, .size = 0 };
static lookup_element lookup_table[NUMBERS];

static void appendNode(nodes_list* list, node* new_node)
{
	if(list->size==0){
		list->start = new_node;
		list->size++;
	}
	else{
		new_node->next = list->start;
		list->start = new_node;
		list->size++;
	}
}

static node* newNode(nodes_list* list, int frequency, int sign, node* left, node* right, node* father, bool original)
{
	node* new = malloc(sizeof(node));
	new->father = father;
	new->freq = frequency;
	new->left = left;
	new->original = original;
	new->right = right;
	new->sign = sign;

	if(list->size==0){
		list->start = new;
		list->size++;
	}
	else if((list->size!=0) & (new->freq<list->start->freq)){
		new->next = list->start;
		list->start = new;
		list->size++;	
	}
	else{
		int i = 2;
		node* prev = list->start;
		node* temp = list->start->next;

		while(i++ <= list->size)
		{
			if (frequency <= temp->freq)
			{
				new->next = prev->next;
				prev->next = new;
				list->size++;
				break;
			}
			prev = temp;
			temp = temp->next;
		}
		
		if(temp == NULL){
			temp->next = new;
			list->size++;
		}
	}

	return new;
}

static node* popNode(nodes_list* list)
{
	list->size--;
	node* temp = list->start;
	list->start = list->start->next;
	return temp;
}

void buildTree(void)
{
	for(int i =0;i<NUMBERS;i++)frequencies[i] = 1;
	for (int i = 0; i < NUMBERS; i++) newNode(&free_nodes, frequencies[i], i, NULL, NULL, NULL, true);

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
	node* last = popNode(&free_nodes);
	appendNode(&tree, last);
	tree.top = last;
}

void buildLookUpTable(void)
{
	for (int i = 0; i < NUMBERS; i++)
	{
		lookup_table[i].bits = 0;
		lookup_table[i].symbol = 0;
	}

	node* main = tree.start;
	while(main!=NULL)
	{
		if (main->original == true)
		{
			node* temp = main;
			int k = 0;
			while (temp->father != NULL) 
			{	
				if (temp == temp->father->left)
				{
					lookup_table[main->sign].symbol |= (1<<k++);
					lookup_table[main->sign].bits++;
				}
				else if (temp == temp->father->right)
				{
					lookup_table[main->sign].symbol |= (0 << k++);
					lookup_table[main->sign].bits++;
				}

				temp = temp->father;
			}
		}
		main = main->next;
	}
}

void huffman(unsigned char* data, unsigned char* output,int size)
{
	size = size/BITS;
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
	node* temp = tree.top;
	
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
			temp = tree.top;
		}

		i++;
	}

	bitstream_write_close(dst_p);
}

void freeHuffmanObjects(void)
{
	while(tree.size!=0) free(popNode(&tree));
	while(free_nodes.size!=0) free(popNode(&free_nodes));
}

