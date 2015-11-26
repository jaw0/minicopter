/*
  Copyright (c) 2015
  Author: Jeff Weisberg <jaw @ tcp4me.com>
  Created: 2015-Oct-06 00:36 (EDT)
  Function: beep beep

*/

#include <conf.h>
#include <proc.h>
#include <gpio.h>
#include <pwm.h>
#include <ioctl.h>
#include <stm32.h>
#include <userint.h>

#include "board.h"

int volume_setting = 4;
int volume         = 16;

void
set_volume(int v){
    if( v > 7 ) v = 7;
    volume_setting = v;
    volume = 1<<v;
}

DEFUN(volume, "set volume")
{
    if( argc == 2 ){
        set_volume( atoi(argv[1]) );
    }else{
        printf("volume: %d\n", volume_setting);
    }

    return 0;
}

void
beep_set(int freq, int vol){
    if( vol > 128 ) vol = 128;
    if( vol < 0  )  vol = 0;

    freq_set(HWCF_TIMER_AUDIO, freq);
    pwm_set(HWCF_TIMER_AUDIO,  vol);
}

void
beep(int freq, int vol, int dur){
    if( vol > 128 ) vol = 128;
    if( vol < 1  )  vol = 1;

    freq_set(HWCF_TIMER_AUDIO, freq);
    pwm_set(HWCF_TIMER_AUDIO,  vol);
    usleep(dur);
    pwm_set(HWCF_TIMER_AUDIO,  0);
}

DEFUN(beep, "beep")
{

    if( argc > 2 )
        beep( atoi(argv[1]), atoi(argv[2]), 250000 );

    return 0;
}


void
beepdown(int n){

    set_led_1(63,63);
    set_led_2(63,63);

    for(; n>0; n--){

        switch(n){
        case 4:
            play(volume, "d4d4d4d4 z");
            break;
        case 3:
            play(volume, "d4d4d4 z4z");
            break;
        case 2:
            set_led_1(0,127);
            set_led_2(0,127);
            play(volume, "d4d4 z3z");
            break;
        case 1:
            set_led_1(127,0);
            set_led_2(127,0);
            play(volume, "c1>");
            break;
        case 0:
            break;
        default:
            play(volume, "e3z3z");
            break;
        }
    }
    set_led_1(0,0);
    set_led_2(0,0);
}

