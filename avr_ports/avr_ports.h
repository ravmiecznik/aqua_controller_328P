/*
 * avr_ports.h
 *
 *  Created on: 16 maj 2018
 *      Author: rafal
 */

#ifndef AVR_PORTS_H_
#define AVR_PORTS_H_

#include <stdint.h>
#include <stdio.h>
#include <avr/io.h>

namespace PIN{};

using namespace PIN;

namespace PIN{
	enum pin_direction{
		in = 0,
		out = 1,
	};

	enum pin_state{
		hi = 1,
		lo = 0,
	};

	enum avr_pin{
		pin0 = 0,
		pin1 = 1,
		pin2 = 2,
		pin3 = 3,
		pin4 = 4,
		pin5 = 5,
		pin6 = 6,
		pin7 = 7,
	};
}

struct AvrPort{
	union{
		volatile uint16_t port;
		struct{
			volatile uint8_t pin_register;		//use to get pin value
			volatile uint8_t port_dir_register;
			volatile uint8_t port_register;		//use to set or read pin settings

		};
		struct{
			volatile bool dirpin0:1;
			volatile bool dirpin1:1;
			volatile bool dirpin2:1;
			volatile bool dirpin3:1;
			volatile bool dirpin4:1;
			volatile bool dirpin5:1;
			volatile bool dirpin6:1;
			volatile bool dirpin7:1;
			volatile bool pin0:1;
			volatile bool pin1:1;
			volatile bool pin2:1;
			volatile bool pin3:1;
			volatile bool pin4:1;
			volatile bool pin5:1;
			volatile bool pin6:1;
			volatile bool pin7:1;
		};
	};
	void set(uint8_t val){
		port_register = val;
	}
	void set_pin(avr_pin pin, pin_state state){
		if(state){
			port_register |= (1<<pin);
		}
		else{
			port_register &= ~(1<<pin);
		}
	}
	void set_pin(avr_pin pin, bool state){
		if(state){
			port_register |= (1<<pin);
		}
		else{
			port_register &= ~(1<<pin);
		}
	}
	void toggle_pin(avr_pin pin){
		port_register ^= (1<<pin);
	}
	uint8_t get(){
		return pin_register;
	}
	bool get_pin(avr_pin pin){
		return pin_register & (1<<pin);
	}
	void operator=(uint8_t val){
		port_register = val;
	}
	uint8_t operator=(AvrPort avrport){
		port_register = avrport.port;
		return port;
	}
	void set_port_direction(pin_direction direction){
		if(direction==in){
			port_dir_register = 0;
		}
		else{
			port_dir_register = 0xff;
		}
	}
	void pull_up(){
		port_register = 0xff;
	}
	void set_pin_direction(avr_pin pin, pin_direction direction){
		if(direction == out){
			port_dir_register |= (1<<pin);
		}
		else{
			port_dir_register &= ~(1<<pin);
		}
	}
};

class AvrPin{
public:
	avr_pin pin;
	AvrPort& port;
	AvrPin(avr_pin pin, AvrPort& port): pin(pin), port(port) {};
	AvrPin(uint8_t pin, AvrPort& port): pin((avr_pin)pin), port(port) {};
	AvrPin(avr_pin pin, AvrPort& port, pin_direction direction): pin(pin), port(port) {
		set_dir(direction);
	};
	void set(pin_state state){
		port.set_pin(pin, state);
	}
	void set(uint8_t state){
		port.set_pin(pin, state);
	}
	void set_dir(pin_direction direction){
		port.set_pin_direction(pin, direction);
	}
	bool get(){
		return port.get_pin(pin);
	}
	void toggle(){
		port.toggle_pin(pin);
	}
	AvrPin& operator=(AvrPin& source){
		set(source.get());
		source.set(get());
		return source;
	}
	void operator=(pin_state state){
		set(state);
	}
	void operator=(uint8_t state){
		set(state);
	}
	operator uint8_t(){
		return get();
	}
	operator bool() {
		return get();
	}
};




#endif /* AVR_PORTS_H_ */
