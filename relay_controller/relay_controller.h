/*
 * relay_controller.h
 *
 *  Created on: 7 kwi 2022
 *      Author: rafal
 */

#ifndef RELAY_CONTROLLER_RELAY_CONTROLLER_H_
#define RELAY_CONTROLLER_RELAY_CONTROLLER_H_

#include "../avr_ports/avr_ports.h"



class RelayController
{
public:
	enum RelayNormalMode {
		closed,
		open
	};
	RelayNormalMode relay_mode;
	AvrPin& relay_pin;
	RelayController(AvrPin& relay_pin, RelayNormalMode relay_mode): relay_pin(relay_pin), relay_mode(relay_mode)
	{
	}

	void activate(){
		relay_pin.set(0&&relay_mode);
	}
	void deactivate(){
		relay_pin.set(1&&relay_mode);
	}
};



#endif /* RELAY_CONTROLLER_RELAY_CONTROLLER_H_ */
