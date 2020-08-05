/**
 * Program skeleton for the course "Programming embedded systems"
 *
 * Lab 1: the elevator control system
 */

/**
 * This file defines the safety module, which observe the running
 * elevator system and is able to stop the elevator in critical
 * situations
 */

#ifndef SAFETY_H
#define SAFETY_H

void setupSafety(unsigned portBASE_TYPE uxPriority);

#endif
