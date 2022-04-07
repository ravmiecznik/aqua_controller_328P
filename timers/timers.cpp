/*
 * timet1.cpp
 *
 *  Created on: 20 mar 2022
 *      Author: rafal
 */

#include <avr/interrupt.h>
#include "../timers/timers.h"
#include "../pgm_data.h"
#include "../build_setup.h"


extern char* STR_BUFFER;


//**********************************************************//
//    AUXILIARY FUNCTIONS                                   //
//**********************************************************//


void null_func(void) {

}


ISR(TIMER1_COMPA_vect){

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
//    TIMESTAMP                                             //
//**********************************************************//

TimeStamp::TimeStamp(uint32_t raw_time_s){
	/*
	 * Init from raw seconds
	 */
	init_from_seconds(raw_time_s);
}

void TimeStamp::init_from_seconds(uint32_t raw_time_s){
	second = raw_time_s % 60;
	raw_time_s /= 60;
	minute = raw_time_s % 60;
	raw_time_s /= 60;
	hour = raw_time_s % 24;
	raw_time_s /= 24;
	day = raw_time_s;
}

uint32_t TimeStamp::to_seconds(){
	uint32_t total_seconds = (uint32_t)second + (uint32_t)minute*60 + (uint32_t)hour*60*60 + (uint32_t)day*24*60*60;
	return total_seconds;
}

void TimeStamp::normalize(uint32_t max_value){
	init_from_seconds(to_seconds() - max_value);
#ifdef DEBUG
	printf("normalized\n%s",(const char*)*this);
#endif
}

char* TimeStamp::buffer = STR_BUFFER;



TimeStamp::operator const char*(){
#ifdef DEBUG
	uint16_t p = 0;
	p += sprintf_P(TimeStamp::buffer, 	  TIMESTAMP_S);
	p += sprintf_P(&TimeStamp::buffer[p], TIMESTAMP_DAY_S, day);
	p += sprintf_P(&TimeStamp::buffer[p], TIMESTAMP_HOUR_S, hour);
	p += sprintf_P(&TimeStamp::buffer[p], TIMESTAMP_MINUTE_S, minute);
	p += sprintf_P(&TimeStamp::buffer[p], TIMESTAMP_SECOND_S, second);
#endif
	return TimeStamp::buffer;
}


//**********************************************************//
//    TIMER PROTOTYPE                                       //
//**********************************************************//

bool Timer::timer_type = true;

uint32_t Timer::max_range_ms(){
	uint64_t v = pow(2, sizeof(toi_count)*8) * pow(2, sizeof(uint16_t)*8);
	return cycles_to_ms<uint64_t>(v, get_prescaler()); // @suppress("Symbol is not resolved") // @suppress("Invalid arguments")
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


TimeStamp Timer::max_range(){
	uint64_t max_cycles = pow(2, sizeof(toi_count)*8) * pow(2, sizeof(uint16_t)*8);
	uint32_t seconds = cycles_to_sec(max_cycles, get_prescaler());
	uint32_t minutes = seconds/60;
	uint32_t hours = minutes/60;
	uint32_t days = hours/24;
	TimeStamp TS;
	TS.day = days;
	TS.hour = hours - 24*days;
	TS.minute = minutes - 60*hours;
	TS.second = seconds - 60*minutes;

	return TS;
}

TimeStamp Timer::now(){
	return TimeStamp(get_ms()/1000);
}



//**********************************************************//
//    TIMER1                                                //
//**********************************************************//


ISR(TIMER1_OVF_vect){
	(*Timer1::toi_count_ptr)++;
}


volatile uint16_t* Timer1::toi_count_ptr = 0;

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


