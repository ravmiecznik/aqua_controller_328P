/*
 * cbuffer.cpp
 *
 *  Created on: 24 mar 2019
 *      Author: rafal
 */

#include "cbuffer.h"



CircBuffer::CircBuffer(uint32_t siz):
	head(0), tail(0), available(0), size(siz){
	buffer = (uint8_t*)malloc(size);
	constr_method = cbuffer_constr_method::with_malloc;
}

CircBuffer::CircBuffer(CircBuffer* cbuffer){
	head = cbuffer->head;
	size = cbuffer->size;
	available = cbuffer->available;
	buffer = cbuffer->buffer;
	constr_method = cbuffer_constr_method::by_pointer;
}

CircBuffer::~CircBuffer(){
	switch (constr_method) {
		case cbuffer_constr_method::with_malloc:
			break;
		default:
			break;
	}
}


uint32_t CircBuffer::free_space(){
	return size - available;
}

void CircBuffer::flush(){
	head = 0;
	tail = 0;
	available = 0;
}

void CircBuffer::flush(uint32_t amount){
	while(amount--)
		get();
}

CircBuffer CircBuffer::peek(){
	/*
	 * Return CircBuffer created with pointer constructor
	 * It makes its own head, tail, available but points to common buffer
	 * In practice this is just freeze state of real CircBuffer can be used to peek
	 */
	return CircBuffer(this);
}

void CircBuffer::peek_sync(CircBuffer* peek_cbuffer){
	peek_cbuffer->available = available;
	peek_cbuffer->head = head;
	peek_cbuffer->tail = tail;
}

bool CircBuffer::put(uint8_t chr){
	buffer[tail] = chr;
	tail = (tail + 1) % size;
	if(available < size)
		available++;
	if(available == size)
		return false;
//		head = tail;
	return true;
}

bool CircBuffer::put(uint32_t amount, uint8_t* ext_buffer){
	uint32_t i=0;
	bool buffer_status;
	while(amount){
		buffer_status = put(ext_buffer[i]);
		if(buffer_status == false){
			return false;
		}
		amount--;
		i++;
	}
	return true;
}

uint8_t CircBuffer::get(){
	if(available){
		char c;
		c = buffer[head];
		available--;
		head = (head + 1) % size;
		return c;
	}
	else
		return 0;
}

void CircBuffer::get(uint32_t amount, uint8_t* ext_buffer){
	while(amount and available){
		*(ext_buffer++) = get();
		amount--;
	}
}

void CircBuffer::get(uint32_t amount, char* ext_buffer){
	while(amount and available){
		*(ext_buffer++) = get();
		amount--;
	}
}

//TODO: size of ext_buffer must be known!!
char* CircBuffer::get_all(char* ext_buffer){
	uint32_t cnt=0;
	char c;
	while(available){
		c = get();
		*(ext_buffer+ cnt++)=c;
	}
	ext_buffer[cnt] = get();
	flush();
	return ext_buffer;
}

volatile uint8_t* CircBuffer::get_buff_ptr(){
	return buffer;
}
