/*
 * iRobot.h
 *
 *  Created on: May 20, 2015
 *      Author: CS152B
 */

#ifndef IROBOT_H_
#define IROBOT_H_

#include "xgpio.h"
#include "xparameters.h"
#include "xuartns550_l.h"

#define MILLISECOND 25000

void wait(int time);
void start();
void forward();
void reverse();
void distance(u8 upper, u8 lower);
void turn(u8 upper, u8 lower);
void angle(u8 upper, u8 lower);
void stop();
u8 sensor_request();

#endif /* IROBOT_H_ */
