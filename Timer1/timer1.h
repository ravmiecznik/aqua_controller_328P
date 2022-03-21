/*
 * timer1.h
 *
 *  Created on: 20 mar 2022
 *      Author: rafal
 */

#ifndef TIMER1_TIMER1_H_
#define TIMER1_TIMER1_H_

#include <avr/io.h>

typedef volatile uint8_t& reg8b_ref;
typedef void (*void_void_fptr)(void);
void register_timer1_compa_func(void_void_fptr func);




/*
 * Same struck for TIMSK 1, 2, 3, 4
 */
struct timsk_reg{
	union{
		uint8_t _timsk;
		struct {
			bool toie			:1;
			bool ociea			:1;
			bool ocieb			:1;
			uint8_t reserved_1	:2;
			bool icie			:1;
			uint8_t reserved_2	:2;
		};
	};
};

/*
 * Same struck for TCCRB 1, 2, 3, 4
 */
struct tccrb_reg{
	union{
		uint8_t _tccrb;
		struct {
			uint8_t cs		:3;		//clock select
			bool wgm_2		:1;		//waveform generation form
			bool wgm_3		:1;		//waveform generation form
			bool reserved	:1;
			bool ices		:1;		//input capture edge select
			bool icnc1		:1;		//input capture noise canceler
		};
	};
};

namespace TccrbClockSelect {
	enum pre {
		no_clock,
		pre1,
		pre8,
		pre64,
		pre256,
		pre1024
	};
}


void setup_timer_ociea(tccrb_reg& tccrb, timsk_reg& timsk, TccrbClockSelect::pre clock_select);
void compa_int_delay(uint16_t seconds);

void compa_int_delay_s(uint16_t seconds, tccrb_reg& tccrb, volatile uint16_t* ocr);

#endif /* TIMER1_TIMER1_H_ */
