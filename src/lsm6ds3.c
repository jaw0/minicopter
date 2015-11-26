/*
  Copyright (c) 2015
  Author: Jeff Weisberg <jaw @ tcp4me.com>
  Created: 2015-Sep-05 19:18 (EDT)
  Function: lsm6ds3 imu

*/

#include <conf.h>
#include <proc.h>
#include <i2c.h>
#include <gpio.h>
#include <stm32.h>
#include <userint.h>

#include "bot.h"
#include "lsm6ds3.h"

static char have_imu=0;

static short imubuf[6];	  // {G,A}{X,Y,Z}{L,H}
static char  probeimu[1];

static short accel_off_x, accel_off_y, accel_off_z;
static short gyro_off_x,  gyro_off_y,  gyro_off_z;


static i2c_msg imuinit[] = {

    // I2C_MSG_C2( LSM6DS3_ADDRESS,      0,             LSM6DS3_REGISTER_CTRL1_XL,  0x80 ), // odr=1.6kHz, 2g, bw=400Hz
    I2C_MSG_C2( LSM6DS3_ADDRESS,      0,             LSM6DS3_REGISTER_CTRL1_XL,  0x85 ), // odr=1.6kHz, 16g, bw=200Hz

    I2C_MSG_C2( LSM6DS3_ADDRESS,      0,             LSM6DS3_REGISTER_CTRL2_G,   0x7C ), // odr=833Hz, 2000dps,
    I2C_MSG_C2( LSM6DS3_ADDRESS,      0,             LSM6DS3_REGISTER_CTRL3_C,   0x44 ), // block, little-endian
    I2C_MSG_C2( LSM6DS3_ADDRESS,      0,             LSM6DS3_REGISTER_CTRL4_C,   0x80 ), // use bw in ctrl1_xl
    I2C_MSG_C2( LSM6DS3_ADDRESS,      0,             LSM6DS3_REGISTER_CTRL5_C,   0 ),
    I2C_MSG_C2( LSM6DS3_ADDRESS,      0,             LSM6DS3_REGISTER_CTRL6_C,   0 ),
    I2C_MSG_C2( LSM6DS3_ADDRESS,      0,             LSM6DS3_REGISTER_CTRL7_G,   0x50 ), // hi-pass = 0.03Hz
    I2C_MSG_C2( LSM6DS3_ADDRESS,      0,             LSM6DS3_REGISTER_CTRL8_XL,  0 ),
    I2C_MSG_C2( LSM6DS3_ADDRESS,      0,             LSM6DS3_REGISTER_CTRL9_XL,  0x38 ),
    I2C_MSG_C2( LSM6DS3_ADDRESS,      0,             LSM6DS3_REGISTER_CTRL10_C,  0x38 ),
};

static i2c_msg imuprobe[] = {
    I2C_MSG_C1( LSM6DS3_ADDRESS,      0,             LSM6DS3_REGISTER_WHO_AM_I ),
    I2C_MSG_DL( LSM6DS3_ADDRESS,      I2C_MSGF_READ, 1, probeimu ),
};

static i2c_msg imureadall[] = {
    I2C_MSG_C1( LSM6DS3_ADDRESS,      0,             LSM6DS3_REGISTER_OUTX_L_G ),
    I2C_MSG_DL( LSM6DS3_ADDRESS,      I2C_MSGF_READ, 12, imubuf ),
};


/****************************************************************/

void
init_imu(void){
    char i;
    for(i=0; i<10; i++){
        // init
        i2c_xfer(I2CUNIT, ELEMENTSIN(imuinit), imuinit, 1000000);
        // try to read
        i2c_xfer(I2CUNIT, ELEMENTSIN(imuprobe), imuprobe, 100000);
        if( probeimu[0] ) break;
    }
    bootmsg(" lsm6ds3");
}

static int i2c_speed[] = { 900000, 800000, 700000, 600000, 500000, 400000, 300000, 200000, 100000 };

DEFUN(imuprobe, "probe imu")
{
    int i;
    int imumax=0;

    //ui_pause();

    for(i=0; i<ELEMENTSIN(i2c_speed); i++){
        i2c_set_speed(I2CUNIT, i2c_speed[i] );
        i2c_xfer(I2CUNIT, ELEMENTSIN(imuinit), imuinit, 1000000);

        probeimu[0] = 0;
        i2c_xfer(I2CUNIT, ELEMENTSIN(imuprobe), imuprobe, 100000);

        if( probeimu[0] ){
            have_imu = 1;
            if( i2c_speed[i] > imumax ) imumax = i2c_speed[i];
        }
    }

    //ui_resume();
    printf("imu %d max %d\n", have_imu, imumax);
    return 0;
}

void
read_imu_all(void){
    i2c_xfer(I2CUNIT, ELEMENTSIN(imureadall), imureadall, 100000);
}

void
read_imu_quick(void){
    i2c_xfer(I2CUNIT, ELEMENTSIN(imureadall), imureadall, 100000);
}

void
read_imu_most(void){
    i2c_xfer(I2CUNIT, ELEMENTSIN(imureadall), imureadall, 100000);
}


/****************************************************************/
static inline int
_accel_x(void){
    short ax = imubuf[3];
    ax >>= 1;
    return ax;
}

static inline int
_accel_y(void){
    short ay = imubuf[4];
    ay >>= 1;
    return ay;
}

static inline int
_accel_z(void){
    short az = imubuf[5];
    az >>= 1;
    return az;
}

static inline int
_gyro_x(void){
    short gz = imubuf[0];
    return gz;
}

static inline int
_gyro_y(void){
    short gz = imubuf[1];
    return gz;
}

static inline int
_gyro_z(void){
    short gz = imubuf[2];
    return gz;
}

/****************************************************************/

#if ACCEL_ROTATE == 0
int accel_x(void){ return   _accel_x() - accel_off_x; }
int accel_y(void){ return   _accel_y() - accel_off_y; }

#elif ACCEL_ROTATE == 180
int accel_x(void){ return - _accel_x() + accel_off_x; }
int accel_y(void){ return - _accel_y() + accel_off_y; }

#elif ACCEL_ROTATE == 90
int accel_x(void){ return - _accel_y() + accel_off_y; }
int accel_y(void){ return   _accel_x() - accel_off_x; }

#elif ACCEL_ROTATE == 270
int accel_x(void){ return   _accel_y() - accel_off_y; }
int accel_y(void){ return - _accel_x() + accel_off_x; }

#endif

int accel_z(void){ return _accel_z() - accel_off_z; }

/****************************************************************/
// An excellent angler, and now with God.
//   -- The Complete Angler., Izaak Walton.

#if GYRO_ROTATE == 0
int gyro_x(void){ return   _gyro_x() - gyro_off_x; }
int gyro_y(void){ return   _gyro_y() - gyro_off_y; }

#elif GYRO_ROTATE == 180
int gyro_x(void){ return - _gyro_x() + gyro_off_x; }
int gyro_y(void){ return - _gyro_y() + gyro_off_y; }

#elif GYRO_ROTATE == 90
int gyro_x(void){ return - _gyro_y() + gyro_off_y; }
int gyro_y(void){ return   _gyro_x() - gyro_off_x; }

#elif GYRO_ROTATE == 270
int gyro_x(void){ return   _gyro_y() - gyro_off_y; }
int gyro_y(void){ return - _gyro_x() + gyro_off_x; }

#endif

int gyro_z(void){ return _gyro_z() - gyro_off_z; }

/****************************************************************/

void
imu_calibrate(void){
    int gxtot=0, gytot=0, gztot=0;
    int axtot=0, aytot=0, aztot=0;
    int n;

    gyro_off_x  = 0;
    gyro_off_y  = 0;
    gyro_off_z  = 0;
    accel_off_x = 0;
    accel_off_y = 0;
    accel_off_z = 0;

    for(n=0; n<1000; n++){
        read_imu_all();
        gxtot += _gyro_x();
        gytot += _gyro_y();
        gztot += _gyro_z();
        axtot += _accel_x();
        aytot += _accel_y();
        aztot += _accel_z() - 1000;
        usleep(1000);
    }

    accel_off_x = axtot/1000;
    accel_off_y = aytot/1000;
    accel_off_z = aztot/1000;
}

/****************************************************************/

DEFUN(testimu, "test imu")
{
    while(1){
        read_imu_all();

        printf("gyro %6d %6d %6d\n", gyro_x(),    gyro_y(),    gyro_z());
        printf("accl %6d %6d %6d\n", accel_x(),   accel_y(),   accel_z());
        printf("\n");

        usleep(250000);
    }
    return 0;
}

