/*
 * timet1.cpp
 *
 *  Created on: 20 mar 2022
 *      Author: rafal
 */

#include "timers.h"

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

//ISR(TIMER0_COMPA_vect){
//	PORTB ^= _BV(PB5);
//}

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

void set_fast_pwm_timer0(uint8_t pwm){
	((tccra_reg&)TCCR0A).com_a_1 = 0;
	((tccra_reg&)TCCR0A).wgm_0 = 0;
	((tccra_reg&)TCCR0A).wgm_1 = 0;
	if(pwm>0){
		((tccra_reg&)TCCR0A).com_a_1 = 1;
		((tccra_reg&)TCCR0A).wgm_0 = 1;
		((tccra_reg&)TCCR0A).wgm_1 = 1;
		((tccrb_reg&)TCCR0B).cs = TccrbClockSelect::pre1;

		pwm = pwm>100 ? 100:pwm;
	//	pwm = 100 - pwm;
	//
		uint16_t pwm_raw = 0xff*pwm/100;
		OCR0A = pwm_raw;
	}
}

/////////////////////////////////////////////////////////////////////////////////



ISR(TIMER1_OVF_vect){
	(*Timer1::toi_count_ptr)++;
}


uint16_t* Timer1::toi_count_ptr = 0;

Timer1::Timer1(TccrbClockSelect::pre prescaler):
		Timer(prescaler, (timsk_reg&)TIMSK1, (tccra_reg&)TCCR1A, (tccrb_reg&)TCCR1B, (uint16_t&)TCNT1)
{
	tccrb.cs = prescaler;
	timsk.toie = true;
	toi_count_ptr = &toi_count;
}


TimeStamp::TimeStamp(Timer* timer):timer(timer){
	tic = timer->get_ms();
}

uint16_t TimeStamp::toc(){
	uint16_t now = timer->get_ms();
	uint16_t toc = now - tic;
	tic = timer->get_ms();
	return toc;
}


