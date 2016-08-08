/***********************************************************************************************************************
Code:				Code for Automatic Loading-Unloading Assembly for Weighing Application (With Sensor)
Date Created:		3/12/2012
Last Update:		3/20/2012		
Description:		The Code describes the implementation of a loading-unloading assembly operated by a two DC motors
					and controlled by ARM Controller LPC2148. 
					- DC Motor 2 operates a gripper that open/closes to load/unload a workpiece
					- Dc Motor 1 controls a rack and pinion mechanism that pushes the gripper up and down to operate the 
					machinery
					The two motors are activated in synchronization to perform the action - the process is closed loop 
					assuming varying velocity operation of the motors with change in load and therefore controlling 
					the motion using slit sensor interrupts.
					The Arm controller controls the motors via a relay driven driver circuit consisting of a H-bridge 
					converter to allow motion as well as direction control.
					The status of the operation is displayed on a UART Terminal - displays success or failure of each loading
					and unloading operation.
					
					The pin connections are as follows:
					
					Motor 1 control 			- P0.31
					Motor 2 control 			- P0.10
					Motor 1 Direction Control 	- P0.30
					Motor 2 Direction Control 	- P0.22

***********************************************************************************************************************/

#include<LPC21XX.H>

#define ENABLE_MOTOR_1		IO0CLR=0x82400000			 										// CLR P0.31 (T8) CLR P0.30 (T3) CLR P0.22 (T1)
#define DISABLE_MOTOR_1	 	IO0SET=0x82400000
#define DISABLE_MOTOR_2		IO1SET=0x02000000;	IO0SET=0x00000400			 					// Set P0.31 (T8) SET P0.30 (T3) SET P0.22 (T1)	 
#define DISABLE_MOTOR_2		IO0SET=0x00000400;	IO1SET=0x02000000			 				    // SET P0.10 (T6) SET P1.25 (T7) 
#define ENABLE_MOTOR_2		IO0CLR=0x00000400;	IO1CLR=0x02000000			 				    // CLR P0.10 (T6) CLR P1.25 (T7)

#define SET_DIRECTION_CLOCKWISE_MOTOR_1			IO0SET=0x02000000;	IO0CLR=0x80400000     		// SET P0.30 (T3) CLR P0.22 (T1)
#define SET_DIRECTION_ANTICLOCKWISE_MOTOR_1		IO0CLR=0x82000000;	IO0SET=0x00400000   		// CLR P0.30 (T3) SET P0.22 (T1)
#define SET_DIRECTION_CLOCKWISE_MOTOR_2 		IO1SET=0x02000000;	IO0CLR=0x00000400  	    	//SET P1.25 (T7) CLR P0.10 (T6) 
#define SET_DIRECTION_ANTICLOCKWISE_MOTOR_2 	IO1CLR=0x02000000;	IO0SET=0x00000400  			//CLR P1.25 (T7) Set P0.10 (T6)

#define ROTATION_DELAY_MOTOR_1					2500000	 										// determines degree of rotation for Motor 1
#define ROTATION_DELAY_MOTOR_2_OPEN  			1500000	 										// determines degree of rotation for Motor 2 while opening
#define ROTATION_DELAY_MOTOR_2_CLOSE 			1550000	 										// determines degree of rotation for Motor 2 while closing
#define DELAY_FOR_STABILITY						10000000		 						    	// waits for stability
#define REPEAT_CYCLE_DELAY						20000000		 								// Adjustable delay between two cycles
#define DELAY_FOR_POSITION_CHECK				2800000											// delay for checking motor position
#define DELAY_FOR_MOTOR_CHECK					500000											// delay for checking if motor is jammed
#define SENSOR_INPUT 							IO0PIN 								 			// selecting P0.30 as sensor input


  int 	MOT_FLAG = 0;									// Flag for motor position, intialised for position1 = 0
  int   STOP_MOT = 0;				 					// Flag for stopping motor
 
  int count = 0;
  char commanbuffer[30];

  unsigned long 	i;
  unsigned long 	j;
 
 void SerialPort_Initialize(void)
{
	PINSEL0 |= 0x00000005;    							// select TX and RX port pins                            
	U0LCR = 0x83;             							// 8 bits, no Parity, 1 Stop bit 
	U0DLM = 0x00;					
	U0DLL = 0x29;    	
	U0LCR = U0LCR & 0x7f; 	  							// disable MSB
	U0FCR   = 0x00000001;    							//Enable FIFO & trigger level to 1  bytes      
	VICIntSelect = 0x00;
}
 
unsigned char putchr (unsigned char ch) 
{
   while (!(U0LSR & 0x20)); 							// wait for TX buffer to empty 
   return (U0THR = ch);	   								// put char to Transmit Holding Register
}
	   
void uart0Puts(const char *string)
{
	 char ch;
	 const char *dummy_string;
	
	 dummy_string = string;
	 while ((ch = *dummy_string)!=0) 
	 {
	 	if (putchr(ch)<=0x00) {break;} 				// check end of string
	 	dummy_string++;
	 }
	 
}
  
  int main(void)
{
  PINSEL0 = 0x00000000;
  PINSEL1 = 0x00000000;
  IO0DIR  = 0x82400400;			 						// P0.25(P1), P0.31(T8), P0.22(T1), P0.10(T6)selscted as outputs, P0.30(T3)	selected as input
  IO1DIR  = 0x02000000;		     						// P1.25 (T7) selected as input  

  DISABLE_MOTOR_1;
  IO0SET = 0x82400400;
  IO1SET = 0x02000000; 

  SerialPort_Initialize();


 while(1)
{
    unsigned long temp; 	

	if((SENSOR_INPUT & 0x40000000) == 0x00000000)	{uart0Puts("High_Detect_1 \r\n"); 		
													 for(temp=0; temp<=1000; temp++);		}										

	if((SENSOR_INPUT & 0x40000000) == 0x40000000)	{uart0Puts("High_Glitch_1 \r\n");
													 goto mot_cont;							}


	if((SENSOR_INPUT & 0x40000000) == 0x00000000) 	{uart0Puts("High \r\n");			// Sensor HIGH and MOT_FLAG = 0 indicates motor at position 1
													 for(temp=0; temp<=5000; temp++);	   
													 uart0Puts("UP_Position \r\n"); 
													 MOT_FLAG=0;						//	MOT_FLAG set to 0 again after one cycle	
													/*  while(1)
													{
														 if(MOT_FLAG==1)	 					
														 {												
														 MOT_FLAG=0;
														 break;
														 }
														 else
														 {
														 break;
														 }
													}
													*/
mot_cont_3:											 DISABLE_MOTOR_1;
 													 for(temp=0; temp<=REPEAT_CYCLE_DELAY; temp++);
													 ENABLE_MOTOR_1;
													 SET_DIRECTION_CLOCKWISE_MOTOR_1;
													 while((SENSOR_INPUT & 0x40000000) == 0x00000000);		}		// Move motor downward to position2	  
																															
mot_cont_1:		
	if((SENSOR_INPUT & 0x40000000) == 0x40000000)	{uart0Puts("Low_Detect_1 \r\n"); 
													 for(temp=0;temp<=5000;temp++);		}
		
		else										{uart0Puts("Low_Detect_1 not working \r\n"); 
													 goto mot_cont_3;						}

	if((SENSOR_INPUT & 0x40000000) == 0x00000000)	{uart0Puts("Low_Glitch_1 \r\n");
													 goto mot_cont_3;   					}
	
	while((SENSOR_INPUT & 0x40000000)==0x40000000) {uart0Puts("Moving_Down \r\n");
													 for(temp=0;temp<=1000;temp++);		}
	
	uart0Puts("Low \r\n");

	if ((SENSOR_INPUT & 0x40000000) == 0x00000000) {uart0Puts("High_Detect_2 \r\n");
													 for(temp=0;temp<=1000;temp++);		}
	
	if((SENSOR_INPUT & 0x40000000) != 0x00000000)	{uart0Puts("High_Glitch_2 \r\n");
													 goto mot_cont_1;						}
   
	if ((SENSOR_INPUT & 0x40000000)==0x00000000)	{uart0Puts("DOWN_Position \r\n");				// Sensor HIGH for second time indicates  motor at position 2 
													 //uart0Puts("High \r\n"); 
													 //for(temp=0; temp<=1000; temp++);
													 DISABLE_MOTOR_1;
													 /*if(MOT_FLAG == 1) break;*/
													 MOT_FLAG=0;
												     /*	while(1)											 
													{	if(MOT_FLAG == 0) 	{MOT_FLAG=1;						// MOT_FLAG set to 1 for position 2
																			 break;			}
														else				{break;			}
													}
  */
	
	for(temp=0; temp<=1000000; temp++);
		
	SET_DIRECTION_CLOCKWISE_MOTOR_2;								 	// Move Motor2	clockwise to unload
	 for(temp = ROTATION_DELAY_MOTOR_2_OPEN; temp>1; temp--);
	
	DISABLE_MOTOR_2;												 
	 for(temp = DELAY_FOR_STABILITY; temp>1; temp--);					// wait for stability 
	
	SET_DIRECTION_ANTICLOCKWISE_MOTOR_2; 							 	// Move Motor2 anticlockwise to load
	 for(temp = ROTATION_DELAY_MOTOR_2_CLOSE; temp>1; temp--);		 	
	 for(temp=0; temp<=1000000; temp++);



mot_cont_2:		
	DISABLE_MOTOR_2;
	ENABLE_MOTOR_1;
	SET_DIRECTION_ANTICLOCKWISE_MOTOR_1;							   // Move Motor1 anticlockwise to return to position 1
	for(temp=0; temp<=10000; temp++);
										   
	while((SENSOR_INPUT & 0x40000000) == 0x00000000);						
																							}

mot_cont: 	
	if((SENSOR_INPUT & 0x40000000) == 0x40000000)	{uart0Puts("Low_Detect_2 \r\n");
													 for(temp=0; temp<=5000; temp++);		}
	else											{uart0Puts("Low_Detect_2 not working \r\n");
													 goto mot_cont_2; 						}

	if((SENSOR_INPUT & 0x40000000) == 0x00000000)	{uart0Puts("Low_Glitch_2 \r\n");
													 goto mot_cont_2;  						}
	else											{uart0Puts("Low_Glitch_2 not working \r\n");}

	while((SENSOR_INPUT & 0x40000000)==0x40000000) {uart0Puts("Moving_up \r\n");	// Keep Motor1 on until Sensor HIGH
													 //uart0Puts("Low \r\n");
							 						 /*for(temp=0; temp<=1000; temp++);*/	}
 
    count = count + 1;
    sprintf(commanbuffer,"Total Counts = %d \r\n", count); 	
	uart0Puts(commanbuffer);

	}
 
}
