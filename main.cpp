/*
 * main.cpp
 *
 *  Created on: 20 mar 2022
 *      Author: rafal
 */


#include <util/delay.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include "Timer1/timer1.h"

#define LOOP_PERIOD			500		//cant exceed 1000
#define RELAY_SUMP_PUMP 	PB4
#define RELAY_FEED_PUMP 	PB3
#define LEVEL_SENSOR_LOW	PB2
#define LEVEL_SENSOR_HI		PB1
#define ARDUINO_LED			PB5
#define BTN					PB0
#define OC0A				PD6
#define EXT_DIODE			PD7


#define GUARD_TIME_WCM		0		//water change mode
#define WATER_CHANGE_MODE_DURATION	(60*60)
#define GUARD_TIME_NWCM		(5*60)	//not water change mode


#if LOOP_PERIOD > 1000
	#error "LOOP_PERIOD exceed 1000"
#endif

volatile uint16_t GUARD_COUNTER=0;
volatile bool WATER_CHANGE_MODE = false;
volatile uint16_t GUARD_TIME = GUARD_TIME_NWCM;

bool switch_relay_sump_pump_on_low_lvl_sensor();

namespace RELAY_STATE {
	enum state {
		on,
		off
	};
}

void enable_pcint_check_interrupt(){
	PCICR |= _BV(PCIE0);
	PCMSK0 |= _BV(PCINT4);	//enable interrupt 1 of PCINT
}


//void disable_pcint_check_interrupt(){
//	PCICR &= ~_BV(PCIE0);
//}

ISR(PCINT0_vect){
	switch_relay_sump_pump_on_low_lvl_sensor();
}


bool portb_get_pin(uint8_t pin){
	uint8_t samples_num = 6;
	uint8_t pin_value = 0;
	for(uint8_t i=0; i<samples_num; i++){
		pin_value += PINB & _BV(pin);
		_delay_us(1);
	}
	return pin_value > samples_num/2;
}

bool relay_control(RELAY_STATE::state state, uint8_t relay_num){
	bool pin_value = portb_get_pin(relay_num);
	if(state == RELAY_STATE::on and pin_value){
		PORTB &= ~_BV(relay_num);
		return true;
	}
	else if(state == RELAY_STATE::off and not pin_value){
		PORTB |= _BV(relay_num);
		return false;
	}
	return portb_get_pin(relay_num);
}




bool switch_relay_sump_pump_on_low_lvl_sensor(){
	/*
	 * used in ISR
	 */

	if(not portb_get_pin(LEVEL_SENSOR_LOW)){		//enable relay if level alarm
		relay_control(RELAY_STATE::on, RELAY_SUMP_PUMP);
		GUARD_COUNTER = 0;
		return true;
	}
	else{
		if(GUARD_COUNTER>=GUARD_TIME){
			return relay_control(RELAY_STATE::off, RELAY_SUMP_PUMP);	//disable relay if not level alarm (enable pump)
		}
	}
	return false;
}


bool enable_sump_pump_after_guard_time(volatile uint16_t& guard_count){
	if(guard_count < GUARD_TIME){
		guard_count++;
	}
	else if(guard_count>=GUARD_TIME){
		return switch_relay_sump_pump_on_low_lvl_sensor();
	}
	return false;
}

bool monitor_water_change_mode_long_press(){
	/*
	 * Checks if water change mode button is long pressed
	 */
	static uint8_t water_change_mode_trigger_count = 0;
	if(not (portb_get_pin(BTN))){
		water_change_mode_trigger_count++;
	}
	else{
		water_change_mode_trigger_count = 0;

	}
	return water_change_mode_trigger_count > 2;
}

void update_guard_time_on_wCM(){
	if(WATER_CHANGE_MODE){
		GUARD_TIME = GUARD_TIME_WCM;
	}
	else{
		GUARD_TIME = GUARD_TIME_NWCM;
	}
	GUARD_COUNTER = GUARD_TIME;	//reset GUARD_COUNTER
}

bool monitor_water_change_mode_button(){
	bool state_changed = false;
	if(monitor_water_change_mode_long_press()){
//	if(true){
		WATER_CHANGE_MODE ^= true;
//		WATER_CHANGE_MODE = true;
		state_changed = true;
		update_guard_time_on_wCM();
	}

	if(WATER_CHANGE_MODE){
		PORTD |= _BV(EXT_DIODE);
	}
	else{
		PORTD &= ~_BV(EXT_DIODE);
	}
	return state_changed;
}

void handle_water_change_mode_state(){
	static uint16_t water_change_mode_counter = 0;
	if(monitor_water_change_mode_button()){
		water_change_mode_counter = 0;

	}
	if(WATER_CHANGE_MODE and water_change_mode_counter < WATER_CHANGE_MODE_DURATION){
		water_change_mode_counter++;
	}
	else if(WATER_CHANGE_MODE and water_change_mode_counter >= WATER_CHANGE_MODE_DURATION){
		WATER_CHANGE_MODE = false;
		update_guard_time_on_wCM();
		water_change_mode_counter = 0;
	}
	else{
		water_change_mode_counter = 0;
	}


}

void blink_short_and_sleep(){
	uint8_t duration = 50;
	if(not WATER_CHANGE_MODE){
		PORTD |= _BV(EXT_DIODE);
		_delay_ms(duration);
		PORTD &= ~_BV(EXT_DIODE);
		_delay_ms(LOOP_PERIOD - duration);
	}
	else{
		_delay_ms(LOOP_PERIOD);
	}
}

void blink_long_and_sleep(){
	uint16_t duration = 450;
	if(not WATER_CHANGE_MODE){
		PORTD |= _BV(EXT_DIODE);
		_delay_ms(duration);
		PORTD &= ~_BV(EXT_DIODE);
		_delay_ms(LOOP_PERIOD - duration);
	}
	else{
		_delay_ms(LOOP_PERIOD);
	}
}

uint32_t abs(uint32_t a, uint32_t b){
	//condition ? result_if_true : result_if_false
	return a > b ? a - b : b -a;
}

uint32_t loop_seconds(uint32_t loop_count){
	return loop_count/(1000/LOOP_PERIOD);
}

int main(){
	DDRB &= ~_BV(LEVEL_SENSOR_LOW) & ~_BV(LEVEL_SENSOR_HI) & ~_BV(BTN);		//as input
	PORTB |= _BV(LEVEL_SENSOR_LOW) | _BV(LEVEL_SENSOR_HI) | _BV(BTN);		//pull up
	DDRB |= _BV(RELAY_SUMP_PUMP)
			| _BV(RELAY_FEED_PUMP) | _BV(ARDUINO_LED);		//as output

	DDRD |= _BV(EXT_DIODE) | _BV(OC0A);

	enable_pcint_check_interrupt();
	relay_control(RELAY_STATE::off, RELAY_SUMP_PUMP);
	relay_control(RELAY_STATE::off, RELAY_FEED_PUMP);
	_delay_ms(500);
	GUARD_COUNTER=GUARD_TIME;
	switch_relay_sump_pump_on_low_lvl_sensor();
	sei();
	uint32_t loop_count=0;
	bool pump_was_started = false;
	uint32_t sump_pump_start_tstamp = 0;
	uint32_t feed_pump_start_tstamp = 0;


//	PORTD |= _BV(OC0A);
	PORTD &= ~_BV(OC0A);
	uint8_t fan_speed = 100;
	set_fast_pwm_timer0(fan_speed);
	while(true){
		if(not( loop_count % (500/LOOP_PERIOD))){
			set_fast_pwm_timer0(fan_speed);
			fan_speed = fan_speed >= 100 ? 2:fan_speed+5;
		}
		if(not( loop_count % (1000/LOOP_PERIOD))){				//every second check
			pump_was_started = not enable_sump_pump_after_guard_time(GUARD_COUNTER);

			if(pump_was_started and WATER_CHANGE_MODE){
				sump_pump_start_tstamp = loop_seconds(loop_count);
			}
			else if (not WATER_CHANGE_MODE){
				sump_pump_start_tstamp = 0;
			}

			handle_water_change_mode_state();
			if(GUARD_COUNTER>=GUARD_TIME){
				blink_short_and_sleep();
			}
			else{
				blink_long_and_sleep();
			}
			PORTB ^= _BV(ARDUINO_LED);
		}
		else{
			_delay_ms(LOOP_PERIOD);
		}




		if(WATER_CHANGE_MODE){
			if(portb_get_pin(LEVEL_SENSOR_HI)){
				relay_control(RELAY_STATE::on, RELAY_FEED_PUMP);
				feed_pump_start_tstamp = loop_seconds(loop_count);
			}

			else{
				if(( abs( sump_pump_start_tstamp, feed_pump_start_tstamp) < 5)){
					if( sump_pump_start_tstamp and abs( loop_seconds(loop_count), sump_pump_start_tstamp) > 30){
						relay_control(RELAY_STATE::off, RELAY_FEED_PUMP);
						feed_pump_start_tstamp = 0;
					}
				}
				else{
					relay_control(RELAY_STATE::off, RELAY_FEED_PUMP);
					feed_pump_start_tstamp = 0;
				}
			}
		}
		else{
			relay_control(RELAY_STATE::off, RELAY_FEED_PUMP);
			feed_pump_start_tstamp = 0;
		}
		loop_count++;


	}
}

