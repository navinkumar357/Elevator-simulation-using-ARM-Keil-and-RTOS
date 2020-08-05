/**
 * Program skeleton for the course "Programming embedded systems"
 *
 * Lab 1: the elevator control system
 */

/**
 * The planner module, which is responsible for consuming
 * pin/key events, and for deciding where the elevator
 * should go next
 */

#include "FreeRTOS.h"
#include "task.h"
#include <stdio.h>

#include "global.h"
#include "planner.h"
#include "assert.h"
#include "position_tracker.h"


#define MOTOR_UPWARD   (TIM3->CCR1)
#define MOTOR_DOWNWARD (TIM3->CCR2)
#define MOTOR_STOPPED  (!MOTOR_UPWARD && !MOTOR_DOWNWARD)
#define DOORS_CLOSED  GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_8)


#define debounced 1252
#define STOP_PRESSED  GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_3)

static void plannerTask(void *params) {
	int f1=0,f2=0,f3=0;
	int dist, started=0,bypass=0;
	PinEvent value;
	portTickType xLastWakeTime;
	int dir, spf=0;

  // ...
for(;;){
	xLastWakeTime = xTaskGetTickCount();
	
	if(MOTOR_UPWARD)
		dir=1;
	else if (MOTOR_DOWNWARD)
		dir=0;
	dist=getCarPosition();
	xQueueReceive( pinEventQueue, &( value ), 50 / portTICK_RATE_MS);
	
	if(STOP_PRESSED)
	{
		printf("Stopped \n");
		setCarMotorStopped(1);
		spf=1;
	}
	else if(STOP_RELEASED && spf==1)
	{
		printf("Released \n");
		spf=0;
		setCarMotorStopped(0);
	}
	if (value==TO_FLOOR_1)
		{
			printf("Floor 1\n");
			f1=1;
		}
	
	else if (value==TO_FLOOR_2)
		{
				printf("Floor 2\n");
				f2=1;
		}
	
	else if (value==TO_FLOOR_3)
		{
			printf("Floor 3\n");
			f3=1;
		}
			
		
		if(MOTOR_STOPPED){
			if(dist >= -1 && dist <= 1 && DOORS_CLOSED && started==1) {
        // reached floor 1
        printf("arrived at floor 1\n");
				f1=0;
				bypass=0;
				vTaskDelay(1000/portTICK_RATE_MS);
      }
      if (dist >= 399 && dist <= 401 && DOORS_CLOSED) {
        // reached floor 2
        printf("arrived at floor 2\n");
				f2=0;
				started=1;
				bypass=0;
				vTaskDelay(1000/portTICK_RATE_MS);

      }
      if (dist >= 799 && dist <= 801 && DOORS_CLOSED) {
        // reached floor 3
        printf("arrived at floor 3\n");
        f3=0;
				started=1;
				bypass=0;
				vTaskDelay(1000/portTICK_RATE_MS);
      }
		}
		
		/*To ensure motor continues in its direction rather than changing directions after reaching a floor*/
		
		if(dir==1){
			if (f2==1 && MOTOR_STOPPED && DOORS_CLOSED && dist<400){
				setCarTargetPosition(400);
				bypass=1;
			}
			else if (f3==1 && MOTOR_STOPPED && DOORS_CLOSED){
				setCarTargetPosition(800);
				bypass=1;
			}
		}
		else if(dir==0){
			if(f2==1 && MOTOR_STOPPED && DOORS_CLOSED && dist>400){
				setCarTargetPosition(400);
				bypass=1;
			}
			else if (f1==1 && MOTOR_STOPPED && DOORS_CLOSED){
				setCarTargetPosition(0);
				bypass=1;
			}
		}
		
		/*To go to a particuar floor to which the button was pressed*/
		
		if (f1==1 && MOTOR_STOPPED && DOORS_CLOSED && bypass==0){
			setCarTargetPosition(0);
		}
		
		else if (f2==1 && MOTOR_STOPPED && DOORS_CLOSED && bypass==0){
			setCarTargetPosition(400);	
		}
		
		else if (f3==1 && MOTOR_STOPPED && DOORS_CLOSED && bypass==0){
			setCarTargetPosition(800);
		}
		
		vTaskDelayUntil(&xLastWakeTime, 10 / portTICK_RATE_MS);
}
}

void setupPlanner(unsigned portBASE_TYPE uxPriority) {
  xTaskCreate(plannerTask, "planner", 100, NULL, uxPriority, NULL);
}
