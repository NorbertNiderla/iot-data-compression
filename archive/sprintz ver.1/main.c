#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "../../../compression_iot/archive/sprintz ver.1/huffman.h"
#include "../../../compression_iot/archive/sprintz ver.1/sprintz.h"
#include "xtimer.h"

int main(void)
{
	uint16_t data[] = { 1,2,3,2,6,9,3,10,11,4,14,19,5,18,30,6,22,31,7,26,32,8,30,40,
						9,34,48,10,38,56,11,42,64,12,46,72,13,50,80,14,54,88,15,58,96,16,62,104,
						2,4,6,4,12,18,6,12,22,8,28,38,10,36,60,12,44,62,14,52,64,16,60,80 };
		int number_of_sets = 3;
		uint16_t data_de[72];
		unsigned char dataEncoded[128] = {0};

	xtimer_ticks32_t timestamp_1 = xtimer_now();

	buildTree();

	xtimer_ticks32_t timestamp_2 = xtimer_now();

	buildLookUpTable();

	xtimer_ticks32_t timestamp_3 = xtimer_now();

	sprintz(data, number_of_sets, dataEncoded);

	xtimer_ticks32_t timestamp_4 = xtimer_now();

	sprintzDecode(dataEncoded, data_de);

	xtimer_ticks32_t timestamp_5 = xtimer_now();

			volatile int check = 1;
		for(int k=0;k<number_of_sets;k++)
		{
			for (int i = 0; i < 24; i++)
			{
				if(data_de[i+k*24]!=data[i+k*24])
				{
					check = 0;
					printf("%d %d %d\n",k,i,data_de[i+k*24]-data[i+k*24]);
				}
			}
		}

		if(check==1) printf("Sprintz works properly!\n");
		else printf("Sprintz is not working!\n");
		freeHuffmanObjects();

		xtimer_ticks32_t tree_time =xtimer_diff(timestamp_2,timestamp_1);
		xtimer_ticks32_t table_time =xtimer_diff(timestamp_3,timestamp_2);
		xtimer_ticks32_t encode_time =xtimer_diff(timestamp_4,timestamp_3);
		xtimer_ticks32_t decode_time =xtimer_diff(timestamp_5,timestamp_4);
		printf("Build Huffman tree time: %u\n",tree_time.ticks32);
		printf("Build Huffman lookup table time: %u\n",table_time.ticks32);
		printf("Encode time: %u\n",encode_time.ticks32);
		printf("Decode time: %u\n",decode_time.ticks32);
		return 0;
}
