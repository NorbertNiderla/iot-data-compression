#pragma once
void Entropy_AddToHist(unsigned char* symbols, int size);
void Entropy_AddToHistInt(int* symbols, int size);
void Entropy_ResetBuffer();
float Entropy_CalculateEntropy();
