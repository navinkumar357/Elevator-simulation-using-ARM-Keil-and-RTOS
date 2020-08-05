/**
 * Program skeleton for the course "Programming embedded systems"
 *
 * Lab 1: the elevator control system
 */

/**
 * This file defines the safety module, which observes the running
 * elevator system and is able to stop the elevator in critical
 * situations
 */

#include "FreeRTOS.h"
#include "task.h"
#include "stm32f10x_tim.h"
#include "stm32f10x_gpio.h"
#include <stdio.h>

#include "global.h"
#include "assert.h"

#define POLL_TIME (10 / portTICK_RATE_MS)

#define MOTOR_UPWARD   (TIM3->CCR1)
#define MOTOR_DOWNWARD (TIM3->CCR2)
#define MOTOR_STOPPED  (!MOTOR_UPWARD && !MOTOR_DOWNWARD)

#define STOP_PRESSED  GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_3)
#define AT_FLOOR      GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_7)
#define DOORS_CLOSED  GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_8)

static portTickType xLastWakeTime;
	vs32 spdchk=-1,p1=0,p2=0,spd;
	vs32 motpos, wostp=0,df=0;
	int started=0;

static void check(u8 assertion, char *name) {
  if (!assertion) {
    printf("SAFETY REQUIREMENT %s VIOLATED: STOPPING ELEVATOR\n", name);
    for (;;) {
	  setCarMotorStopped(1);
  	  vTaskDelayUntil(&xLastWakeTime, POLL_TIME);
    }
  }
}

static void safetyTask(void *params) {
  s16 timeSinceStopPressed = -1, timeSinceAtFloor = -1, timeSinceDoorOpened = -1;

  xLastWakeTime = xTaskGetTickCount();
  for (;;) {
		
		
  // Environment assumption 1: the doors can only be opened if
	//                           the elevator is at a floor and
  //                           the motor is not active

	check((AT_FLOOR && MOTOR_STOPPED) || DOORS_CLOSED,
	       "env1");

		
	// Environment assumption 2: The elevator moves at a 
	//													 maximum speed of 50cm/s
		
	if(spdchk==0){
		p2=getCarPosition();
		spdchk++;
	}
	else if(spdchk==-1){
		p1=getCarPosition();
		spdchk++;
	}
	if(spdchk==1){
		if (p1>p2)
			spd=(p1-p2)/POLL_TIME;
		else if (p1<p2)
			spd=(p2-p1)/POLL_TIME;
		spdchk=-1;}
	check(spd<=50, "env2");


	
	// Environment assumption 3: If the ground floor is put at 0cm 
	//													 in an absolute coordinate system, 
	//													 the second floor is at 400cm and the 
	//													 third floor at 800cm (the at-floor sensor 
	//													 reports a floor with a threshold of +-0.5cm)
		
	if (AT_FLOOR && MOTOR_STOPPED){
		motpos=getCarPosition();
		check(AT_FLOOR &&(
					(-0.5 <= motpos && motpos <= 0.5) || 		//First Floor Limits
					(399.5 <= motpos && motpos <= 400.5) || //Second Floor Limits
					(799.5 <= motpos && motpos <= 800.5 )	)	//Third Floor Limits
					, "env3");
	}
	
	// If Doors have opened they should close after atleast 1s
	if (!DOORS_CLOSED) {
	  if (timeSinceDoorOpened < 0){
	    timeSinceDoorOpened = 0;
			df=1;
		}
     else
	    timeSinceDoorOpened += POLL_TIME;
	} 
	else if(DOORS_CLOSED && df==1){
		check(timeSinceDoorOpened * portTICK_RATE_MS >= 1000,
	        "env4");
		timeSinceDoorOpened = -1;
		df=0;
	}

	
  // System requirement 1: if the stop button is pressed, the motor is
	//                       stopped within 1s

	if (STOP_PRESSED) {
	  if (timeSinceStopPressed < 0)
	    timeSinceStopPressed = 0;
      else
	    timeSinceStopPressed += POLL_TIME;

      check(timeSinceStopPressed * portTICK_RATE_MS <= 1000 || MOTOR_STOPPED,
	        "req1");
	} else {
	  timeSinceStopPressed = -1;
	}

    // System requirement 2: the motor signals for upwards and downwards
	//                       movement are not active at the same time

    check(!MOTOR_UPWARD || !MOTOR_DOWNWARD,
          "req2");

	// fill in safety requirement 3
	motpos=getCarPosition();
	check((motpos<=800.5) ||				//to compensate for the +-0.5cm error of the the at-floor sensor
				(motpos>=-0.5), "req3");	//we have adjusted the limits accordingly.

	// fill in safety requirement 4.1
	//check(!(MOTOR_STOPPED && STOP_PRESSED) || !(MOTOR_STOPPED && AT_FLOOR), "req4");
	
	// fill in safety requirement 4.2
	if((MOTOR_STOPPED && STOP_PRESSED) && (MOTOR_STOPPED && AT_FLOOR)) // if the elevator is at floor and stop pressed then it should not violate the
	{																																	 //safety requirement..	
		check(1,"req4");
	}
	else
		check(!(MOTOR_STOPPED && STOP_PRESSED) || !(MOTOR_STOPPED && AT_FLOOR), "req4");
	

	// fill in safety requirement 5
	if ((AT_FLOOR && MOTOR_STOPPED) && started==1) {
	  if (timeSinceAtFloor < 0){
	    timeSinceAtFloor = 0;
			wostp=1;
			printf("FLAG SET\n");
		}
     else
	    timeSinceAtFloor += POLL_TIME;
	} 
	else if((MOTOR_UPWARD || MOTOR_DOWNWARD) && wostp==1){
		check(timeSinceAtFloor * portTICK_RATE_MS >= 1000,
	        "req5");
		timeSinceAtFloor = -1;
		wostp=0;
	}
		
	if((MOTOR_UPWARD || MOTOR_DOWNWARD) && started==0)
	{
		started = 1;
	}
	

	// If the doors are open, the motor should not move
	if(!MOTOR_STOPPED){
		if(!DOORS_CLOSED){
			check(0, "req6");
		}
	}


	vTaskDelayUntil(&xLastWakeTime, POLL_TIME);
  }

}

void setupSafety(unsigned portBASE_TYPE uxPriority) {
  xTaskCreate(safetyTask, "safety", 100, NULL, uxPriority, NULL);
}
