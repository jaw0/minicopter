/*
  Copyright (c) 2015
  Author: Jeff Weisberg <jaw @ tcp4me.com>
  Created: 2015-Oct-06 00:36 (EDT)
  Function: blinky lights

*/

#include <conf.h>
#include <proc.h>
#include <gpio.h>
#include <pwm.h>
#include <ioctl.h>
#include <stm32.h>
#include <userint.h>

#include "board.h"
#include "dazzle.h"

#define msleep(x)	usleep(x * 1000)

static int current_blink_pattern = 0;



void
set_led_1(int b, int r){
    pwm_set( HWCF_TIMER_LED_1B, b & 255 );
    pwm_set( HWCF_TIMER_LED_1R,  r & 255 );
}

void
set_led_2(int b, int r){
    pwm_set( HWCF_TIMER_LED_2B, b & 255 );
    pwm_set( HWCF_TIMER_LED_2R,  r & 255 );
}

void
set_leds_rgb(int l, int r){
    // RGB, RGB (NB - green value is ignored)

    set_led_1( l & 0xFF, (l>>16) & 0xFF );
    set_led_2( r & 0xFF, (r>>16) & 0xFF );
}


DEFUN(testleds, "test leds")
{

    // red
    set_led_1( 0x0, 0xFF );
    set_led_2( 0x0, 0xFF );
    sleep(1);
    // purple
    set_led_1( 0xFF, 0xFF );
    set_led_2( 0xFF, 0xFF );
    sleep(1);
    // blue
    set_led_1( 0xFF, 0x0 );
    set_led_2( 0xFF, 0x0 );
    sleep(1);

    // 1 blue, 2 red
    set_led_1( 0x00, 0xFF );
    set_led_2( 0xFF, 0x00 );
    sleep(1);

    set_led_1( 0, 0 );
    set_led_2( 0, 0 );

    return 0;
}

/****************************************************************/

void
set_blinky(int p){
    current_blink_pattern = p;
}

DEFUN(set_blinky, "set blink pattern")
{
    if( argc > 1 )
        current_blink_pattern = atoi(argv[1]);
    return 0;
}

static const u_char throb_slow[40] = {
    1, 2, 4, 8, 16,
    16, 8, 4, 2, 1,
    1, 2, 4, 8, 16,
    16, 8, 4, 2, 1,
    0, 0, 0, 0,
    0, 0, 0, 0,
    0, 0, 0, 0,
    0, 0, 0, 0,
    0, 0, 0, 0,
};
static const u_char throb_fast[15] = {
    1, 2, 4, 8, 16,
    16, 8, 4, 2, 1,
    0, 0, 0, 0, 0
};


void
blinky(void){
    short i = 0;

    while(1){
        switch( current_blink_pattern ){
        case BLINK_OVERRIDE:
            msleep(250);
            break;

        case BLINK_OFF:
            set_leds_rgb( 0, 0 );
            msleep(250);
            break;

        case BLINK_BATT_DEAD:
            // |___|___
            set_leds_rgb( 0x7F0000, 0x7F0000 );
            msleep(25);
            set_leds_rgb( 0, 0 );
            msleep(475);
            break;

        case BLINK_ERROR:
            // X_X_X_
            set_leds_rgb( 0xFF0000, 0xFF0000 );
            msleep(125);
            set_leds_rgb( 0, 0 );
            msleep(125);
            break;

        case BLINK_WAIT_USER:
            for(i=0; i<sizeof(throb_slow); i++){
                if( current_blink_pattern != BLINK_WAIT_USER ) break;
                set_led_1( 7 * throb_slow[i], 0 );
                set_led_2( 7 * throb_slow[sizeof(throb_slow)-i-1], 0 );
                msleep(25);
            }
            break;

        case BLINK_FLY_AWAY:
            for(i=0; i<sizeof(throb_fast); i++){
                if( current_blink_pattern != BLINK_FLY_AWAY ) break;
                set_led_1( throb_fast[i], 0 );
                set_led_2( throb_fast[sizeof(throb_fast)-i-1], 0 );
                msleep(25);
            }
            break;

        default:
            msleep(250);
            break;
        }
    }
}

void
init_blinky(void){
    start_proc( 1024, blinky,  "blinky" );
}
