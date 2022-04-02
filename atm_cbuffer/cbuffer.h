/*
 * cbuffer.h
 *
 *  Created on: 28 lip 2017
 *      Author: miecznik
 */

#ifndef CBUFFER_H_
#define CBUFFER_H_

#include <stdint.h>
#include <stdlib.h>
#include <avr/pgmspace.h>
#include <avr/delay.h>
#include <stdio.h>
#include <ctype.h>


namespace cbuffer_constr_method{
	enum constr_method{
		with_malloc,
		by_pointer,
	};
}


class CircBuffer{
private:
	volatile uint32_t head;
	volatile uint32_t tail;
	cbuffer_constr_method::constr_method constr_method;
public:
	volatile uint32_t available;
	volatile uint8_t* buffer;
	volatile uint32_t size;
	CircBuffer(uint32_t siz);
	CircBuffer(CircBuffer* cbuffer);
	~CircBuffer();
	bool put(uint8_t chr);
	bool put(uint32_t amount, uint8_t* ext_buffer);
	uint32_t free_space();
	void flush();
	void flush(uint32_t amount);
	CircBuffer peek();
	void peek_sync(CircBuffer* peek_cbuffer);
	uint8_t get();
	void get(uint32_t amount, uint8_t* ext_buffer);
	void get(uint32_t amount, char* ext_buffer);
	char* get_all(char* ext_buffer);
	template<typename data_type>
	void set_head(uint32_t new_head);
	volatile uint8_t* get_buff_ptr();
};


#endif /* CBUFFER_H_ */
