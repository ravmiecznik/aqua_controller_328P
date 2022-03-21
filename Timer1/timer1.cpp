/*
 * timet1.cpp
 *
 *  Created on: 20 mar 2022
 *      Author: rafal
 */

#include "timer1.h"
#include <avr/interrupt.h>

void null_func(void) {

}

void_void_fptr timer1_compa_func = &null_func;
volatile uint16_t _delay_counter_comp1a = 0;

ISR(TIMER1_COMPA_vect){


	if(_delay_counter_comp1a == 0){
		timer1_compa_func();
		((timsk_reg&)TIMSK1).ociea = false;		//disable ociea
	}
	else {
		_delay_counter_comp1a--;
	}
}

void register_timer1_compa_func(void_void_fptr func){
	timer1_compa_func = func;
}

void setup_timer_ociea(tccrb_reg &tccrb, timsk_reg &timsk, TccrbClockSelect::pre clock_select){
	tccrb.cs = clock_select;
	tccrb.wgm_2 = true; //CTC mode OCR1A
	timsk.ociea = true;
}

uint16_t get_prescaler_value(tccrb_reg &tccrb){
	uint8_t prescaler = tccrb.cs;
	switch (prescaler) {
		case TccrbClockSelect::no_clock:
			return 0;
		case TccrbClockSelect::pre1:
			return 1;
		case TccrbClockSelect::pre8:
			return 8;
		case TccrbClockSelect::pre64:
			return 64;
		case TccrbClockSelect::pre256:
			return 256;
		case TccrbClockSelect::pre1024:
			return 1024;
		default:
			return 0;
	}
}

void compa_int_delay_s(uint16_t seconds, tccrb_reg& tccrb, volatile uint16_t* ocr){
	/*
	 * Delay compa by number of seconds
	 */

		uint16_t prescaler = get_prescaler_value(tccrb);
		uint32_t timer_freq = F_CPU/prescaler;
		uint8_t timer_max_seconds_10 = 0xffff*10/timer_freq;
		uint8_t num_full_cycles = 0;
		if(seconds * 10 > timer_max_seconds_10){
			num_full_cycles = (seconds*10-timer_max_seconds_10)/10;
		}
		_delay_counter_comp1a = num_full_cycles;
		*ocr = seconds*timer_freq;

		setup_timer_ociea((tccrb_reg&)TCCR1B, (timsk_reg&)TIMSK1, TccrbClockSelect::pre1024);
}





