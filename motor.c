/**
 * Program skeleton for the course "Programming embedded systems"
 *
 * Lab 1: the elevator control system
 */

/**
 * The motor actuator module. This module uses pulse-width modulation
 * (PWM) to smoothly control the output of the motor
 */

#include "FreeRTOS.h"
#include "task.h"
#include "stm32f10x_lib.h"
#include "stm32f10x_map.h"
#include <stdio.h>

#include "position_tracker.h"
#include "motor.h"

#include "assert.h"

// Constant acceleration
#define MAX_DUTY          10000	  // The motor output is specified as
                                  // an integer between 0 and MAX_DUTY
#define ACCEL_TIME        500     // time to go from zero to full speed

// De-acceleration: speed is changed proportional to the distance
//                  from the target
#define MAX_SPEED         50      // maximum speed: 50cm/s
#define MIN_SPEED         3       // minimum speed: 3cm/s
#define DUTY_FACTOR       200     // 1cm/s corresponds to duty 200

static s32 maxDutyAtDistance(s32 distance) {
  if (distance > MAX_SPEED)
    distance = MAX_SPEED;
  if (distance < MIN_SPEED)
    distance = MIN_SPEED;
  return distance * DUTY_FACTOR;
}

static s32 min(s32 a, s32 b) {
  if (a < b)
    return a;
  else
    return b;
}

static setCompare(TIM_TypeDef* TIMx, u16 channel, u16 value) {
  switch (channel) {
    case TIM_Channel_1:
	  TIM_SetCompare1(TIMx, value);
	  break;
    case TIM_Channel_2:
	  TIM_SetCompare2(TIMx, value);
	  break;
    case TIM_Channel_3:
	  TIM_SetCompare3(TIMx, value);
	  break;
    case TIM_Channel_4:
	  TIM_SetCompare4(TIMx, value);
	  break;
	default:
	  assert(0);
	  break;
  }
}

static void setDuty(Motor *motor, s32 duty) {
  if (duty < 0) {
    setCompare(motor->TIMx, motor->upChannel, 0);
    setCompare(motor->TIMx, motor->downChannel, (u16)-duty);
  } else {
    setCompare(motor->TIMx, motor->downChannel, 0);
    setCompare(motor->TIMx, motor->upChannel, (u16)duty);
  }
}

static void motorTask(void *params) {
  Motor *motor = (Motor*)params;
  portTickType xLastWakeTime;
  s32 pos, targetPos;
  u8 stopped;
  s32 currentDuty = 0;
  u16 maxDutyChange = MAX_DUTY * motor->pollingPeriod / ACCEL_TIME;

  xLastWakeTime = xTaskGetTickCount();

  for (;;) {
    xSemaphoreTake(motor->lock, portMAX_DELAY);
	pos = getPosition(motor->currentPosition);
    targetPos = motor->targetPosition;
    stopped = motor->stopped;
	xSemaphoreGive(motor->lock);
	
	if (stopped) {
	  // immediately stop the motor

	  if (currentDuty >= maxDutyChange)
	    currentDuty -= maxDutyChange;
	  else if (currentDuty <= -maxDutyChange)
	    currentDuty += maxDutyChange;
      else
	    currentDuty = 0;
      setDuty(motor, currentDuty);

	} else if (targetPos > pos) {
	  // We have to increase the position to reach the target

      setDirection(motor->currentPosition, Up);

	  currentDuty = min(maxDutyAtDistance(targetPos - pos),
	                    currentDuty + maxDutyChange);
      setDuty(motor, currentDuty);
	} else if (targetPos < pos) {
	  // We have to decrease the position to reach the target

      setDirection(motor->currentPosition, Down);

	  currentDuty = -min(maxDutyAtDistance(pos - targetPos),
	                     -currentDuty + maxDutyChange);
      setDuty(motor, currentDuty);
	} else {
      // We have reached the target

      setDirection(motor->currentPosition, Unknown);

      currentDuty = 0;
      setDuty(motor, currentDuty);
	}

	vTaskDelayUntil(&xLastWakeTime, motor->pollingPeriod);
  }
}

void setupMotor(Motor *motor,
                PositionTracker *currentPosition,
				TIM_TypeDef* TIMx,
                u16 upChannel, u16 downChannel,
                portTickType pollingPeriod,
				unsigned portBASE_TYPE uxPriority) {
  TIM_OCInitTypeDef TIM_OCInitStruct;

  portBASE_TYPE res;

  motor->currentPosition = currentPosition;
  motor->targetPosition = 0;
  motor->stopped = 0;
  motor->lock = xSemaphoreCreateMutex();
  assert(motor->lock != NULL);
  motor->TIMx = TIMx;
  motor->upChannel = upChannel;
  motor->downChannel = downChannel;
  motor->pollingPeriod = pollingPeriod;

  // Setup two timer channels for PWM output
  TIM_OCStructInit(&TIM_OCInitStruct);
  TIM_OCInitStruct.TIM_OCMode = TIM_OCMode_PWM1;
  TIM_OCInitStruct.TIM_OCPolarity = 1;
  TIM_OCInitStruct.TIM_Pulse = 0;

  TIM_OCInitStruct.TIM_Channel = upChannel;
  TIM_OCInit(TIMx, &TIM_OCInitStruct);

  TIM_OCInitStruct.TIM_Channel = downChannel;
  TIM_OCInit(TIMx, &TIM_OCInitStruct);

  res = xTaskCreate(motorTask, "motor", 80,
                   (void*)motor, uxPriority, NULL);
  assert(res == pdTRUE);
}

void setTargetPosition(Motor *motor, s32 target) {
  xSemaphoreTake(motor->lock, portMAX_DELAY);
  motor->targetPosition = target;
  xSemaphoreGive(motor->lock);
}

void setMotorStopped(Motor *motor, u8 stopped) {
  xSemaphoreTake(motor->lock, portMAX_DELAY);
  motor->stopped = stopped;
  xSemaphoreGive(motor->lock);
}
