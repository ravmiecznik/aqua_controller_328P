/*
 * timer1.h
 *
 *  Created on: 20 mar 2022
 *      Author: rafal
 */

#ifndef TIMERS_TIMERS_H_
#define TIMERS_TIMERS_H_

#include <avr/io.h>
#include <avr/interrupt.h>
#include <math.h>

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


//uint32_t cycles_to_ms(uint32_t cycles, uint16_t _prescaler);
//uint32_t cycles_to_ms(uint64_t cycles, uint16_t _prescaler);

template<typename CyType>
uint32_t cycles_to_ms(CyType cycles, uint16_t _prescaler){
	uint32_t f_cpu = (F_CPU/_prescaler)/1000;
	return cycles/f_cpu;
}


class Timer;

class TimeStamp{
private:
	uint16_t tic;
	Timer* timer;
public:
	TimeStamp(Timer* timer);
	uint16_t toc();
};


class Timer{
protected:
	TccrbClockSelect::pre prescaler;
	timsk_reg& timsk;
	tccra_reg& tccra;
	tccrb_reg& tccrb;
	uint16_t& tcnt;
	uint16_t toi_count=0;

public:
	Timer(TccrbClockSelect::pre prescaler, timsk_reg& timsk, tccra_reg& tccra, tccrb_reg& tccrb, uint16_t& tcnt):
		prescaler(prescaler), timsk(timsk), tccra(tccra), tccrb(tccrb), tcnt(tcnt){
		//constructor here
	};
	Timer(Timer* timer): prescaler(timer->prescaler), timsk(timer->timsk), tccra(timer->tccra), tccrb(timer->tccrb), tcnt(timer->tcnt){};
	void toi_count_kick(){
		toi_count++;
	}

	uint16_t toi_count_get(){
		return toi_count;
	}

	uint32_t get_ms(){
		return cycles_to_ms<uint16_t>(toi_count*65535, 1);
	}
	TimeStamp tic(){
		return TimeStamp(this);
	}

	Timer& operator=(Timer& _me){
		return _me;
	}

	uint32_t max_range_ms(){
		uint32_t v = pow(2, sizeof(toi_count)*8) * pow(2, sizeof(uint16_t)*8);
		return cycles_to_ms<uint64_t>(v, prescaler);
	}
};



class Timer1: public Timer{
public:
	static uint16_t* toi_count_ptr;
	Timer1(TccrbClockSelect::pre = TccrbClockSelect::pre1);

};

#endif /* TIMERS_TIMERS_H_ */
