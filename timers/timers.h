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
#include <stdio.h>

#define __ASSERT_USE_STDERR
#include <assert.h>

#include "../build_setup.h"


typedef volatile uint8_t& reg8b_ref;
typedef void (*void_void_fptr)(void);
void register_timer1_compa_func(void_void_fptr func);
void set_fast_pwm_timer0(uint8_t pwm);



//**********************************************************//
//    TIMER REGISTERS                                       //
//**********************************************************//


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
		ctc_ocra, 					//clear timer on compare
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



//**********************************************************//
//    AUXILIARY FUNCTIONS                                   //
//**********************************************************//


void compa_int_delay_s(uint16_t seconds, tccrb_reg& tccrb, volatile uint16_t* ocr);


template<typename CyType, typename RetType=uint32_t>
RetType cycles_to_ms(CyType cycles, uint16_t _prescaler){
	RetType f_cpu = (F_CPU/_prescaler)/1000;
	if(_prescaler < 256){
		return cycles/f_cpu;
	}
	else{
		f_cpu = F_CPU/1000;
		return _prescaler*cycles/f_cpu;
	}
}


template<typename CyType>
uint32_t cycles_to_sec(CyType cycles, uint16_t _prescaler){
	return cycles_to_ms<CyType, uint64_t>(cycles, _prescaler)/1000;
}

template<typename CyType>
uint32_t cycles_to_min(CyType cycles, uint16_t _prescaler){
	return cycles_to_sec<CyType>(cycles, _prescaler)/60;
}

template<typename CyType>
uint32_t cycles_to_hour(CyType cycles, uint16_t _prescaler){
	return cycles_to_min<CyType>(cycles, _prescaler)/60;
}

template<typename CyType>
uint32_t cycles_to_days(CyType cycles, uint16_t _prescaler){
	return cycles_to_hour<CyType>(cycles, _prescaler)/24;
}

uint16_t get_prescaler_value(tccrb_reg &tccrb);



//**********************************************************//
//    TIMER CLASSES                                         //
//**********************************************************//

struct TimeStamp{
	/*
	 * TimeStamp with 1s resolution
	 * TODO: rework to support ms
	 */
private:
	uint32_t timestamp_s=0;
	static char* buffer;

public:
	uint16_t day=0;
	uint8_t hour=0;
	uint8_t minute=0;
	uint8_t second=0;


	TimeStamp(uint16_t days, uint8_t hours, uint8_t minutes, uint8_t seconds):
		day(days), hour(hours), minute(minutes), second(seconds)
	{};
	TimeStamp(){
		day = 0;
		hour = 0;
		minute = 0;
		second = 0;
	}

	TimeStamp(uint32_t raw_time_s);
	void init_from_seconds(uint32_t raw_time_s);
	uint32_t to_seconds();

	TimeStamp operator+(TimeStamp& other){
		return TimeStamp(other.to_seconds() + to_seconds());
	}

	operator uint32_t(){
		return to_seconds();
	}

	void normalize(uint32_t max_value);

	operator const char*();
};

class Timer;
/*
 * A root prototype for all timers-> 16bit or 8bit
 */
class TimeCount{
private:
	uint16_t tic;
	Timer* timer;
public:
	TimeCount(Timer* timer);
	uint16_t toc();
};

class Timer{
	/*
	 * Prototype class for both timer 8 and 16
	 */
protected:
	TccrbClockSelect::pre prescaler;
	timsk_reg& timsk;
	tccra_reg& tccra;
	tccrb_reg& tccrb;
	volatile uint16_t toi_count=0;

public:
	static bool timer_type;

	Timer(TccrbClockSelect::pre prescaler, timsk_reg& timsk, tccra_reg& tccra, tccrb_reg& tccrb):
		prescaler(prescaler), timsk(timsk), tccra(tccra), tccrb(tccrb){
	};

	Timer(const Timer& source):
		prescaler(source.prescaler), timsk(source.timsk), tccra(source.tccra), tccrb(source.tccrb){
	};


	void toi_count_kick(){
		toi_count++;
	}

	uint16_t toi_count_get(){
		return toi_count;
	}

	uint32_t get_ms(){
		return cycles_to_ms<uint32_t>(toi_count*65536, get_prescaler());
	}

	int16_t get_prescaler(){
		return get_prescaler_value(tccrb);
	}

	TimeCount tic(){
		return TimeCount(this);
	}


	uint32_t max_range_ms();
	uint32_t max_range_sec();
	uint32_t max_range_min();
	uint32_t max_range_hours();
	uint32_t max_range_days();
	TimeStamp max_range();
	TimeStamp now();

};


template<typename TType>
class TimerT: public Timer{
	/*
	 * Specify timer8 or timer16
	 */
protected:
	TType& tcnt;
public:
	TimerT(TccrbClockSelect::pre prescaler, timsk_reg& timsk, tccra_reg& tccra, tccrb_reg& tccrb, TType& _tcnt):
		Timer(prescaler, timsk, tccra, tccrb), tcnt(_tcnt){};
	TimerT(const TimerT& source):
		Timer(source.prescaler, source.timsk, source.tccra, source.tccrb), tcnt(source.tcnt){
	};
	uint32_t get_timestamp(){
		/*
		 * Returns raw timestamp: 0xFFFF * toi_count + tcnt
		 * if timer is 16bit.
		 */
		return ((uint32_t)toi_count << (sizeof(TType)*8)) + (uint32_t)tcnt;
	}

	uint32_t get_timestamp_s(){
		/*
		 * Returns raw timestamp in seconds
		 */
		return to_seconds(get_timestamp());
	}

	uint32_t to_cycles(uint32_t seconds)
	{
		uint32_t cycles_per_second = F_CPU/get_prescaler();
		return seconds * cycles_per_second;
	}

	uint32_t to_seconds(uint32_t cycles)
	{
		uint32_t cycles_per_second = F_CPU/get_prescaler();
		return cycles/cycles_per_second;
	}

};


class Timer1: public TimerT<uint16_t>{
	/*
	 * 16bit Timer1
	 */
public:
	volatile static uint16_t* toi_count_ptr;
	Timer1(TccrbClockSelect::pre = TccrbClockSelect::pre1);
	Timer1(const Timer1& source);
};

#endif /* TIMERS_TIMERS_H_ */
