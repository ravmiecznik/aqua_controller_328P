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
void set_fast_pwm_timer0(uint8_t pwm);



/*
 * INTERRUPT MASK REGISTER
 * Same struck for TIMSK 1, 2, 3, 4
 */
struct timsk_reg{
	union{
		uint8_t _timsk;
		struct {
			bool toie			:1;
			bool ociea			:1;
			bool ocieb			:1;
			uint8_t reserved	:3;
//			bool icie			:1;
//			uint8_t reserved_2	:2;
		};
	};
};

struct tccra_reg{
	union{
		uint8_t _tccrb;
		struct {
			bool wgm_0			:1;		//waveform generation form
			bool wgm_1			:1;		//waveform generation form
			uint8_t reserved	:2;
			bool com_b_0		:1;		//input capture edge select
			bool com_b_1		:1;		//input capture edge select
			bool com_a_0		:1;		//input capture noise canceler
			bool com_a_1		:1;		//input capture noise canceler
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


namespace WaveFormGenerationMode {
	enum mod {
		normal,
		pwm_phase_correct,
		ctc_ocra, //clear timer on compare
		fast_pwm,
		reserved,
		pwm_phase_correct2,
		reserved2,
		fast_pwm2
	};
}

struct waveform_generation_mode_bit_selection {
	union{
		uint8_t __vaveform_generation_mode_bit_selection;
		struct {
			bool wgm_0_tccra	:1; //refer to tccra
			bool wgm_1_tccra	:1;	//refer to tccra
			bool wgm_2_tccrb	:1;	//refer to tccrb
			bool wgm_3_tccrb	:1;	//refer to tccrb
		};
	};
};


void setup_timer_ociea(tccrb_reg& tccrb, timsk_reg& timsk, TccrbClockSelect::pre clock_select);
void compa_int_delay(uint16_t seconds);

void compa_int_delay_s(uint16_t seconds, tccrb_reg& tccrb, volatile uint16_t* ocr);


class Timer{

};

#endif /* TIMER1_TIMER1_H_ */
