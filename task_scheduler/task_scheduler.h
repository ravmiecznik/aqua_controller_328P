/*
 * task_scheduler.h
 *
 *  Created on: 3 kwi 2022
 *      Author: rafal
 */

#ifndef TASK_SCHEDULER_TASK_SCHEDULER_H_
#define TASK_SCHEDULER_TASK_SCHEDULER_H_

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <assert.h>
#include <avr/pgmspace.h>
#include "../build_setup.h"

/*
 * Example Task usage

	int toggle_led2(){
		static int val=0;
		arduino_led.toggle();
		return val++;
	}

	typedef void (*task_fptr)(void);
	Task<Timer::TimeStamp, task_fptr> task(Timer::TimeStamp(1,2,3,4), (task_fptr)toggle_led);


	or more straight declaration:

	int toggle_led2(){
		static int val=0;
		arduino_led.toggle();
		return val++;
	}

	Task<Timer::TimeStamp, int (*)(void)> task2(Timer::TimeStamp(1,2,3,4), (int (*)(void))toggle_led2);

	...
	task2.run()		//is it possible to provide arguments to run ?

 */

template<typename ArrType>
ArrType* sort(ArrType* array, uint32_t start, uint32_t end){
	/*
	 * Sort an array from start to end index
	 */
	ArrType temp;
	for(uint32_t i = start; i<end; i++)
	{
		for(uint32_t j = i+1; j<end; j++)
		{
			if(array[i]>array[j])
			{
				temp = array[i];
				array[i] = array[j];
				array[j] = temp;
			}
		}
	}
	return array;
}


template<typename TStampType, typename Fptr>
class Task
/*
 * Create a task object: timestamp: executable
 * Timestamp: planned time for executable task
 */
{

	Fptr executable;

public:
	TStampType tstamp;
	Task(TStampType tstamp, Fptr executable):
		tstamp(tstamp), executable(executable)
	{

	}
	Task(){
		tstamp = TStampType(0);
		executable = nullptr;
	}

	Fptr run(){
		if(executable){
#ifdef DEBUG
			printf_P(PSTR("##Task##\n"));
			printf(tstamp);
#endif
			return (Fptr)executable();
		}
		else{
			return nullptr;
		}
	}

	operator uint32_t(){
		return (uint32_t)tstamp;
	}

	operator bool() const{
		return (bool)nullptr;
	}
};



template<typename TaskType>
class SortedStack
{
	TaskType* buffer = nullptr;
	uint8_t size;
	uint8_t stack_ptr;
public:
	SortedStack(uint8_t size):size(size), stack_ptr(size){
		buffer = (TaskType*)malloc(size * sizeof(TaskType));
	}

	bool put(TaskType v){
		if(stack_ptr){
			buffer[stack_ptr-1] = v;
			sort(buffer, stack_ptr-1, size);
			stack_ptr--;
			return true;
		}
		else{
			return false;
		}
	}

	TaskType get(){
		if(stack_ptr < size){
			return buffer[stack_ptr++];
		}
		else{
			TaskType null_task;
			return null_task;
		}
	}

	TaskType peek(){
		return buffer[stack_ptr];
	}

};

template<typename TaskType, class Timer>
class Tasks: public SortedStack<TaskType>
/*
 * Manage task list
 */
{
private:
	Timer& timer;

public:
	Tasks(Timer& timer, uint8_t tasks_num=10):SortedStack<TaskType>(tasks_num), timer(timer){
		assert(timer.timer_type);		//assure only Timer type provided, static_assert not supported for AVR lib
	}

	bool put(TaskType t){
		if(t.tstamp.to_seconds() > timer.max_range_sec()){
			t.tstamp.normalize(timer.max_range_sec());	//overflow protection
		}
		return SortedStack<TaskType>::put(t);
	}

	void check(){
		/*
		 * Run task according to its time stamp
		 */
		TaskType task = SortedStack<TaskType>::peek();
		if(timer.get_timestamp_s() >= task.tstamp.to_seconds()){
			task = SortedStack<TaskType>::get();
			task.run();
		}
	}

};



#endif /* TASK_SCHEDULER_TASK_SCHEDULER_H_ */
