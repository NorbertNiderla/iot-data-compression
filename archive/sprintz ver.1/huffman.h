#ifndef HUFFMAN_CODING_HUFFMAN_H
#define HUFFMAN_CODING_HUFFMAN_H

void huffmanDecode(unsigned char* data, unsigned char* output);
void huffman(unsigned char* data, unsigned char* output, int size);
void freeHuffmanObjects(void);
void buildTree(void);
void buildLookUpTable(void);

#endif //HUFFMAN_CODING_HUFFMAN_H
