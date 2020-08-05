/**
 * Program skeleton for the course "Programming embedded systems"
 *
 * Lab 1: the elevator control system
 */

/**
 * Functions listening for changes of specified pins
 */

#include "FreeRTOS.h"
#include "task.h"

#include "pin_listener.h"
#include "assert.h"
#include "queue.h"

									
static void pollPin(PinListener *listener,
                    xQueueHandle pinEventQueue) {

																						
	if(GPIO_ReadInputDataBit(listener->gpio,listener->pin) && listener->status < 3)
		{
			listener->status++;
		}
	else if(!(GPIO_ReadInputDataBit(listener->gpio,listener->pin)) && listener->status < 3)
		{
			listener->status = 0;				
		}
		
	if (listener->status == 3)
	{
		xQueueSend( pinEventQueue, &listener->risingEvent, 500 / portTICK_RATE_MS);
		listener->status = 0;
	}
}

static void pollPinsTask(void *params) {
  PinListenerSet listeners = *((PinListenerSet*)params);
  portTickType xLastWakeTime;
  int i;

  xLastWakeTime = xTaskGetTickCount();

  for (;;) {
    for (i = 0; i < listeners.num; ++i)
	  pollPin(listeners.listeners + i, listeners.pinEventQueue);
    
	vTaskDelayUntil(&xLastWakeTime, listeners.pollingPeriod);
  }
}

void setupPinListeners(PinListenerSet *listenerSet) {
  portBASE_TYPE res;

  res = xTaskCreate(pollPinsTask, "pin polling",
                    100, (void*)listenerSet,
					listenerSet->uxPriority, NULL);
  assert(res == pdTRUE);
}
