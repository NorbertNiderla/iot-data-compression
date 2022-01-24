#include <stdbool.h>
#ifndef HUFFMAN_CODING_HUFFMAN_H
#define HUFFMAN_CODING_HUFFMAN_H

void huffmanDecode(unsigned char* input,size_t input_buffer_size, unsigned char* output, int size);
int huffmanEncode(unsigned char* input, int size, unsigned char* output, size_t output_buffer_size);
void freeHuffmanObjects(void);
void buildTree(void);
void buildLookUpTable(void);
int huffmanEncodeWithTable(unsigned char* input, int size, unsigned char* output, size_t output_buffer_size, bool new_table);

#endif //HUFFMAN_CODING_HUFFMAN_H
