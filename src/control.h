

#define MAG_OFF_X	-11000
#define MAG_OFF_Y	2000
#define MAG_OFF_Z	0

// motor control constants
#define MOT_MIN		20	// max pwm value when motors don't move
#define MOT_SLOW	50	// pwm value when motors are slowly spinning
#define MOT_LIFT	450	// pwm value when device just starts to lift

// PID constants
// pitch
#define KPP		0.000045
#define KIP		0.00000015
#define KDP		0.03375
// roll - vehicle is symmetric x/y
#define KPR		KPP
#define KIR		KIP
#define KDR		KDP
// yaw
#define KPY	        0.00009
#define KIY		0.0000003
#define KDY		0.0675
// altitude (PD)
#define KPA		0.012
#define KDA		0.0006




