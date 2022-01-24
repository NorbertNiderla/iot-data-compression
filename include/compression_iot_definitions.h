/*
 * compression_iot_definitions.h
 *
 *  Created on: Jul 15, 2021
 *      Author: Norbert Niderla
 */

#ifndef COMPRESSION_IOT_INCLUDE_COMPRESSION_IOT_DEFINITIONS_H_
#define COMPRESSION_IOT_INCLUDE_COMPRESSION_IOT_DEFINITIONS_H_

#define ENCODER_DC_VALUE	(1)

#define WHOLE_DATA_TESTING	(0)
#define BYTES_TEST			(WHOLE_DATA_TESTING)
#define	TIME_TEST			(!WHOLE_DATA_TESTING)
#define READ_DATA_FROM_FILE	(WHOLE_DATA_TESTING)

#if READ_DATA_FROM_FILE
#define DATA_BATCH_SIZE	(512)
#endif

#endif /* COMPRESSION_IOT_INCLUDE_COMPRESSION_IOT_DEFINITIONS_H_ */