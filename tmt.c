//by Norbert Niderla, 2021
//TMT by Liang, 2010

#include <stdlib.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>

#include "aac.h"
#include "tmt.h"

#define M_MAX   (256)
#define DATA_BITWIDTH   (16)

#define PRINT_STATS	(0)
#if PRINT_STATS
#include <stdio.h>
#pragma message "tmt: print statistics enabled"
#endif

#define ENABLE_DEBUG    (0)
#if ENABLE_DEBUG
#include <stdio.h>
#pragma message "tmt: debug enabled"
#endif

#define SYMBOL_COUNTER_RANGE    (32)
static uint8_t symbol_counter[SYMBOL_COUNTER_RANGE] = {0};

static float get_entropy(int* data, int size){
	float entropy = 0;
    float single_symbol_entropy = 1.0f/(float)size*log(1.0f/(float)size)/log(2.0f);
    for(int i = 0; i < SYMBOL_COUNTER_RANGE; i++) symbol_counter[i] = 0;

    for(int i = 0; i < size; i++){
        if((data[i] < SYMBOL_COUNTER_RANGE) & (data[i] > -SYMBOL_COUNTER_RANGE)){
            symbol_counter[(int)abs(data[i])]++;
        } else {
            entropy += single_symbol_entropy;
        }
    }

    for(int i = 0; i < SYMBOL_COUNTER_RANGE; i++)
    	if(symbol_counter[i] != 0)
    		entropy += (float)symbol_counter[i]/(float)size*log((float)symbol_counter[i]/(float)size)/log(2.0f);

    return -entropy + 1;
}

static unsigned get_l_symbols(unsigned M, unsigned R){
    return(floor(log((float)R)/log((float)M)));
}

static float get_gamma(int* data, int size, unsigned R, unsigned M, unsigned K, float H){
    int N = size;
    unsigned N_neg = 0;
    unsigned N_nneg = 0;
    unsigned N_ovh = 0;
    unsigned N_ori = 0;

    for(int i = 0; i < size; i++){
        if((data[i] < 0) & (data[i] >= -((int)R)))
            N_neg++;
        else if((data[i] > 0) & (data[i] <= ((int)R)))
            N_nneg++;
        else{
            N_ovh++;
            N_ori++;
        }
    }

    float l = (float)get_l_symbols(M, R);
    float s = H*l*N_nneg+H*(l+1)*N_neg+H*l*N_ovh+K*N_ori;
    float gamma = 1.0f - s/(float)K/(float)N;

    return gamma;
} 

static void get_encoding_parameters(int* data, int size, unsigned* R_value, unsigned* M_value, unsigned* system_value){
    float sd = 0;

	for (int k = 0; k < size; k++) sd += ((float)(pow((double)data[k], 2)));

    unsigned R, M;
    float k;
    if(sd == 0){
        R = 0;
        M = 1;
        k = 1;
#if PRINT_STATS
        printf("%3d %3d %2.1f %2.1f 0\n", R, M, 0.0, 0.0);
#endif
    } else {

        sd = (float)sqrt((sd / (float) size));
        k = ceil(log((double)(3*sd))/log((double)M_MAX));
        float L = (float)floor(pow((double)(3*sd), 1/k));

        float entropy = get_entropy(data, size);

        float gamma_max_l = get_gamma(data, size, (unsigned)round((pow(L, k)-1)), L, DATA_BITWIDTH, entropy);
        float gamma_max_l_1 = get_gamma(data, size, (unsigned)round((pow(L+1,k)-1)), L+1, DATA_BITWIDTH, entropy);

        if(gamma_max_l > gamma_max_l_1){
            M = (unsigned)L;
        }else {
            M = (unsigned)L+1;
        }

        R = (unsigned)pow((double)M, k) - 1;
        
#if PRINT_STATS
        printf("%3d %3d %2.1f %2.1f %2.1f\n", R, M, sd, k, entropy);
#endif
    }

    if(k == 0) k = 1; //nie jestem pewien czy jest to wystarczające dla wszystkich przypadków
    *R_value = R;
    *M_value = M;
    *system_value = (unsigned)k;

}

tmt_placeholded_symbols_t plh_symbols;

static void plh_symbols_init(void){
    for(int i = 0; i < PLACEHOLDED_SYMBOLS_ARRAY_LEN; i++)
        plh_symbols.symbols[i] = 0;
    
    plh_symbols.len = 0;
}

static void plh_symbols_add(int sym){
    plh_symbols.symbols[plh_symbols.len] = sym;
    plh_symbols.len++;
    if(plh_symbols.len >= PLACEHOLDED_SYMBOLS_ARRAY_LEN){
    	printf("tmt_encode: plh_symbols_add: plh symbols array overflow\n");
    }
}

static unsigned plh_symbols_get_len(void){
    return plh_symbols.len;
}

#define ARRAY_TO_ENCODE_LEN	(40*4)
static unsigned array_to_encode[ARRAY_TO_ENCODE_LEN];

#if ENCODER_DC_VALUE
tmt_data_t tmt_encode(int *data, 
                int size, 
                unsigned char* output,
                int output_buffer_size, 
                int dc_value){
    
    for (int i = (size - 1); i > 0; i--) data[i] -= data[i - 1];
	data[0] -= dc_value;

	plh_symbols_init();
    tmt_data_t encoding_parameters;
    encoding_parameters.placeholded_symbols_nr = 0;



    unsigned R, M, sys_base;
    get_encoding_parameters(data, size, &R, &M, &sys_base);


    for(int i = 0; i < ARRAY_TO_ENCODE_LEN; i++) array_to_encode[i] = 0;
    unsigned idx_to_encode = 0;
    div_t div_res;
    for(int i = 0; i < size; i++){
        if((data[i] > (int)R) | (data[i] < -(int)R)){
            array_to_encode[idx_to_encode++] = M + PLACEHOLDER_OFFSET; //encode(PLACEHOLDER)
            plh_symbols_add(data[i]);
        } else {
            if(data[i]<0){
                array_to_encode[idx_to_encode++] = M + SIGN_SYMBOL_OFFSET; //encode(SIGN_SYMBOL)
                data[i] = -data[i];
            }
            for(unsigned c = 0; c < sys_base; c++){
                div_res = div(data[i], M);
                data[i] = div_res.quot;
                array_to_encode[idx_to_encode++] = (unsigned)div_res.rem;//encode(div_res.rem)
            }
        }
        if(idx_to_encode >= ARRAY_TO_ENCODE_LEN){
        	printf("tmt: array_to_encode array overflow!\n");
        }
    }


    adaptive_arithmetic_set_number_of_symbols(M + 2);

#if ENABLE_DEBUG
    printf("\ntmt_encode: arithmetic encode input:\n");
    for(unsigned i = 0; i < idx_to_encode;i++){
    	printf("%d ", array_to_encode[i]);
    }printf("\n");
#endif

    unsigned bytes = adaptive_arithmetic_encode(array_to_encode, idx_to_encode, output, output_buffer_size);

    encoding_parameters.R = R;
    encoding_parameters.M = M;
    encoding_parameters.bits = bytes << 3;
    encoding_parameters.arithmetic_encoded_symbols = idx_to_encode;
    encoding_parameters.placeholded_symbols_nr = plh_symbols_get_len();

    return(encoding_parameters);
}
#else
#endif

void tmt_decode(unsigned char* input, int* output, int size, tmt_data_t parameters){

    int k = (int)round(log((double)(parameters.R + 1))/log((double)parameters.M));
    adaptive_arithmetic_set_number_of_symbols(parameters.M + 2);
    unsigned* temp = array_to_encode;
    for(int i = 0; i < ARRAY_TO_ENCODE_LEN; i++) array_to_encode[i] = 0;


    adaptive_arithmetic_decode(input, parameters.bits, temp, parameters.arithmetic_encoded_symbols);


#if ENABLE_DEBUG
    printf("\ntmt_decode: arithmetic decode output:\n");
    for(unsigned i = 0; i < parameters.arithmetic_encoded_symbols;i++){
    	printf("%d ", temp[i]);
    }printf("\n");
#endif

    unsigned idx = 0;
    unsigned idx_plh = 0;
    unsigned neg = 0;
    for(int i = 0; i< size; i++){
        neg = 0;
        for(int c = (k - 1); c >= 0; c--){
            if(temp[idx] == parameters.M + PLACEHOLDER_OFFSET){
                idx++;
                output[i] = plh_symbols.symbols[idx_plh++];
                break;
            }
            if(temp[idx] == parameters.M + SIGN_SYMBOL_OFFSET){
                idx++;
                neg = 1;
            }
            if(c == 0){
            	output[i] += temp[idx];
            } else {
            	output[i] += temp[idx]*(parameters.M^c);
            }
            idx++;
        }
        if(neg==1){
            output[i] = -output[i];
        }    
    }

    for (int i = 1; i < size; i++)
		output[i] += output[i - 1];
}
