#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdint.h>
#include <stdbool.h>

#define SYMBOLS 256

static float dens = 0;
static int n_calc = 0;

void resetDensCounter(void){
	dens = 0;
	n_calc = 0;
}
void setFrequencies(unsigned char* input, int size, int* freqs){
	int cnt = 0;
	for(int i = 0; i<size; i++) if(input[i]==0) cnt++;
	dens = dens + ((float)cnt/(float)size-dens)/(++n_calc);
	if(dens==1) freqs[0] = 10000;
	else if(dens==0) freqs[0] = 1;
	else freqs[0] = (int)(-dens*750/(dens-1));
	//printf("%d\n",(int)(dens*100));
}
static int calcMean(unsigned char* input, int size){
	int sum = 0;
	for(int i = 0; i<size; i++) sum += (int)input[i];
	return (int)sum/size;
}

static int calcMeanInt(int* input, int size){
	int sum = 0;
	for(int i = 0; i<size; i++) sum += input[i];
	return (int)sum/size;
}

static double getA(int mean){
	double a;
	if( mean<30) a = (double)-3;
	else if((mean>=30) & (mean<45)) a = (double)0.0772*mean + (double)-5.2899;
	else if((mean>=45) & (mean<125)) a = (double)0.0199*mean + (double)-2.6923;
	else if(mean>=125) a = (double)-0.2;
	return a;
}

static void getDistribution(int* output, double a, int max){
	for(int k = 0;k<SYMBOLS;k++){
		output[k] = (a*(double)k)+max;
		if(output[k]<=0) output[k] = 1;
	}
	output[0] = 5*output[0];
}

static void getDistributionUint8(uint8_t* output, double a, int max){
	for(int k = 0;k<SYMBOLS;k++){
		output[k] = (a*(double)k)+max;
		if(output[k]<=0) output[k] = 1;
	}
}

void setFreqs(unsigned char* input, int size, int max_value, int* freqs){
	int mean = calcMean(input, size);
	//printf("%d\n", mean);//DEBUG
	double a = getA(mean);
	getDistribution(freqs,a,max_value);
}

void setOcc(int* input, int size, int L, uint8_t* occ){
	int mean = calcMeanInt(input, size);
	double a = getA(mean);
	getDistributionUint8(occ,a,256);
	int sum = 0;
	for(int i = 0; i<SYMBOLS; i++) sum += occ[i];
	int coeff = sum/L;
	for(int i = 0; i<SYMBOLS; i++){
		occ[i] = (int)round(occ[i]/coeff);
		if(occ[i]<1) occ[i] = 1;
	}

	occ[0] = occ[0]+L-300;//DEBUG
	sum = 0;
	for(int i = 0; i<SYMBOLS; i++) sum += occ[i];

	int m_b  = L-sum;
	if(m_b>0){
		int idx = 0;
		while(m_b!=0){
			occ[idx++]++;
			m_b--;
			if(idx==256) idx=0;
		}
	}
	else if(m_b<0){
		int idx = 255;
		while(m_b!=0){
			if(occ[idx]>1){
				occ[idx]--;
				m_b++;
			}
			idx--;
			if(idx==-1) idx=255;
		}
	}
}

void setOccurrences(int* input, int size, int L, uint16_t* occ){
	int cnt = 0;
	for(int i = 0; i<size; i++) if(input[i]==0) cnt++;
	dens = dens + ((float)cnt/(float)size-dens)/(++n_calc);
	if(dens==1) occ[0] = 10000;
	else if(dens==0) occ[0] = 1;
	else occ[0] = (int)(-dens*(250+5*3)/(dens-1));

	//printf("%d\n",(int)(dens*100));

	int sum = 0;
	for(int i = 0; i<SYMBOLS; i++) sum += occ[i];
	float coeff = (float)sum/(float)L;
	for(int i = 0; i<SYMBOLS; i++){
		occ[i] = (int)round(((float)occ[i])/coeff);
		if(occ[i]<1) occ[i] = 1;
	}

	sum = 0;
	for(int i = 0; i<SYMBOLS; i++) sum += occ[i];

	int m_b  = L-sum;
	if(m_b>0){
		int idx = 0;
		while(m_b!=0){
			occ[idx++]++;
			m_b--;
			if(idx==256) idx=0;
		}
	}
	else if(m_b<0){
		bool lin_flag = true;
		for(int l = 1; l<256; l++) if(occ[l]!=1) lin_flag = false;
		if(lin_flag == true) occ[0] = occ[0]+m_b;
		else{
			int idx = 255;
			while(m_b!=0){
				if(occ[idx]>1){
					occ[idx]--;
					m_b++;
				}
				idx--;
				if(idx==-1) idx=255;
			}
		}
	}
}




