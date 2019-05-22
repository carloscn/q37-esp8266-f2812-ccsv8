/*
 * esp8266.h
 *
 *  Created on:
 *      Author: delvis
 */

#ifndef INC_ESP8266_H_
#define INC_ESP8266_H_

#include "DSP281x_Device.h"     // DSP281x Headerfile Include File
#include "DSP281x_Examples.h"   // DSP281x Examples Include File
#include "string.h"
#include "stdio.h"
#include "stdlib.h"
#include "stdint.h"
#include "stdbool.h"

#include "sci.h"

extern void esp8266_write_to_host(float *data);
extern bool is_enable_send;

#endif /* INC_ESP8266_H_ */
