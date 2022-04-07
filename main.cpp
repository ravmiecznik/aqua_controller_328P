/*
 * main.cpp
 *
 *  Created on: 20 mar 2022
 *      Author: rafal
 */


#include <util/delay.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include "atm328_usart/usart.h"
#include "setup.h"
#include "pgm_data.h"
#include "timers/timers.h"
#include "relay_controller/relay_controller.h"


#if LOOP_PERIOD > 1000
	#error "LOOP_PERIOD exceed 1000"
#endif


volatile uint16_t GUARD_COUNTER=0;
volatile bool WATER_CHANGE_MODE = false;
volatile uint16_t GUARD_TIME = GUARD_TIME_NWCM;



namespace RELAY_STATE {
	enum state {
		on,
		off
	};
}

void enable_pcint_check_interrupt(){
	PCICR |= _BV(PCIE0);
	PCMSK0 |= _BV(PCINT2);	//enable interrupt 1 of PCINT
}


ISR(PCINT0_vect){
	arduino_led.toggle();
}



bool relay_control(RELAY_STATE::state state, AvrPin relay_pin){
	if(state == RELAY_STATE::on and relay_pin){
		relay_pin = PIN::lo;
		return true;
	}
	else if(state == RELAY_STATE::off and not relay_pin){
		relay_pin = PIN::hi;
		return false;
	}
	return relay_pin;
}




uint32_t absolute(uint32_t a, uint32_t b){
	return a > b ? a - b : b -a;
}


uint32_t loop_seconds(uint32_t loop_count){
	return loop_count/(1000/LOOP_PERIOD);
}



int relay_off_task();

int relay_on_task(){
	RELAY1.activate();
	bool set = Task_Scheduler.put(
			SimpleTask(
					TimeStamp(timer1.get_timestamp_s() + 4),
					relay_off_task)
			);
	printf("set: %d ", set);
	printf("now\n%s\n\n", (const char*)timer1.now());
	return printf("%s\n", __FUNCTION__);
}

int relay_off_task(){
	RELAY1.deactivate();
	bool set = Task_Scheduler.put(
			SimpleTask(
					TimeStamp(timer1.get_timestamp_s() + 4),
					relay_on_task)
			);
	printf("set: %d ", set);
	return printf("%s\n", __FUNCTION__);

}


int main(){
	setup_stdout_for_printf();
	level_sensor_low = PIN::hi;
	relay1_ctrl = PIN::lo;
	enable_pcint_check_interrupt();
	sei();
	printf("now\n%s\n\n", (const char*)timer1.now());


	//**********************************************************//
	//    SCHEDULE SOME TEST TASKS                              //
	//**********************************************************//



	uint16_t cnt;

	printf("max timer range as TimeStamp\n");
	printf("%s\n", (const char*)timer1.max_range());
//	relay_on_task();
//	relay_off_task();

	Task_Scheduler.put(
			SimpleTask(
					TimeStamp(timer1.get_timestamp_s() + 20),
					relay_on_task)
			);
	_delay_ms(1000);
	while(true){
		_delay_ms(LOOP_PERIOD);
		Task_Scheduler.check();
//		if(not (cnt++%3)){
//			RELAY1.activate();
//		}
//		else
//		{
//			RELAY1.deactivate();
//		}
//		Task_Scheduler.put(
//				SimpleTask(
//						TimeStamp(timer1.get_timestamp_s() + 1),
//						recursive_task)
//				);



	}
}

