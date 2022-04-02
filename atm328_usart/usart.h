/*
 * usart.h
 *
 *  Created on: Jul 28, 2017
 *      Author: rafal
 *
 *      This library supports atmega128
 */

#ifndef USART_H_
#define USART_H_

#include "../atm_cbuffer/cbuffer.h"
#include <avr/io.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <util/crc16.h>

#define _crc(crc, data) _crc_xmodem_update(crc, data)
#define USART0_ENABLE

#ifdef USART0_ENABLE
void usart0_bind_buffers(CircBuffer* rx_buffer, CircBuffer* tx_buffer);
#endif


uint16_t calc_crc(uint8_t *buffer, uint16_t siz);

enum usart_num{
	usart0,
};

void rx_null_function(uint8_t data);


//TODO: to use cbuffer methods test inheritance of cbuffer
class Usart{
private:
	volatile uint8_t* uart_status_register;
	volatile uint8_t* uart_control_register_B;
	volatile uint8_t* uart_control_register_C;
	volatile uint8_t* uart_data_register;
	volatile uint8_t* ubrrh_register;
	volatile uint8_t* ubrrl_register;
	uint8_t rx_interrupt_enable_bit;
	uint8_t uart_enable_flags;
	uint8_t uart_async_8bit_noparity_1stopbit_flags;
	uint8_t udrie;
	uint8_t u2x;
public:
	CircBuffer rx_buffer;
	CircBuffer tx_buffer;
	Usart(usart_num u_num, uint32_t baud, uint32_t rx_buff_siz=100, uint32_t tx_buff_siz=100, void (*rx_action)(uint8_t data)=rx_null_function);
	void uart_init(uint32_t baud);
	void rx_interrupt_disable();
	void rx_interrupt_enable();
	void Putchar(char c);
	void puts_p(const char*);
	void puts_p(const char* str, char c);
	char get();
	void get(uint32_t amount, uint8_t* ext_buffer);
	char* get(uint32_t amount, char* ext_buffer);
	uint16_t get_uint();
	char* get_all(char* ext_buff);
	uint32_t available();
};




#endif /* USART_H_ */
