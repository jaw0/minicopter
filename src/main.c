/*
  Copyright (c) 2015
  Author: Jeff Weisberg <jaw @ tcp4me.com>
  Created: 2015-Oct-06 00:43 (EDT)
  Function: main

*/


#include <conf.h>
#include <proc.h>
#include <gpio.h>
#include <ioctl.h>
#include <error.h>
#include <stm32.h>
#include <userint.h>

#include "board.h"
#include "util.h"
#include "dazzle.h"


extern void blinky(void);


DEFUN(save, "save all config data")
{
    save_config("config.rc");
    return 0;
}

unsigned int
random(void){
    return lrand48();
}

void
onpanic(const char *msg){
    int i;

    set_motors(0,0,0,0);

    set_blinky( BLINK_OVERRIDE );
    set_leds_rgb(  0xFF0000, 0xFF0000 );
    beep_set(200, 127);

    splhigh();
    currproc = 0;

    while(1){
        set_leds_rgb(  0xFF0000, 0 );
        beep_set(150, 127);
        for(i=0; i<5000000; i++){ asm("nop"); }

        set_leds_rgb(  0, 0xFF0000 );
        beep_set(250, 127);
        for(i=0; i<5000000; i++){ asm("nop"); }
    }
}



//################################################################

void
main(void){

    board_init();
    set_led_1( 0xFF, 0xFF );
    set_led_2( 0xFF, 0xFF );

    run_script("config.rc");
    run_script("startup.rc");
    printf("\n");

    init_blinky();

    if( battery_voltage() <= BATT_WEAK ){
        set_blinky( BLINK_BATT_DEAD );
        play(16, "b-3b-3b-4b-4b-4b-3>b-3>>z");
        while(1) sleep(1);
    }

    play(8, "a>>4g4c+4");
    set_blinky( BLINK_WAIT_USER );

    // RSN - start flight interface

    // continue with interactive shell
}

