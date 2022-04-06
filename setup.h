/*
 * setup.h
 *
 *  Created on: 2 kwi 2022
 *      Author: rafal
 */

#ifndef SETUP_H_
#define SETUP_H_

#include "avr_ports/avr_ports.h"
#include "atm328_usart/usart.h"
#include "task_scheduler/task_scheduler.h"
#include "timers/timers.h"

#define LOOP_PERIOD			500		//cant exceed 1000
#define GUARD_TIME_WCM		0		//water change mode
#define WATER_CHANGE_MODE_DURATION	(60*60)
#define GUARD_TIME_NWCM		(5*60)	//not water change mode

void setup_stdout_for_printf();

//globals
extern AvrPin arduino_led;
extern AvrPin relay_sump_pump;
extern AvrPin relay_feed_pump;
extern AvrPin level_sensor_low;
extern AvrPin level_sensor_hi;
extern AvrPin button;
extern AvrPin ext_diode;
extern AvrPin out_compare_0A;

extern Usart usart;
extern Timer1 timer1;

typedef int (*fptr_int_void)(void);
typedef Task<TimeStamp, fptr_int_void> SimpleTask;
extern Tasks<SimpleTask , Timer1> Task_Scheduler;

#endif /* SETUP_H_ */
