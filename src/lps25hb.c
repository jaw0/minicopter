/*
  Copyright (c) 2015
  Author: Jeff Weisberg <jaw @ tcp4me.com>
  Created: 2015-Oct-09 23:32 (EDT)
  Function: lps25hb sensor

*/

#include <conf.h>
#include <proc.h>
#include <i2c.h>
#include <gpio.h>
#include <stm32.h>
#include <userint.h>

#include "bot.h"
#include "lps25hb.h"

static char have_sen=0;
static char probesen[1];
static int  pressure;
static short temper;

static i2c_msg seninit[] = {

    I2C_MSG_C2( LPS25HB_ADDRESS,      0,             LPS25HB_REGISTER_CTRL1,    0xC4 ), // ODR=25Hz
    I2C_MSG_C2( LPS25HB_ADDRESS,      0,             LPS25HB_REGISTER_CTRL2,    0x00 ),
    I2C_MSG_C2( LPS25HB_ADDRESS,      0,             LPS25HB_REGISTER_CTRL3,    0x00 ),
    I2C_MSG_C2( LPS25HB_ADDRESS,      0,             LPS25HB_REGISTER_CTRL4,    0x00 ),
    I2C_MSG_C2( LPS25HB_ADDRESS,      0,             LPS25HB_REGISTER_RES_CONF, 0x0F ),  // 512 samples
    I2C_MSG_C2( LPS25HB_ADDRESS,      0,             LPS25HB_REGISTER_REF_P_XL, 0x00 ),
    I2C_MSG_C2( LPS25HB_ADDRESS,      0,             LPS25HB_REGISTER_REF_P_L,  0x00 ),
    I2C_MSG_C2( LPS25HB_ADDRESS,      0,             LPS25HB_REGISTER_REF_P_H,  0x00 ),
};

static i2c_msg senprobe[] = {
    I2C_MSG_C1( LPS25HB_ADDRESS,      0,             LPS25HB_REGISTER_WHO_AM_I ),
    I2C_MSG_DL( LPS25HB_ADDRESS,      I2C_MSGF_READ, 1, probesen ),
};

static i2c_msg senreadall[] = {
    I2C_MSG_C1( LPS25HB_ADDRESS,      0,             LPS25HB_REGISTER_PRESS_OUT_XL | 0x80 ),
    I2C_MSG_DL( LPS25HB_ADDRESS,      I2C_MSGF_READ, 3, (char*)&pressure ),
    //I2C_MSG_C1( LPS25HB_ADDRESS,      0,             LPS25HB_REGISTER_TEMP_OUT_L | 0x80 ),
    //I2C_MSG_DL( LPS25HB_ADDRESS,      I2C_MSGF_READ, 2, (char*)&temper ),
};


/****************************************************************/

void
init_pressure(void){
    char i;
    for(i=0; i<10; i++){
        // init
        i2c_xfer(I2CUNIT, ELEMENTSIN(seninit), seninit, 1000000);
        // try to read
        i2c_xfer(I2CUNIT, ELEMENTSIN(senprobe), senprobe, 100000);
        if( probesen[0] == LPS25HB_WHO_I_AM ) break;
    }
    bootmsg(" lps25hb");
}

static int i2c_speed[] = { 900000, 800000, 700000, 600000, 500000, 400000, 300000, 200000, 100000 };

DEFUN(psenprobe, "probe pressure sensor")
{
    int i;
    int senmax=0;

    //ui_pause();

    for(i=0; i<ELEMENTSIN(i2c_speed); i++){
        i2c_set_speed(I2CUNIT, i2c_speed[i] );
        i2c_xfer(I2CUNIT, ELEMENTSIN(seninit), seninit, 1000000);

        probesen[0] = 0;
        i2c_xfer(I2CUNIT, ELEMENTSIN(senprobe), senprobe, 100000);

        if( probesen[0] == LPS25HB_WHO_I_AM ){
            have_sen = 1;
            if( i2c_speed[i] > senmax ) senmax = i2c_speed[i];
        }
    }

    //ui_resume();
    printf("sen %d max %d\n", have_sen, senmax);
    return 0;
}

void
read_pressure(void){
    pressure = 0;
    i2c_xfer(I2CUNIT, ELEMENTSIN(senreadall), senreadall, 100000);

    // sign extend the 24bit value
    u_char *p = (u_char*)&pressure;
    if( p[2] & 0x80 )
        p[3] = 0xFF;
    else
        p[3] = 0;

}

int
current_pressure(void){
    return pressure;
}


/****************************************************************/


DEFUN(testpress, "test pressure sensor")
{
    while(1){
        read_pressure();
        printf("press %6d temp %4d\n", current_pressure(), temper);
        usleep(250000);
    }
    return 0;
}

