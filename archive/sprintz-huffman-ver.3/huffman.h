#pragma once
#include "../../../compression_iot/archive/sprintz-huffman-ver.3/bitstream.h"
#include "../../../compression_iot/archive/sprintz-huffman-ver.3/circular_short_buffer.h"

void buildTree(void);
void buildLookUpTable(void);
int huffmanByte(short_buffer* src, bitstream_state_t* dst);
int huffmanStreamClose(short_buffer* src, bitstream_state_t* dst);
void huffmanByteDecode(bitstream_state_t* src,short_buffer* dst);
