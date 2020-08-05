/**
 * Program skeleton for the course "Programming embedded systems"
 *
 * Lab 1: the elevator control system
 */

/**
 * The motor actuator module. This module uses pulse-width modulation
 * (PWM) to smoothly control the output of the motor
 */

#ifndef MOTOR_H
#define MOTOR_H

#include "FreeRTOS.h"
#include "semphr.h"
#include "stm32f10x_tim.h"

#include "position_tracker.h"

typedef struct {

  vs32 targetPosition;				// Position that we currently are
                                    // supposed to go to
  u8 stopped;                       // set when the motor is supposed to
                                    // be stopped immediately
  xSemaphoreHandle lock;


  PositionTracker *currentPosition;	// We need to keep a reference to the
                                    // position tracker, to be able to stop
									// the motor at the right point

  TIM_TypeDef* TIMx;                // Timer and channels used for PWM
  u16 upChannel, downChannel;

  portTickType pollingPeriod;       // Period at which current and target
                                    // position are compared

} Motor;

void setupMotor(Motor *motor,
                PositionTracker *currentPosition,
				TIM_TypeDef* TIMx,
                u16 upChannel, u16 downChannel,
                portTickType pollingPeriod,
				unsigned portBASE_TYPE uxPriority);

void setTargetPosition(Motor *motor, s32 target);

// Stop motor immediately (usually due to safety reasons)
void setMotorStopped(Motor *motor, u8 stopped);

#endif
