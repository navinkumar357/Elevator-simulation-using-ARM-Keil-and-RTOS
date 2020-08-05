/**
 * Program skeleton for the course "Programming embedded systems"
 *
 * Lab 1: the elevator control system
 */

/**
 * Main file of the system; module setup and initialisation takes
 * place here
 */

#include <stdio.h>

#include "FreeRTOS.h"
#include "task.h"
#include "setup.h"

#include "stm32f10x_conf.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_tim.h"
#include "stm32f10x_lib.h"
#include "stm32f10x_map.h"

#include "global.h"
#include "pin_listener.h"
#include "position_tracker.h"
#include "motor.h"
#include "planner.h"
#include "safety.h"

#include "assert.h"

/*-----------------------------------------------------------*/
/* Input module */

xQueueHandle pinEventQueue;

/**
 * This array describes which pins are connected to which
 * events
 */
PinListener pinListeners[] =
   { { GPIOC, GPIO_Pin_0, TO_FLOOR_1,       UNASSIGNED, 0 },
     { GPIOC, GPIO_Pin_1, TO_FLOOR_2,       UNASSIGNED, 0 },
     { GPIOC, GPIO_Pin_2, TO_FLOOR_3,       UNASSIGNED, 0 },
     { GPIOC, GPIO_Pin_3, STOP_PRESSED,     STOP_RELEASED },
     { GPIOC, GPIO_Pin_7, ARRIVED_AT_FLOOR, LEFT_FLOOR },
     { GPIOC, GPIO_Pin_8, DOORS_CLOSED,     DOORS_OPENING } };

PinListenerSet listenerSet = {
   pinListeners,            // Array connecting pins with events
   6,						// size of the array
   10 / portTICK_RATE_MS,	// Rate at which the status of pins is checked
   1,                       // Priority
   NULL };					// Event queue (set in "setupInputModule")


/**
 * Object responsible for keeping track of the car position
 */
PositionTracker carPositionTracker;

/**
 * Create all objects and tasks belonging to the input module
 */
void setupInputModule() {
  GPIO_InitTypeDef GPIO_InitStructure;

  pinEventQueue = xQueueCreate(32, sizeof(PinEvent));
  assert(pinEventQueue != NULL);
  listenerSet.pinEventQueue = pinEventQueue;

  // Initialise pins 0, 1, 2, 3, 7, 8, 9 of GPIOC for input
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | 
                                GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init( GPIOC, &GPIO_InitStructure );

  // Setup tasks listening at the pins
  setupPinListeners(&listenerSet);

  // Setup task keeping track of the car position
  setupPositionTracker(&carPositionTracker,
                       GPIOC, GPIO_Pin_9,
					   3 / portTICK_RATE_MS, 4);
}

/*-----------------------------------------------------------*/
/* Actuator module */

/**
 * Object responsible for driving the motor
 */
Motor carMotor;

/**
 * Create all objects and tasks belonging to the actuator module
 */
void setupActuatorModule() {
  TIM_TimeBaseInitTypeDef timInit;

  /* Setup timer TIM3 for pulse-width modulation:
     100kHz, periodically counting from 0 to 9999
     This will generate a 10Hz pulse; far too slow for an
     actual motor, but fast enough to see something blinking
    */
  RCC_APB1PeriphClockCmd( RCC_APB1Periph_TIM3, ENABLE );

  TIM_DeInit( TIM3 );
  TIM_TimeBaseStructInit( &timInit );

  timInit.TIM_Period = (unsigned portSHORT)0x270F; // Auto-reload period
  timInit.TIM_Prescaler = 719;                     // Prescaler; resulting frequency
                                                   //  is 72MHz / 720 = 100kHz
  timInit.TIM_ClockDivision = TIM_CKD_DIV1;        // Clock division 1
  timInit.TIM_CounterMode = TIM_CounterMode_Up;    // Counting upwards
    
  TIM_TimeBaseInit( TIM3, &timInit );
  TIM_ARRPreloadConfig( TIM3, ENABLE );

  TIM_Cmd(TIM3, ENABLE);

  setupMotor(&carMotor, &carPositionTracker,
             TIM3, TIM_Channel_1, TIM_Channel_2,
			 30 / portTICK_RATE_MS, 2);
}

/*-----------------------------------------------------------*/
/* Functions defined in global.h */

s32 getCarPosition() {
  return getPosition(&carPositionTracker);
}

void setCarTargetPosition(s32 target) {
  setTargetPosition(&carMotor, target);
}

void setCarMotorStopped(u8 stopped) {
  setMotorStopped(&carMotor, stopped);
}

/*-----------------------------------------------------------*/

/*
 * Entry point of program execution
 */
int main( void )
{
  prvSetupHardware();

  setupInputModule();
  setupActuatorModule();
  setupPlanner(1);
  setupSafety(3);

  printf("Setup completed\n");  // this is redirected to USART 1

  vTaskStartScheduler();
  assert(0);
  return 0;                 // not reachable
}
/*-----------------------------------------------------------*/

void assert_failed(u8* file, u32 line) {
  printf("ASSERTION FAILURE: %s:%d\n", file, line);
}


