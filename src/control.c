/*
  Copyright (c) 2015
  Author: Jeff Weisberg <jaw @ tcp4me.com>
  Created: 2015-Sep-05 19:18 (EDT)
  Function: motor control

*/

#include <conf.h>
#include <nstdio.h>
#include <proc.h>
#include <userint.h>
#include <stm32.h>

#include "bot.h"
#include "util.h"
#include "dazzle.h"
#include "control.h"

#define GY2DEG		0.00007	// degrees/millisec
#define RADIAN2DEG  	57.2958
#define RADIAN2GYRO	(RADIAN2DEG / GY2DEG)
#define CIRCLE2GYRO	5142857
#define SEALEVEL	4150272		// 1013.25 * 4096
#define FA		15
#define FC		15
#define FP		15
#define AALPHA		24
#define CALPHA		24
#define PALPHA		100.0
#define VALPHA		200.0
#define ZALPHA		2047
#define AALPH2		((AALPHA+1) / 2)
#define CALPH2		((CALPHA+1) / 2)
#define ZALPH2		((ZALPHA+1) / 2)

#define MOTOR_AMAX	800

#define GYRO2DEG(x)	((x) * 7 / 100000)
#define DEG2GYRO(x)	((x) * 100000 / 7)


int   accx_lpf     = 0;
int   accy_lpf     = 0;
int   accz_lpf     = 1000;
int   magx_lpf     = 0;
int   magy_lpf     = 0;
int   magz_lpf     = 0;
int   press_lpf    = SEALEVEL;
int   angle_gp     = 0;
int   angle_gr     = 0;
int   angle_gy     = 0;
int   angle_ax     = 0;
int   angle_ay     = 0;
int   angle_mz     = 0;
int   angle_ez     = 0;
int   offset_yaw   = 0;
int   offset_az    = 1000;
int   offset_press = SEALEVEL;
int   baro_alt     = 0;
// positive pitch  = > nose up
// positive roll   = > right up
int   angle_pitch  = 0;
int   angle_roll   = 0;
int   angle_yaw    = 0;
float altitude     = 0;
float valtitude    = 0;
int   err_ip       = 0;
int   err_ir       = 0;
int   err_iy       = 0;

// desired set points
int target_pitch = 0;
int target_roll  = 0;
int target_yaw   = 0;
int target_gx    = 0;
int target_gy    = 0;
int target_gz    = 0;
int target_alt   = 0;
int target_valt  = 0;
// simple feedforward
int motor_ff     = 0;
int uhoh_count   = 0;
int moving	 = 0;

int err_p, err_r, err_y, err_a;
int adj_p, adj_r, adj_y, adj_a;

extern float  atan2f(float, float);
extern double atan2(double, double);


void
filter_reset(void){

    accx_lpf    = 0;
    accy_lpf    = 0;
    accz_lpf    = 1000;
    magx_lpf    = 0;
    magy_lpf    = 0;
    magz_lpf    = 0;
    press_lpf   = SEALEVEL;
    angle_gp    = 0;
    angle_gr    = 0;
    angle_gy    = 0;
    angle_pitch = 0;
    angle_roll  = 0;
    angle_yaw   = 0;
    offset_yaw  = 0;
    err_ip      = 0;
    err_ir      = 0;
    err_iy      = 0;
    altitude    = 0;
    valtitude   = 0;
    adj_p       = 0;
    adj_r       = 0;
    adj_y       = 0;
    adj_a       = 0;
    err_p       = 0;
    err_r       = 0;
    err_y       = 0;
    err_a       = 0;
    uhoh_count  = 0;
    moving      = 0;
}

void
acc_filter(void){
    // lpf
    accx_lpf  = (FA * accx_lpf + 256 * accel_x()) / (FA + 1);
    accy_lpf  = (FA * accy_lpf + 256 * accel_y()) / (FA + 1);
    accz_lpf  = (FA * accz_lpf + 256 * accel_z()) / (FA + 1);
}

void
mag_filter(void){
    // lpf
    magx_lpf  = (FC * magx_lpf + 256 * (compass_x() - MAG_OFF_X)) / (FC + 1);
    magy_lpf  = (FC * magy_lpf + 256 * (compass_y() - MAG_OFF_Y)) / (FC + 1);
    magz_lpf  = (FC * magz_lpf + 256 * (compass_z() - MAG_OFF_Z)) / (FC + 1);
}

void
press_filter(void){

    press_lpf = (FP * press_lpf + current_pressure()) / (FP + 1);
}


void
filter_sensors(void){

    acc_filter();
    mag_filter();
    press_filter();

    // angles reported by accelerometer
    angle_ax  = - atan2f( (float)accx_lpf, (float)accz_lpf) * RADIAN2GYRO;
    angle_ay  =   atan2f( (float)accy_lpf, (float)accz_lpf) * RADIAN2GYRO;
    angle_mz  =   atan2f( (float)magx_lpf, (float)magy_lpf) * RADIAN2GYRO;

    // bias yaw angle around target, to avoid handling the discontinuity going through the filter
    angle_ez =    angle_mz - target_yaw - offset_yaw;
    angle_ez =    (angle_ez + 2 * CIRCLE2GYRO) % CIRCLE2GYRO;
    if( angle_ez > CIRCLE2GYRO/2) angle_ez = CIRCLE2GYRO - angle_ez;

    // informational only
    angle_gr  += gyro_x();
    angle_gp  += gyro_y();
    angle_gy  += gyro_z();

    // my, what nice math you have.
    // you have such a lovely file.
    // -- complimentary filter
    angle_pitch += gyro_y();
    angle_roll  += gyro_x();
    angle_yaw   += gyro_z();

    // complementary filter
    angle_pitch = (AALPHA * angle_pitch + angle_ax + AALPH2) / (AALPHA+1);
    angle_roll  = (AALPHA * angle_roll  + angle_ay + AALPH2) / (AALPHA+1);
    // RSN - improve compass calibration and uncomment
    // angle_yaw   = (CALPHA * angle_yaw   + angle_ez + CALPH2) / (CALPHA+1);
#if 0
    // altitude (units of um)
    //     meters = (1-(Pressure/1013.25)^0.190284) * 44307
    //            ~ - reading * 0.190284 * 44307 / 4096 / 1013.25
    baro_alt    = (offset_press - press_lpf) * 2031;

    int prevalt  = altitude;
    int prevvalt = valtitude;
    valtitude  += (accz_lpf - offset_az) / 256;
    altitude   += valtitude * 0.0098;	// um; 9.8/1000

    altitude    = (PALPHA * altitude   + baro_alt) / (PALPHA+1);
    valtitude   = (VALPHA * valtitude  + 102.0 * (altitude - prevalt) ) / (VALPHA+1);
#endif
}

/****************************************************************/

void
read_all_sensors(void){
    read_imu_all();
    read_compass();
    read_pressure();
    read_battery();
}


void
reset_control(){
    int i;

    filter_reset();
    imu_calibrate();

    for(i=0; i<500; i++){
        read_all_sensors();
        acc_filter();
        mag_filter();
        press_filter();
        pause();
    }

    offset_yaw   = atan2f( (float)magx_lpf, (float)magy_lpf) * RADIAN2GYRO;
    offset_press = press_lpf;
    offset_az    = accz_lpf;
}

int
safety_check(void){

    int p = ABS(angle_pitch);
    int r = ABS(angle_roll);
    int y = ABS(gyro_z());

    if( p > DEG2GYRO(90) )  goto uhoh;
    if( r > DEG2GYRO(90) )  goto uhoh;
    if( y > DEG2GYRO(1)/5 ) goto uhoh;

    if( battery_voltage() < BATT_DEAD ) goto uhoh;

    uhoh_count = 0;
    return 0;

uhoh:
    if( ++uhoh_count > 100 ){
        return 1;
    }

    return 0;
}

static inline int
ibound(int v, int lim){

    //if( v > lim ) return lim;
    //if( v <-lim ) return -lim;
    return v;
}

void
control_step(void){

    read_all_sensors();
    filter_sensors();

    err_p  = target_pitch - angle_pitch;
    err_r  = target_roll  - angle_roll;
    err_y  = 0            - angle_yaw;	// is measured from target
    err_a  = target_alt   - altitude;

    int err_dp = target_gy    - gyro_y();
    int err_dr = target_gx    - gyro_x();
    int err_dy = target_gz    - gyro_z();
    int err_da = target_valt  - valtitude;

    // I (avoid excessive windup)
    err_ip     = ibound(err_ip + err_p, 1023.0/KIP);
    err_ir     = ibound(err_ir + err_r, 1023.0/KIR);
    err_iy     = ibound(err_iy + err_y, 1023.0/KIY);

    // PID
    adj_p   = KPP * err_p + KIP * err_ip + KDP * err_dp;
    adj_r   = KPR * err_r + KIR * err_ir + KDR * err_dr;
    adj_y   = KPY * err_y + KIY * err_iy + KDY * err_dy;
    adj_a   = KPA * err_a  /* PD only */ + KDA * err_da;

    if( motor_ff + adj_a > MOTOR_AMAX )
        adj_a = MOTOR_AMAX - motor_ff;

    // feed forward
    int m1 = motor_ff;
    int m2 = motor_ff;
    int m3 = motor_ff;
    int m4 = motor_ff;

    // adjust motors
    m1 += adj_a;
    m2 += adj_a;
    m3 += adj_a;
    m4 += adj_a;

    m1 -= adj_p;
    m2 -= adj_p;
    m3 += adj_p;
    m4 += adj_p;

    m1 += adj_r;
    m2 -= adj_r;
    m3 -= adj_r;
    m4 += adj_r;

    m1 -= adj_y;
    m2 += adj_y;
    m3 -= adj_y;
    m4 += adj_y;

    set_motors( m1, m2, m3, m4 );

}

/****************************************************************/

static void
display_angle(void){

    printf("opress: %d oaz %d\n", offset_press, offset_az);
    while(1){
        printf("acc: %2d %2d %2d\n", GYRO2DEG(angle_ax),    GYRO2DEG(angle_ay),   GYRO2DEG(angle_mz));
        printf("gyr: %2d %2d %2d\n", GYRO2DEG(angle_gp),    GYRO2DEG(angle_gr),   GYRO2DEG(angle_gy));
        printf("flt: %2d %2d %2d\n", GYRO2DEG(angle_pitch), GYRO2DEG(angle_roll), GYRO2DEG(angle_yaw));
        printf("prs: %6d %6d\n",     press_lpf, baro_alt);
        printf("alt: %4d %4d %6d %6d\n", accz_lpf, offset_az, altitude, valtitude);

        printf("\n");
        usleep(100000);
    }
}


DEFUN(testfilter, "test sensor filter")
{
    short i;

    reset_control();
    play(volume, "a3a3a3");


    proc_t pid = start_proc( 1024, display_angle, "dpya" );

    while(1){
        if( check_button() )     break;
        if( check_upsidedown() ) break;

        read_all_sensors();
        filter_sensors();

        pause();
    }

    sigkill(pid);
    wait(pid);

    return 0;
}



