#pragma once

void setFreqs(unsigned char* input, int size, int max_value, int* freqs);
void setOcc(int* input, int size, int L, uint8_t* occ);
void setFrequencies(unsigned char* input, int size, int* freqs);
void resetDensCounter(void);
void setOccurrences(int* input, int size, int L, uint16_t* occ);
void setOccurrencesNormal(int* input, int size, int L, uint16_t* occ);
