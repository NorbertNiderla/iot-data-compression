//by Norbert Niderla, 2021

#ifndef DATA_H
#define	DATA_H

#define DATA_SIZE (1000)
#define N_DATASETS  (9)

const int* get_ph_data_ptr(int offset);
const int* get_temp_data_ptr(int offset);
const int* get_cond_data_ptr(int offset);
const int* get_vol_data_ptr(int offset);
const int* get_gas_data_ptr(int offset);
const int* get_water_data_ptr(int offset);
const int* get_heart_data_ptr(int offset);
const int* get_hum_data_ptr(int offset);
const int* get_light_data_ptr(int offset);
const int* get_data_ptr(int offset, int id);

#endif
