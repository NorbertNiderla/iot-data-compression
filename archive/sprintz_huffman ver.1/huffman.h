#include "../../../compression_iot/archive/sprintz_huffman ver.1/bitstream.h"
#include "../../../compression_iot/archive/sprintz_huffman ver.1/circular_short_buffer.h"
#ifndef HUFFMAN_CODING_HUFFMAN_H
#define HUFFMAN_CODING_HUFFMAN_H

void buildTree(void);
void buildLookUpTable(void);
int huffmanByte(short_buffer* src, bitstream_state_t* dst);
int huffmanStreamClose(short_buffer* src, bitstream_state_t* dst);
void huffmanByteDecode(bitstream_state_t* src,short_buffer* dst);
#endif //HUFFMAN_CODING_HUFFMAN_H
