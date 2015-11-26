/*
  Copyright (c) 2015
  Author: Jeff Weisberg <jaw @ tcp4me.com>
  Created: 2015-Oct-06 00:30 (EDT)
  Function: measure battery voltage

*/

#include <conf.h>
#include <proc.h>
#include <gpio.h>
#include <adc.h>
#include <error.h>
#include <stm32.h>
#include <userint.h>

#include "board.h"

static int batt_voltage=0;


int
battery_voltage(void){
    return batt_voltage >> 8;
}

void
read_battery(void){

    int v = adc_get( HWCF_ADC_BATV );
    v <<= 8;

    // keep a running average
    if( batt_voltage <= 0 )
        batt_voltage = v;
    else
        batt_voltage = (batt_voltage + v) >> 1;
}

void
init_power(void){
    short i;

    bootmsg(" power");
    adc_init( HWCF_ADC_BATV, 1 );

    for(i=0; i<100; i++){
        read_battery();
    }
}


DEFUN(testbatt, "test battery")
{
    short i;

    for(i=0; i<100; i++){
        usleep(1000);
        read_battery();
    }

    int l = battery_voltage();

    float v = (l + 0.5)
        * 2.0 * 3.3	// 1:2 voltage divider, 3.3V Vcca
        / 4096.0;

    float p = (v - 3.2) * 100.0;
    if( p < 0 )   p = 0;
    if( p > 100 ) p = 100;


    printf("battery %.2fV  %d%% (%d)\n", v, (int)p, l);

    return 0;
}
