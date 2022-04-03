/*
 * timet1.cpp
 *
 *  Created on: 20 mar 2022
 *      Author: rafal
 */

#include "timers.h"
#include "../pgm_data.h"
#include <avr/interrupt.h>




//**********************************************************//
//    AUXILIARY FUNCTIONS                                   //
//**********************************************************//


void null_func(void) {

}

void_void_fptr timer1_compa_func = &null_func;	//safely point a pointer function to null function
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

void set_fast_pwm_timer0(uint8_t pwm){
	/*
	 * Will be removed
	 */
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




//**********************************************************//
//    TIMER PROTOTYPE                                       //
//**********************************************************//


uint32_t Timer::max_range_ms(){
	uint64_t v = pow(2, sizeof(toi_count)*8) * pow(2, sizeof(uint16_t)*8);
	return cycles_to_ms<uint64_t>(v, get_prescaler());
}
uint32_t Timer::max_range_sec(){
	uint64_t v = pow(2, sizeof(toi_count)*8) * pow(2, sizeof(uint16_t)*8);
	return cycles_to_sec(v, get_prescaler());
}

uint32_t Timer::max_range_min(){
	uint64_t v = pow(2, sizeof(toi_count)*8) * pow(2, sizeof(uint16_t)*8);
	return cycles_to_min(v, get_prescaler());
}

uint32_t Timer::max_range_hours(){
	uint64_t v = pow(2, sizeof(toi_count)*8) * pow(2, sizeof(uint16_t)*8);
	return cycles_to_hour(v, get_prescaler());
}

uint32_t Timer::max_range_days(){
	uint64_t v = pow(2, sizeof(toi_count)*8) * pow(2, sizeof(uint16_t)*8);
	return cycles_to_days(v, get_prescaler());
}

Timer::TimeStamp Timer::max_range(){
	uint64_t max_cycles = pow(2, sizeof(toi_count)*8) * pow(2, sizeof(uint16_t)*8);
	uint32_t seconds = cycles_to_sec(max_cycles, get_prescaler());
	uint32_t minutes = seconds/60;
	uint32_t hours = minutes/60;
	uint32_t days = hours/24;
	TimeStamp TS;
	TS.days = days;
	TS.hours = hours - 24*days;
	TS.minutes = minutes - 60*hours;
	TS.seconds = seconds - 60*minutes;

	return TS;
}



//**********************************************************//
//    TIMER1                                                //
//**********************************************************//


ISR(TIMER1_OVF_vect){
	(*Timer1::toi_count_ptr)++;
}


uint16_t* Timer1::toi_count_ptr = 0;

Timer1::Timer1(TccrbClockSelect::pre prescaler):
		TimerT<uint16_t>(prescaler, (timsk_reg&)TIMSK1, (tccra_reg&)TCCR1A, (tccrb_reg&)TCCR1B, (uint16_t&)TCNT1)
{
	tccrb.cs = prescaler;
	timsk.toie = true;
	toi_count_ptr = &toi_count;
}

Timer1::Timer1(const Timer1& source):
		TimerT<uint16_t>(source.prescaler, source.timsk, source.tccra, source.tccrb, source.tcnt)
{
	tccrb.cs = source.prescaler;
	timsk.toie = source.timsk.toie;
	toi_count_ptr = source.toi_count_ptr;
}


TimeCount::TimeCount(Timer* timer):timer(timer){
	tic = timer->get_ms();
}

uint16_t TimeCount::toc(){
	uint16_t now = timer->get_ms();
	uint16_t toc = now - tic;
	tic = timer->get_ms();
	return toc;
}


