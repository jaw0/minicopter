/*
  Copyright (c) 2015
  Author: Jeff Weisberg <jaw @ tcp4me.com>
  Created: 2015-Oct-09 23:32 (EDT)
  Function: lis3mdl magnetometer

*/

#include <conf.h>
#include <proc.h>
#include <i2c.h>
#include <gpio.h>
#include <stm32.h>
#include <userint.h>

#include "bot.h"
#include "lis3mdl.h"

static char  have_sen = 0;
static char  probesen[1];
static short magbuf[3];

static i2c_msg seninit[] = {

    I2C_MSG_C2( LIS3MDL_ADDRESS,      0,             LIS3MDL_REGISTER_CTRL1,    0x1E ), // ODR=1000Hz
    I2C_MSG_C2( LIS3MDL_ADDRESS,      0,             LIS3MDL_REGISTER_CTRL2,    0x00 ), // 4gauss
    I2C_MSG_C2( LIS3MDL_ADDRESS,      0,             LIS3MDL_REGISTER_CTRL3,    0x00 ), // continuous
    I2C_MSG_C2( LIS3MDL_ADDRESS,      0,             LIS3MDL_REGISTER_CTRL4,    0x0C ), // little-endian
    I2C_MSG_C2( LIS3MDL_ADDRESS,      0,             LIS3MDL_REGISTER_CTRL5,    0x40 ),
};

static i2c_msg senprobe[] = {
    I2C_MSG_C1( LIS3MDL_ADDRESS,      0,             LIS3MDL_REGISTER_WHO_AM_I ),
    I2C_MSG_DL( LIS3MDL_ADDRESS,      I2C_MSGF_READ, 1, probesen ),
};

static i2c_msg senreadall[] = {
    I2C_MSG_C1( LIS3MDL_ADDRESS,      0,             LIS3MDL_REGISTER_OUT_X_L | 0x80 ),
    I2C_MSG_DL( LIS3MDL_ADDRESS,      I2C_MSGF_READ, 6, (char*)magbuf ),
};


/****************************************************************/

void
init_compass(void){
    char i;
    for(i=0; i<10; i++){
        // init
        i2c_xfer(I2CUNIT, ELEMENTSIN(seninit), seninit, 1000000);
        // try to read
        i2c_xfer(I2CUNIT, ELEMENTSIN(senprobe), senprobe, 100000);
        if( probesen[0] ) break;
    }
    bootmsg(" lis3mdl");
}

static int i2c_speed[] = { 900000, 800000, 700000, 600000, 500000, 400000, 300000, 200000, 100000 };

DEFUN(compassprobe, "probe pressure sensor")
{
    int i;
    int senmax=0;

    //ui_pause();

    for(i=0; i<ELEMENTSIN(i2c_speed); i++){
        i2c_set_speed(I2CUNIT, i2c_speed[i] );
        i2c_xfer(I2CUNIT, ELEMENTSIN(seninit), seninit, 1000000);

        probesen[0] = 0;
        i2c_xfer(I2CUNIT, ELEMENTSIN(senprobe), senprobe, 100000);

        if( probesen[0] ){
            have_sen = 1;
            if( i2c_speed[i] > senmax ) senmax = i2c_speed[i];
        }
    }

    //ui_resume();
    printf("sen %d max %d\n", have_sen, senmax);
    return 0;
}

void
read_compass(void){
    i2c_xfer(I2CUNIT, ELEMENTSIN(senreadall), senreadall, 100000);
}

/****************************************************************/

static inline int
_compass_x(void){
    return magbuf[0];
}

static inline int
_compass_y(void){
    return magbuf[1];
}

static inline int
_compass_z(void){
    return  magbuf[2];
}


#if COMPASS_ROTATE == 0
int compass_x(void){ return   _compass_x(); }
int compass_y(void){ return   _compass_y(); }

#elif COMPASS_ROTATE == 180
int compass_x(void){ return - _compass_x(); }
int compass_y(void){ return - _compass_y(); }

#elif COMPASS_ROTATE == 90
int compass_x(void){ return - _compass_y(); }
int compass_y(void){ return   _compass_x(); }

#elif COMPASS_ROTATE == 270
int compass_x(void){ return   _compass_y(); }
int compass_y(void){ return - _compass_x(); }

#endif

int compass_z(void){ return _compass_z(); }


/****************************************************************/

#include "control.h"

DEFUN(testcomp, "test compass")
{
    while(1){
        read_compass();
        printf("comp %6d %6d %6d\n", compass_x(), compass_y(), compass_z() );
        printf("comp %6d %6d %6d\n", compass_x() - MAG_OFF_X, compass_y() - MAG_OFF_Y, compass_z() - MAG_OFF_Z);
        printf("\n");
        usleep(250000);
    }
    return 0;
}

DEFUN(calibcomp, "calibrate compass")
{
    int xa=0,xz=0, ya=0,yz=0;
    int i;

    for(i=0; i<10000; i++){
        read_compass();
        int x = compass_x();
        if( !xa || x < xa ) xa = x;
        if( !xz || x > xz ) xz = x;

        int y = compass_y();
        if( !ya || y < ya ) ya = y;
        if( !yz || y > yz ) yz = y;

        int z = compass_z();
        printf("%d %d %d\n", x, y, z);
        usleep(1000);
    }

    //printf("x: %d %d; y %d %d\n", xa, xz, ya, yz );

    return 0;
}
