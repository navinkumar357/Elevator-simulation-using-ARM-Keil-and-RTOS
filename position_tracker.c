/**
 * Program skeleton for the course "Programming embedded systems"
 *
 * Lab 1: the elevator control system
 */

/**
 * Class for keeping track of the car position.
 */

#include "FreeRTOS.h"
#include "task.h"
#include "stdio.h"

#include "position_tracker.h"

#include "assert.h"

int prevsate = 0;
int presentstate = 0;
 
portTickType xLastWakeTime;


static void positionTrackerTask(void *params) {
  PositionTracker *newPT=(PositionTracker *)params; //typecasting back from (void *) to (PositionTracker *)
	//setCarTargetPosition(400);
	//prevstate=GPIO_ReadInputDataBit(newPT->gpio,newPT->pin);

	for (;;) {
		xLastWakeTime = xTaskGetTickCount();
		presentstate = GPIO_ReadInputDataBit(newPT->gpio, newPT->pin);  // reads pin 9
		if (presentstate > prevsate) {
			if(xSemaphoreTake(newPT->lock, portMAX_DELAY)){
				if (newPT->direction == Up) {
					newPT->position++;
				}
				else if (newPT->direction == Down) {
					newPT->position--;
				}
				else if (newPT->position == Unknown) {
					newPT->position = newPT->position;
					// printf("Nothing\n");
				}
				printf("%ld\n",newPT->position);
				xSemaphoreGive(newPT->lock);
			}
			else printf("Semaphore take failed\n");
    }
		prevsate = presentstate;
   vTaskDelayUntil(&xLastWakeTime, newPT->pollingPeriod);
  }
  
}

void setupPositionTracker(PositionTracker *tracker,
                          GPIO_TypeDef * gpio, u16 pin,
						  portTickType pollingPeriod,
						  unsigned portBASE_TYPE uxPriority) {
  portBASE_TYPE res;

  tracker->position = 0;
  tracker->lock = xSemaphoreCreateMutex();
  assert(tracker->lock != NULL);
  tracker->direction = Unknown;
  tracker->gpio = gpio;
  tracker->pin = pin;
  tracker->pollingPeriod = pollingPeriod;

  res = xTaskCreate(positionTrackerTask, "position tracker",
                    80, (void*)tracker, uxPriority, NULL);
  assert(res == pdTRUE);
}

void setDirection(PositionTracker *tracker, Direction dir) {
	if(xSemaphoreTake( tracker->lock,
										 portMAX_DELAY )){
	tracker->direction=dir;
	xSemaphoreGive(tracker->lock);
								 }
}

s32 getPosition(PositionTracker *tracker) {
	return tracker->position;
}

