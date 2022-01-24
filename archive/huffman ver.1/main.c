#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "../../../compression_iot/archive/huffman ver.1/huffman.h"
#include "../../../compression_iot/archive/huffman ver.1/sprintz.h"

int main(void)
{
	uint32_t data[] = { 1,2,3,2,6,9,3,10,11,4,14,19,5,18,30,6,22,31,7,26,32,8,30,40,2,4,6,4,12,18,6,12,22,8,28,38,10,36,60,12,44,62,14,52,64,16,60,80 };
		int number_of_sets = 2;
		uint32_t data_de[48];
		unsigned char dataEncoded[128] = {0};

		buildTree();
		buildLookUpTable();
		
		sprintz(data, number_of_sets, dataEncoded);
		sprintzDecode(dataEncoded, data_de);

		volatile int check = 1;
		for(int k=0;k<number_of_sets;k++)
		{
			for (int i = 0; i < 24; i++)
			{
				if(data_de[i+k*24]!=data[i+k*24])
				{
					check = 0;
					printf("%d %d\n",k,i);
				}
			}
		}

		if(check==1) printf("Sprintz works properly!\n");
		else printf("Sprintz is not working!\n");
		freeHuffmanObjects();
		return 0;
}
