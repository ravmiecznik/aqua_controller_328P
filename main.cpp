/*
 * main.cpp
 *
 *  Created on: 20 mar 2022
 *      Author: rafal
 */


#include <avr/delay.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include "Timer1/timer1.h"


#define LEVEL_SENSOR1_IN PB4
#define RELAY1_OUT PB5
#define GUARD_TIME 5

volatile uint16_t cnt=0;

void disable_lvl_check_interrupt();

void relay_on(){
	PORTB |= _BV(RELAY1_OUT);
}

void relay_off(){
	PORTB &= ~_BV(RELAY1_OUT);
}

bool portb_get_pin(uint8_t pin){
	return PINB & _BV(pin);
}

void reset_guard_cnt(){
	cnt = 0;
}

bool switch_relay_on_lvl_sensor(){
	if(not portb_get_pin(LEVEL_SENSOR1_IN)){
		relay_off();
		return false;
	}
	else{
		reset_guard_cnt();
//		relay_on();
		return true;
	}
}


ISR(PCINT0_vect){
	switch_relay_on_lvl_sensor();
}

//switch_relay_on_lvl_sensor( portb_get_pin(LEVEL_SENSOR1_IN) );

void enable_lvl_check_interrupt(){
	PCICR |= _BV(PCIE0);
	PCMSK0 |= _BV(PCINT4);	//enable interrupt 1 of PCINT
}

void disable_lvl_check_interrupt(){
	PCMSK0 &= ~_BV(PCINT4);	//enable interrupt 1 of PCINT
	compa_int_delay_s(GUARD_TIME, (tccrb_reg&)TCCR1B, &OCR1A);
}

int main(){
//	register_timer1_compa_func(enable_lvl_check_interrupt);

	DDRB &= ~_BV(LEVEL_SENSOR1_IN);		//as input
	DDRB |= _BV(RELAY1_OUT);					//as output
	PORTB |= _BV(LEVEL_SENSOR1_IN);		//pull up

	_delay_ms(1000);
	enable_lvl_check_interrupt();

	sei();

	while(true){
		_delay_ms(1000);
		cnt++;
		if(not (cnt%GUARD_TIME)){
			relay_on();
		}
//		switch_relay_on_lvl_sensor();
	}

}

