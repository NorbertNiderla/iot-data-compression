#include "../../../compression_iot/archive/sprintz_huffman ver.2/bitstream.h"
#ifndef HUFFMAN_CODING_HUFFMAN_H
#define HUFFMAN_CODING_HUFFMAN_H

void huffmanDecode(unsigned char* src, unsigned char* dst, int byte_count);
int huffman(unsigned char* src, unsigned char* dst,int size);
void freeHuffmanObjects(void);
void buildTree(void);
void buildLookUpTable(void);

#endif //HUFFMAN_CODING_HUFFMAN_H
