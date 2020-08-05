/**
 * Program skeleton for the course "Programming embedded systems"
 *
 * Lab 1: the elevator control system
 */

/**
 * Functions listening for changes of specified pins
 */

#ifndef PIN_LISTENER_H
#define PIN_LISTENER_H

#include "FreeRTOS.h"
#include "queue.h"
#include "stm32f10x_gpio.h"

#include "global.h"

typedef struct {
  GPIO_TypeDef * gpio;		  // Pin to listener at, e.g., GPIOC,
  u16 pin;               	  // GPIO_Pin_4
  PinEvent risingEvent;       // Event raised when pin changes from 0 to 1
  PinEvent fallingEvent;      // Event raised when pin changes from 1 to 0

  u8 status;                  // internal state
} PinListener;

typedef struct {
  PinListener *listeners;	          // Array of PinListeners
  int num;					          // size of the array
  portTickType pollingPeriod;         // how often the status of pins is polled
  unsigned portBASE_TYPE uxPriority;  // Priority of the polling task
  xQueueHandle pinEventQueue;         // queue where events are sent to
} PinListenerSet;

/**
 * Set up an array of PinListeners. This creates a
 * (single) task that is regularly polling the status of
 * the specified pins
 */
void setupPinListeners(PinListenerSet *listenerSet);

#endif
