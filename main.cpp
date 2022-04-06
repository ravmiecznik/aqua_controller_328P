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





int led_toggle(){
	char c[] = "rafal\n";
	arduino_led.toggle();
	printf(c);
	return 0;
}

int task1(){
	return printf("taks1\n");
}

int task2(){
	return printf("taks2\n");
}


int task3(){
	return printf("taks3\n");
}

int recursive_task(){
	arduino_led = PIN::lo;
	arduino_led.set(Task_Scheduler.put(
			SimpleTask(
					TimeStamp(timer1.get_timestamp_s()),
					recursive_task)
			));
	return printf("recur:\n%s\n", (const char*)timer1.now());
}


int main(){
	setup_stdout_for_printf();
	level_sensor_low = PIN::hi;
	out_compare_0A = PIN::lo;
	enable_pcint_check_interrupt();
	sei();
	printf("now\n%s\n\n", (const char*)timer1.now());


	//**********************************************************//
	//    SCHEDULE SOME TEST TASKS                              //
	//**********************************************************//

	Task_Scheduler.put(
			SimpleTask(
					TimeStamp(timer1.get_timestamp_s() + 3),
					task3)
			);


	Task_Scheduler.put(
			SimpleTask(
					TimeStamp(timer1.get_timestamp_s() + 10),
					task2)
			);

	uint16_t cnt;

	printf("max timer range as TimeStamp\n");
	printf("%s\n", (const char*)timer1.max_range());
	recursive_task();
	_delay_ms(1000);
	while(true){
		_delay_ms(LOOP_PERIOD);
		Task_Scheduler.check();
//		Task_Scheduler.put(
//				SimpleTask(
//						TimeStamp(timer1.get_timestamp_s() + 1),
//						recursive_task)
//				);



	}
}

