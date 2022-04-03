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
#include "avr_ports/avr_ports.h"
#include "atm328_usart/usart.h"
#include "setup.h"
#include "pgm_data.h"
#include "Timers/timers.h"



#if LOOP_PERIOD > 1000
	#error "LOOP_PERIOD exceed 1000"
#endif

AvrPin arduino_led		(PIN::pin5, (AvrPort&)PINB, PIN::out);	//Arduino D13
AvrPin relay_sump_pump	(PIN::pin4, (AvrPort&)PINB, PIN::out);	//Arduino D12
AvrPin relay_feed_pump	(PIN::pin3, (AvrPort&)PINB, PIN::out);	//Arduino D11
AvrPin level_sensor_low	(PIN::pin2, (AvrPort&)PINB, PIN::in);	//Arduino D10
AvrPin level_sensor_hi	(PIN::pin1, (AvrPort&)PINB, PIN::in);	//Arduino D09
AvrPin button			(PIN::pin0, (AvrPort&)PINB, PIN::in);	//Arduino D08
AvrPin ext_diode		(PIN::pin7, (AvrPort&)PIND, PIN::out);	//Arduino D07
AvrPin out_compare_0A	(PIN::pin6, (AvrPort&)PIND, PIN::out);	//Arduino D06

Usart usart(usart0, 9600, 20, 20);

volatile uint16_t GUARD_COUNTER=0;
volatile bool WATER_CHANGE_MODE = false;
volatile uint16_t GUARD_TIME = GUARD_TIME_NWCM;


bool switch_relay_sump_pump_on_low_lvl_sensor(AvrPin level_sensor_low, AvrPin relay_sump_pump);

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
	switch_relay_sump_pump_on_low_lvl_sensor(level_sensor_low, relay_sump_pump);
	arduino_led.toggle();
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




bool switch_relay_sump_pump_on_low_lvl_sensor(AvrPin level_sensor_low, AvrPin relay_sump_pump){
	/*
	 * used in ISR
	 */

	if(not level_sensor_low){		//enable relay if level alarm
		relay_control(RELAY_STATE::on, relay_sump_pump);
		GUARD_COUNTER = 0;
		return true;
	}
	else{
		if(GUARD_COUNTER>=GUARD_TIME){
			return relay_control(RELAY_STATE::off, relay_sump_pump);	//disable relay if not level alarm (enable pump)
		}
	}
	return false;
}


bool enable_sump_pump_after_guard_time(volatile uint16_t& guard_count){
	if(guard_count < GUARD_TIME){
		guard_count++;
	}
	else if(guard_count>=GUARD_TIME){
		return switch_relay_sump_pump_on_low_lvl_sensor(level_sensor_low, relay_sump_pump);
	}
	return false;
}

bool monitor_water_change_mode_long_press(AvrPin button){
	/*
	 * Checks if water change mode button is long pressed
	 */
	static uint8_t water_change_mode_trigger_count = 0;
	if(not button){
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

bool monitor_water_change_mode_button(AvrPin ext_diode, AvrPin button){
	bool state_changed = false;
	if(monitor_water_change_mode_long_press(button)){
		WATER_CHANGE_MODE ^= true;
		state_changed = true;
		update_guard_time_on_wCM();
	}

	if(WATER_CHANGE_MODE){
		ext_diode = PIN::hi;
	}
	else{
		ext_diode = PIN::lo;
	}
	return state_changed;
}

void handle_water_change_mode_state(AvrPin ext_diode, AvrPin button){
	static uint16_t water_change_mode_counter = 0;
	if(monitor_water_change_mode_button(ext_diode, button)){
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

void blink_short_and_sleep(AvrPin ext_diode){
	uint8_t duration = 50;
	if(not WATER_CHANGE_MODE){
		ext_diode = PIN::hi;
		_delay_ms(duration);
		ext_diode = PIN::lo;
		_delay_ms(LOOP_PERIOD - duration);
	}
	else{
		_delay_ms(LOOP_PERIOD);
	}
}

void blink_long_and_sleep(AvrPin ext_diode){
	uint16_t duration = 450;
	if(not WATER_CHANGE_MODE){
		ext_diode = PIN::hi;
		_delay_ms(duration);
		ext_diode = PIN::lo;
		_delay_ms(LOOP_PERIOD - duration);
	}
	else{
		_delay_ms(LOOP_PERIOD);
	}
}


uint32_t absolute(uint32_t a, uint32_t b){
	return a > b ? a - b : b -a;
}


uint32_t loop_seconds(uint32_t loop_count){
	return loop_count/(1000/LOOP_PERIOD);
}


/****************************************************************/
/*
 * Configuration of standard output for functions printf, printf_p and so on
 */
static int put(char c, FILE *stream){
	usart.Putchar(c);
	return 0;
}
static FILE uartout = {0};
void setup_stdout_for_printf(){
	fdev_setup_stream(&uartout, put, NULL, _FDEV_SETUP_WRITE);
	stdout = &uartout;
	stderr = &uartout;
}
/****************************************************************/


int printfp(const char* format, ...){
	/*
	 * Uses formated print to attach pgmspace string to message
	 */
	if(strlen_P(format) <70){
		char buff[50];
		va_list arg;
		int done;
		va_start (arg, format);
		done = vsprintf_P(buff, format, arg);
		va_end (arg);
		printf(buff);
		return done;
	}
	return 0;
}


int main(){

	setup_stdout_for_printf();
	level_sensor_low = PIN::hi;
	out_compare_0A = PIN::lo;
	enable_pcint_check_interrupt();
	relay_control(RELAY_STATE::off, relay_sump_pump);
	relay_control(RELAY_STATE::off, relay_feed_pump);
	Timer1 timer1(TccrbClockSelect::pre1);
	_delay_ms(500);
	GUARD_COUNTER=GUARD_TIME;
	switch_relay_sump_pump_on_low_lvl_sensor(level_sensor_low, relay_sump_pump);
	sei();
	uint32_t loop_count=0;
	bool pump_was_started = false;
	uint32_t sump_pump_start_tstamp = 0;
	uint32_t feed_pump_start_tstamp = 0;

	uint8_t fan_speed = 100;
	set_fast_pwm_timer0(fan_speed);
	timer1.tic();
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

			handle_water_change_mode_state(ext_diode, button);
			if(GUARD_COUNTER>=GUARD_TIME){
				blink_short_and_sleep(ext_diode);
			}
			else{
				blink_long_and_sleep(ext_diode);
			}
			arduino_led.toggle();
			Timer::TimeStamp TS = timer1.max_range();
			printfp(timer_range_fmt, TS.days, TS.hours, TS.minutes, TS.seconds);
//			printf("%u\n", timer1.max_range_min());
//			printf("%u\n", tref.toc());
		}
		else{
			_delay_ms(LOOP_PERIOD);

		}




		if(WATER_CHANGE_MODE){
			if(level_sensor_hi){
				relay_control(RELAY_STATE::on, relay_feed_pump);
				feed_pump_start_tstamp = loop_seconds(loop_count);
			}

			else{
				if(( absolute( sump_pump_start_tstamp, feed_pump_start_tstamp) < 5)){
					if( sump_pump_start_tstamp and absolute( loop_seconds(loop_count), sump_pump_start_tstamp) > 30){
						relay_control(RELAY_STATE::off, relay_feed_pump);
						feed_pump_start_tstamp = 0;
					}
				}
				else{
					relay_control(RELAY_STATE::off, relay_feed_pump);
					feed_pump_start_tstamp = 0;
				}
			}
		}
		else{
			relay_control(RELAY_STATE::off, relay_feed_pump);
			feed_pump_start_tstamp = 0;
		}
		loop_count++;


	}
}

