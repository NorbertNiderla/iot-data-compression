#include "../../../../compression_iot/archive/source/board_2_dim_new/rice.h"

#include <stdlib.h>
#include <math.h>
#include <stdio.h>

#include "../../../../compression_iot/archive/source/board_2_dim_new/bitstream.h"
#include "../../../../compression_iot/archive/source/board_2_dim_new/entropy.h"

#define DIM 2

static int hist[256] = {0};

static void zigzag(int* data, int size){
	for(int idx=0;idx<size;idx++){
		if(data[idx]<0)data[idx] = -2*data[idx]-1;
		else data[idx] = 2*data[idx];
	}
}

static void zigzagDecode(int* data, int size){
	for(int idx=0;idx<size;idx++){
		if(data[idx]%2==1)data[idx]=(data[idx]+1)/(-2);
		else data[idx] = data[idx]/2;
	}
}
/*
static int std_min_bit_length(int x){
  if(x == 0) return 0;

  return(int)(floor(log2(abs(x))) + 1);
}

static int findAbsoluteSmallestValue(int* arr, int size){
	int i = 0;
	int val = abs(arr[0]);
	while(i<size){
		if(abs(arr[i])<val) val = abs(arr[i]);
		i++;
	}
	return val;
}

static int findAbsoluteBiggestValue(int* arr, int size){
	int i = 1;
	int val = arr[0];
	while(i<size){
		if(abs(arr[i])>val) val = abs(arr[i]);
		i++;
	}
	return val;
}
*/
static int rice_parameter_estimate(int* n, int size, int offset){
	int sum = 0;
	for(int i = offset; i<DIM*size; i+=DIM) sum += n[i];
	if(sum==0) return 0;
	float mu = (float)sum/(float)size;
	float r = ceil(log2(log(2)*mu));
	if(r<0) return 0;
 	return r;
}

/*
static int rice_parameter_optimise(int* n, int size, int offset){
	int R_min = -1;
	int R_max = -1;

	if(R_min < 0){
		R_min = std_min_bit_length(findAbsoluteSmallestValue(n, size)) - 1;
		if(R_min<0) R_min = 0;
	}

	if(R_max < 0) R_max = std_min_bit_length(findAbsoluteBiggestValue(n, size));

	int s, r_val;
	r_val = 999;
	int b_val = 999;

	//printf("%d %d\n", R_min,R_max);//DEBUG
	for(int r = R_min; r <= R_max; r++){
		s = 0;
		for(int i = offset; i<DIM*size; i+=DIM){
			s += (abs(n[i]))>>r;
		}

		if((s + size*(r + 2))<b_val){
			b_val = (s + size*(r + 2));
			r_val = r;
		}
	}
	return r_val;
}
*/

int riceEncode(int* n, int size, unsigned char* output, size_t output_buffer_size){

	bitstream_state_t state;
	bitstream_state_t* state_p = &state;
	bitstream_init(state_p, output, output_buffer_size);

	int n_bits = 0;

	zigzag(n,size*DIM);

	int r[DIM] = {0};
	for(int k = 0; k<DIM; k++){
		//r[k] = rice_parameter_optimise(n, size, k);
		r[k] = rice_parameter_estimate(n, size, k);
		if(r[k] == 999) return 0;
		bitstream_append_bits(state_p,r[k],5);
		n_bits += 5;
	}
	//printf("%d %d\n", r[0],r[1]);//DEBUG

	for(int i = 0; i<size*DIM; i++){
		int t = i%DIM;
		int s = n[i]>>r[t];
		bitstream_append_bits(state_p, (unsigned long long)~(0xFFFFF<<s),s);
		bitstream_append_bits(state_p, 0, 1);
		bitstream_append_bits(state_p, n[i]&(~(0xFFFFF<<r[t])),r[t]);
		n_bits += (s+1+r[t]);
	}
	bitstream_write_close(state_p);

	for(int i =0;i<(int)ceil((double)n_bits/8);i++) hist[output[i]]++;

	return n_bits;
}

void riceDecode(unsigned char* input, int size, int* n){
	bitstream_state_t state;
	bitstream_state_t* state_p = &state;
	bitstream_init(state_p, input, 64);
	unsigned long long m,bit;
	unsigned long long* r = (unsigned long long*)malloc(DIM*sizeof(unsigned long long));
	for(int k=0;k<DIM;k++) bitstream_read_bits(state_p,&(r[k]),8);
	for(int i = 0; i<size*DIM; i++){
		int t = i%DIM;
		int s = 0;
		bitstream_read_bits(state_p,&bit,1);
		while(bit!=0){
			s++;
			bitstream_read_bits(state_p,&bit,1);
		}
		bitstream_read_bits(state_p,&m,r[t]);
		n[i] = (s<<r[t]) + m;
	}
	zigzagDecode(n,size*DIM);
}

void printRiceHist(void){
	for(int i =0;i<256;i++){
		printf("%d\n", hist[i]);
		hist[i] = 0;
	}
}
