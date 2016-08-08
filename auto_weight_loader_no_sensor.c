/***********************************************************************************************************************
Code:				Code for Automatic Loading-Unloading Assembly for Weighing Application (No Sensor)
Date Created:		3/12/2012
Last Update:		3/20/2012		
Description:		The Code describes the implementation of a loading-unloading assembly operated by a two DC motors
					and controlled by ARM Controller LPC2148. 
					- DC Motor 2 operates a gripper that open/closes to load/unload a workpiece
					- Dc Motor 1 controls a rack and pinion mechanism that pushes the gripper up and down to operate the 
					machinery
					The two motors are activated in synchronization to perform the action - the process is open loop 
					assuming constant velocity operation of the motors and therefore controlling the motion by time delays
					The Arm controller controls the motors via a relay driven driver circuit consisting of a H-bridge 
					converter to allow motion as well as direction control
					The pin connections are as follows:
					
					Motor 1 control 			- P0.31
					Motor 2 control 			- P0.10
					Motor 1 Direction Control 	- P0.30
					Motor 2 Direction Control 	- P0.22

***********************************************************************************************************************/
#include <LPC21xx.H>                       

#define DELAY 1000000
#define ENABLE_MOTOR_1						IO0CLR=0x80000000;							 // CLR P0.31 (T8) CLR P0.30 (T3) CLR P0.22 (T1)
#define DISABLE_MOTOR_1	 					IO0SET=0x80000000;			 				 // Set P0.31 (T8) SET P0.30 (T3) SET P0.22 (T1)	 
#define DISABLE_MOTOR_2						IO0SET=0x00000400;IO1SET=0x02000000			 // SET P0.10 (T6) SET P1.25 (T7) 
#define ENABLE_MOTOR_2						IO0CLR=0x00000400;IO1CLR=0x02000000			 // CLR P0.10 (T6) CLR P1.25 (T7)
#define SET_DIRECTION_CLOCKWISE_MOTOR_1		IO0SET=0x40000000;IO0CLR=0x00400000     	 // SET P0.30 (T3) CLR P0.22 (T1)
#define SET_DIRECTION_ANTICLOCKWISE_MOTOR_1	IO0CLR=0x40000000;IO0SET=0x00400000	    	 // CLR P0.30 (T3) SET P0.22 (T1)
#define SET_DIRECTION_CLOCKWISE_MOTOR_2 	IO1SET=0x02000000;IO0CLR=0x00000400   	     // SET P1.25 (T7) CLR P0.10 (T6) 
#define SET_DIRECTION_ANTICLOCKWISE_MOTOR_2 IO1CLR=0x02000000;IO0SET=0x00000400   	 	 // CLR P1.25 (T7) Set P0.10 (T6)

// Determines the degree with which the motor rotates. To increase the degree of rotation
// Increase the value of delay. 
   
#define ROTATION_DELAY_MOTOR_1				2500000	 									// determines degree of rotation for Motor 1
#define ROTATION_DELAY_MOTOR_2_OPEN  		5000000	 									// determines degree of rotation for Motor 2 while opening
#define ROTATION_DELAY_MOTOR_2_CLOSE 		5100000	 									// determines degree of rotation for Motor 2 while closing
#define DELAY_FOR_STABILITY					100000000		 							// waits for stability
#define REPEAT_CYCLE_DELAY					20000000		 							// Adjustable delay between two cycles
#define SENSOR_INPUT 						IO0PIN		 


int main (void)
 {
  PINSEL0 = 0x00000000;
  PINSEL1 = 0x00000000;
  IO0DIR  = 0xC0400400;			 							// P0.30(T3), P0.31(T8), P0.22(T1), P0.10(T6)
  IO1DIR  = 0x02000000;		     							// P1.25 (T7)   
  DISABLE_MOTOR_1;
  
  IO0SET  = 0x40000000;
  IO0SET  = 0x00400000;
  DISABLE_MOTOR_2;

	while(1)												// Repeats loading - unloading cycle indefinitely
	{ 
	unsigned long temp; 									// used for delays: local variable
	for(temp = REPEAT_CYCLE_DELAY;temp>1;temp--);
	
	//Push Gripper downward
	 SET_DIRECTION_CLOCKWISE_MOTOR_1;						
	 ENABLE_MOTOR_1;  																	  
	 for(temp = ROTATION_DELAY_MOTOR_1;temp>1;temp--);		

	 DISABLE_MOTOR_1;										
	 IO0SET=0x40000000;										
	 IO0SET = 0x00400000;
	 for(temp = 20000000;temp>1;temp--);
	 
	 //Open Gripper to load workpiece for weighing
	 SET_DIRECTION_CLOCKWISE_MOTOR_2; 						
	 ENABLE_MOTOR_2;
	 for(temp = ROTATION_DELAY_MOTOR_2_OPEN;temp>1;temp--);

	 DISABLE_MOTOR_2;										 
	 for(temp = DELAY_FOR_STABILITY;temp>1;temp--);		

	//Close gripper to grip the workpiece back again
	 SET_DIRECTION_ANTICLOCKWISE_MOTOR_2;
	 ENABLE_MOTOR_2;
	 for(temp = ROTATION_DELAY_MOTOR_2_CLOSE;temp>1;temp--);
	 
	 DISABLE_MOTOR_2;
	 for(temp = 20000000;temp>1;temp--);
	 
	 //Pull Gripper Upwards
	 SET_DIRECTION_ANTICLOCKWISE_MOTOR_1;
	 ENABLE_MOTOR_1;
	 for(temp = ROTATION_DELAY_MOTOR_1;temp>1;temp--);
	 for(temp = 1500000;temp>1;temp--);

	 DISABLE_MOTOR_1;
	 IO0SET=0x40000000;
	 IO0SET = 0x00400000;	   
	 
	}		 // end of while loop

}



