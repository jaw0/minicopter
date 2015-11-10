// Copyright (c) 2015
// Author: Jeff Weisberg <jaw @ tcp4me.com>
// Created: 2015-Oct-04 22:32 (EDT)
// Function: frame for mini-copter


show_env   = 0;

pcb_x      = 25.0;
pcb_y      = 25.0;
pcb_z      =  1.6;

peg_pos    = 1.25;	// in from corner
prop_x     = 56.0;	// prop diam plus margin
mot_z      = 10.0; 	// ht of center of motor
motor_phi  = 5.0;	// slight dihedral
motor_diam = 7;


//################################################################

mot_pos = (prop_x - pcb_x) / 2;
mot_armang = atan( (mot_z + 2) / (mot_pos + peg_pos) / sqrt(2) );

$fn = 64;
$fa = .5;
$fs = 0.05;

//################################################################

module vcyl(x,y,z,diam1, diam2, len){
    translate([x,y,z])
        cylinder(r1=diam1/2, r2=diam2/2, h=len);
}

module hcyl(x,y,z,diam1, diam2, len){
    translate([x,y,z])
        rotate([0,90,0])
            cylinder(r1=diam1/2, r2=diam2/2, h=len);
}

module box(x,y,z, dx,dy,dz){
    translate([x, y, z]) cube([dx, dy, dz]);
}


module proparm(){

    // board support+attach
    translate([ (pcb_x/2-peg_pos)*sqrt(2), 0, 0]) rotate([0,0,45]){
        vcyl(0,0,-4, 3.5, 3.5,4);
        // slightly tapered pins
        vcyl(0,0,-.1, 1.0, 0.85, 2.1);
        box(-3.5,-.5,-1.5, 4,1,1.5);
        box( -.5,-.5,-1.5, 1,4,1.5);

    }

    // legs
    translate([ (pcb_x/2-peg_pos)*sqrt(2), 0, -3.0]) rotate([0,30,0]){

        intersection(){
            union(){
                box(0,-1.5,2.0, 25, 3, 1);
                rotate([0,-3,0]) box(0,-.5,0, 25, 1, 3);
            }
            box(0,-1.5,-1, 25, 3, 4);
            hcyl(0,0,2.5, 5.3,2.25, 25);
        }
        translate([25,0,1.25]) sphere(2.0);
    }

    difference(){
        union(){
            // motor arms
            intersection(){
                translate([ (pcb_x/2-peg_pos)*sqrt(2), 0, -2.0]) rotate([0,-mot_armang,0]) {
                    box(0, -1.5, 0,   (mot_pos+peg_pos)*sqrt(2) / cos(mot_armang), 3.0,1 );
                    box(0,-.5,  -2, (mot_pos+peg_pos)*sqrt(2) / cos(mot_armang), 1,3.0 );
                }
                // clean up the bottom
                box(-5,-5,-4, 60,60,60);
            }
            // motor mounts
            translate([prop_x/2 * sqrt(2),0, mot_z]) rotate([0,-motor_phi,0]){
                vcyl(0,0,-3.5, motor_diam+2, motor_diam+2, 6.5 );

                // prop
                if( show_env ) color([.75,0,.75,1]) vcyl(0,0,5, 45, 45, 5);
            }

        }
        union(){
            // hole for motor
            translate([prop_x/2 * sqrt(2),0, mot_z]) rotate([0,-motor_phi,0]){
                vcyl(0,0,-5, motor_diam-.5, motor_diam-.5, 9 );
                rotate([0,0,90]) box(motor_diam/2-2, -.5,-5, 5,1,9);
            }
        }
    }
}

module cage(){
    // outer box
    difference(){
        union(){
            box( 28/2+.5, -28/2-2, -2.0, 2.0, 28+4, 4.0);
            box( 28/2-.5,  -28/2-2, -1.0, 2.0, 28+4, 2.0);
        }
        // clean up/bevel the corners
        union(){
            for(a = [-45, 45]){
                rotate([0,0,a])
                    translate([ (pcb_x/2-peg_pos)*sqrt(2), 0, 0])
                        rotate([0,-mot_armang,0])
                            box(-2,-4,-6.1,  8,8,2);
            }
        }
    }
    // rubber band attachment pegs
    hcyl(28/2+.5, -6, 0,  2,2,5);
    hcyl(28/2+.5,  6, 0,  2,2,5);
    hcyl(28/2+.5, -2, 0,  2,2,5);
    hcyl(28/2+.5,  2, 0,  2,2,5);

}


// ################################################################

if( show_env ) color([.75,0,.75, 1]) box(-pcb_x/2, -pcb_y/2, 0, pcb_x, pcb_y, pcb_z);

for( angle = [ 0 : 90 : 270 ] ){
    rotate([0,0,45+angle]) proparm();

    // outer cage
    rotate([0,0,angle]) translate([0,0,.4]) cage();

}




