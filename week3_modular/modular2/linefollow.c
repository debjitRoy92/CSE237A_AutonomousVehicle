#include "create.h"
#include <stdio.h>
#include <wiringPi.h>
#include <softPwm.h>
#include <stdint.h>

void init_shared_variable(SharedVariable* sv) {
	sv->bProgramExit = 0;
	sv->drive_state = STOP_WAIT;
	sv->init_start = 0;
	sv->manual_stop = 0;
	sv->current_direction = STOP;
	turn = 1;	
}

void init_sensors(SharedVariable* sv) {
	pinMode(LEFT_LED, INPUT);
	pinMode(RIGHT_LED, INPUT);
	pinMode(FRONT_LED, INPUT);
/*	pinMode(US_1_ECHO, INPUT);
	pinMode(US_1_TRIG, OUTPUT);
	pinMode(US_2_ECHO, INPUT);
	pinMode(US_2_TRIG, OUTPUT);
	pinMode(US_3_ECHO, INPUT);
	pinMode(US_3_TRIG, OUTPUT);
	pinMode(US_4_ECHO, INPUT);
	pinMode(US_4_TRIG, OUTPUT);
	pinMode(US_5_ECHO, INPUT);
	pinMode(US_5_TRIG, OUTPUT);
*/
}
void setup_US(int TRIG, int ECHO){
	pinMode(TRIG, INPUT);
        pinMode(ECHO, OUTPUT);
        digitalWrite(ECHO, LOW);

        //TRIG pin must start LOW
        delay(30);
        pinMode(TRIG, OUTPUT);
        pinMode(ECHO, INPUT);
 
        //TRIG pin must start LOW
        digitalWrite(TRIG, LOW);
        delay(30);
}
int getCM(int TRIG, int ECHO) {
        //Send trig pulse
        digitalWrite(TRIG, HIGH);
        delayMicroseconds(20);
        digitalWrite(TRIG, LOW);
 
        //Wait for echo start
        while(digitalRead(ECHO) == LOW);
 
        //Wait for echo end
        long startTime = micros();
        while(digitalRead(ECHO) == HIGH);
        long travelTime = micros() - startTime;
 
        //Get distance in cm
        int distance = travelTime / 58;
 
        return distance;
}
void body_ultrasound(SharedVariable* sv) {
	int dist_2,dist_3,dist_4;
	while(1){
	//setup(US_1_TRIG,US_1_ECHO);
	setup_US(US_2_TRIG,US_2_ECHO);
	setup_US(US_3_TRIG,US_3_ECHO);
	setup_US(US_4_TRIG,US_4_ECHO);
	//setup(US_5_TRIG,US_5_ECHO);

	dist_2 = getCM(US_2_TRIG,US_2_ECHO);
	dist_3 = getCM(US_3_TRIG,US_3_ECHO);
	dist_4 = getCM(US_4_TRIG,US_4_ECHO); 
	if((dist_2>5 && dist_2<30)||(dist_3>5 && dist_3<30)||(dist_4>5 && dist_4<30))
	{
		sv->drive_state = OBSTACLE_DETECTED;	
	}	
	}
}

void body_irled(SharedVariable* sv) {
	while(1){
	if(!digitalRead(LEFT_LED))
		sv->left_led = 1;
	else
		sv->left_led = 0;
	if(!digitalRead(RIGHT_LED))
		sv->right_led = 1;
	else
		sv->right_led = 0;
	if(!digitalRead(FRONT_LED))
		sv->front_led = 1;
	else
		sv->front_led = 0;
	}
}


void body_linefollow(SharedVariable* sv){
	while(!sv->init_start);
	printf("After Init_start\n");
	while(!sv->manual_stop)
	{	
	  if((!sv->front_led)){
		sv->drive_state = DRIVE_FORWARD;
		}
	  else{	
		if(turn == 1){
		sv->drive_state = TURN_RIGHT;
		printf("turn right set\n");
		stop(sv);
		}
		else if(turn == 2)
		sv->drive_state = TURN_LEFT;
		
		//turn = 0;		
	  }
		  	
	  switch(sv->drive_state){
		case DRIVE_FORWARD:
	if(!sv->right_led && !sv->left_led)
	{
		//printf("Forward");
		forward(sv);
		//delay(1000);	
	}	
	if(sv->right_led) //right is off implies both scenarios where left and right both are off
	{
		
		while(sv->right_led){		
			left(sv);
			delay(50);
			forward(sv);
			delay(100);
		}
		right(sv);
		delay(33);
		forward(sv);	
	}
	if(sv->left_led) 
	{
		while(sv->left_led){		
			right(sv);
			delay(50);
			forward(sv);
			delay(100);
		}
		left(sv);
		delay(33);
		forward(sv);
			
	}				
			//drive_forward(sv);
			break;
		case TURN_RIGHT:	
		printf("ENtered turn right\n");
		while(sv->left_led != 1 || sv->right_led != 1)
		{	forward(sv);			
			delay(50);		
		}	
		if (sv->right_led == 1) {
			while (sv->left_led != 1)
				{right(sv);
				 delay(300);}		
		}	
		printf("before while\n");
		while(sv->left_led == 1)		
		{	right(sv);
			delay(50);	
		}		
		printf("after while\n");
		while(sv->left_led != 1 && sv->front_led != 1) 
		{	forward(sv);
			delay(50);		
		}		
		printf("after forward\n");
		if (sv->front_led == 1) {
			while (sv->left_led != 1) 
				{right(sv);
				  delay(50);}
			printf("left led on\n");
		}

		//if(sv->left_led == 1) {
		        //long startTime = micros();
			while (sv->left_led == 1) 
				{right(sv);
				  //delay(50);
				}	
			printf("left led off\n");
			//long rotateTime = micros() - startTime;
			//printf("rotate time is %d\n",rotateTime);
			//forward(sv);
			//delay(1000);
			//printf("turning left");
			//left(sv);
			//printf("left turned");			
			//delay(1000);
			//stop(sv);
		//}
		//	turn_right(sv);
			break;
		case TURN_LEFT:	
			turn_left(sv);
			break;
		case OBSTACLE_DETECTED:	
			lane_change(sv);
			break;
		case STOP_WAIT:
			stop(sv);
			break;

		default : break;	
		  }	
	}

	printf("Manual Stop is %d\n",sv->manual_stop);
	stop();

}

void body_keypress(SharedVariable* sv)
{
	int x = ' ';
	int y = ' ';
 	int z = ' ';

// Initialize Irobot
 	create_init(sv);
	printf("press aaa\n");
 while (!(x == 'a' && y == 'a' && z == 'a')) { // 3 times ESC
	sv->init_start = 0;
	
	x = getch();
	y = getch();
	z = getch(); 
	}
	printf("pressed aaa\n");

 sv->init_start =1;

  while (1) {
	x = getch();
	y = getch();
	z = getch(); 

	if (x == 27 && y == 27 && z == 27) {
	sv->manual_stop = 1;
	break;
	}

  }	
	stop(sv);	
	printf("pressed e e e \n");

}

void drive_forward(SharedVariable* sv)
{
	if(!sv->right_led && !sv->left_led)
	{
		//printf("Forward");
		forward(sv);
		//delay(1000);	
	}	
	if(sv->right_led) //right is off implies both scenarios where left and right both are off
	{
		
		while(sv->right_led){		
			left(sv);
			delay(50);
			forward(sv);
			delay(100);
		}
		right(sv);
		delay(33);
		forward(sv);	
	}
	if(sv->left_led) 
	{
		while(sv->left_led){		
			right(sv);
			delay(50);
			forward(sv);
			delay(100);
		}
		left(sv);
		delay(33);
		forward(sv);
			
	}				
}

void turn_right(SharedVariable* sv)
{
		printf("ENtered turn right\n");
		while(sv->left_led != 1 || sv->right_led != 1)
		{	forward(sv);			
			delay(50);		
		}	
		if (sv->right_led == 1) {
			while (sv->left_led != 1)
				{right(sv);
				 delay(300);}		
		}	
		printf("before while\n");
		while(sv->left_led == 1)		
		{	right(sv);
			delay(50);	
		}		
		printf("after while\n");
		while(sv->left_led != 1 && sv->front_led != 1) 
		{	forward(sv);
			delay(50);		
		}		
		printf("after forward\n");
		if (sv->front_led == 1) {
			while (sv->left_led != 1) 
				{right(sv);
				  delay(50);}
			printf("left led on\n");
		}

		//if(sv->left_led == 1) {
		        //long startTime = micros();
			while (sv->left_led == 1) 
				{right(sv);
				  delay(50);}	
			printf("left led off\n");
			//long rotateTime = micros() - startTime;
			//printf("rotate time is %d\n",rotateTime);
			forward(sv);
			delay(1000);
			printf("turning left");
			left(sv);
			printf("left turned");			
			delay(1000);
			//stop(sv);
		//}
		
}

void turn_left(SharedVariable* sv)
{
		while(sv->left_led != 1 && sv->right_led != 1)
			forward(sv);			

		while(sv->front_led != 1)
			forward(sv);			

		while(sv->left_led != 1 && sv->right_led != 1)
			forward(sv);			

		while(sv->left_led == 1 && sv->right_led == 1)		
			left(sv);

		while(sv->right_led != 1 && sv->front_led != 1) 
			forward(sv);
		
		if (sv->front_led == 1) {
			while (sv->right_led != 1) 
				left(sv);
		}

		//if(sv->right_led == 1) {
		        long startTime = micros();
			while (sv->right_led == 1) 
				left(sv);
			int rotateTime = micros() - startTime;
			forward(sv);
			delay(100);
			right(sv);			
			usleep(rotateTime/2);
			stop(sv);
		//}
}

void lane_change(SharedVariable* sv)
{}