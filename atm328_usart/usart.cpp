/*
 * usart.cpp
 *
 *  Created on: Jul 28, 2017
 *      Author: rafal
 *      This library supports atmega128
 *
 */

#include "usart.h"
#include <avr/interrupt.h>
#include <avr/delay.h>
#include <string.h>


void rx_null_function(uint8_t data)
{

}

void (*RX_FUNCTION_U0)(uint8_t data) = rx_null_function;

Usart::Usart(usart_num usart, uint32_t baud, uint32_t rx_buff_siz, uint32_t tx_buff_siz, void (*rx_action)(uint8_t data)):
				rx_buffer(rx_buff_siz), tx_buffer(tx_buff_siz){

	uart_status_register = &UCSR0A;
	uart_control_register_B = &UCSR0B;
	uart_control_register_C = &UCSR0C;
	uart_data_register = &UDR0;
	ubrrh_register = &UBRR0H;
	ubrrl_register = &UBRR0L;
	udrie = UDRIE0;
	u2x = U2X0;
	/* Enable USART receiver and transmitter and receive complete interrupt */
	rx_interrupt_enable_bit = RXCIE0;
	uart_enable_flags = _BV(RXCIE0)|_BV(RXEN0)|_BV(TXEN0);
	uart_async_8bit_noparity_1stopbit_flags = _BV(UCSZ01) | _BV(UCSZ00);
	usart0_bind_buffers(&rx_buffer, &tx_buffer);
	RX_FUNCTION_U0 = rx_action;

	uart_init(baud);

}

void Usart::Putchar(char c){
	while ( tx_buffer.available == tx_buffer.size ) {
		;/* wait for free space in buffer */
	}
	tx_buffer.put((uint8_t)c);
	/* enable UDRE interrupt */
	*uart_control_register_B |= _BV(udrie);
	_delay_us(100);
}

void Usart::uart_init(uint32_t baudrate){
	baudrate = F_CPU/16/baudrate -1;
	if ( baudrate & 0x8000 ) {
		*uart_status_register = (1<<u2x);  //Enable 2x speed
		baudrate &= ~0x8000;
	}
	*ubrrh_register = (uint8_t)(baudrate>>8);
	*ubrrl_register = (uint8_t) baudrate;

	/* Enable USART receiver and transmitter and receive complete interrupt */
	*uart_control_register_B = uart_enable_flags;

	*uart_control_register_C = uart_async_8bit_noparity_1stopbit_flags;
}

void Usart::rx_interrupt_disable(){
	*uart_control_register_B &= ~_BV(rx_interrupt_enable_bit);
}

void Usart::rx_interrupt_enable(){
	*uart_control_register_B |= _BV(rx_interrupt_enable_bit);
}

char Usart::get(){
	return rx_buffer.get();
}

void Usart::get(uint32_t amount, uint8_t* ext_buffer){
	rx_buffer.get(amount, ext_buffer);
}

char* Usart::get(uint32_t amount, char* ext_buffer){
	rx_buffer.get(amount, ext_buffer);
	return ext_buffer;
}

char* Usart::get_all(char* ext_buff){
	return rx_buffer.get_all(ext_buff);
}

uint32_t Usart::available(){
		return rx_buffer.available;
}

void Usart::puts_p(const char* str, char c){
		puts_p(str);
		Putchar(c);
}



CircBuffer *rx0_buffer;
CircBuffer *tx0_buffer;

void usart0_bind_buffers(CircBuffer* rx_buffer, CircBuffer* tx_buffer){
	rx0_buffer = rx_buffer;
	tx0_buffer = tx_buffer;
}


////////USART0//////////////////////////////////////////////////////////////////////////
	#define UART0_STATUS   UCSR0A
	#define UART0_CONTROL  UCSR0B
	#define UART0_DATA     UDR0
	#define UART0_UDRIE    UDRIE0


ISR(USART_RX_vect)
/*************************************************************************
Function: UART Receive Complete interrupt
Purpose:  called when the UART has received a character
**************************************************************************/
{
    volatile uint8_t data;
	data = UDR0;
	rx0_buffer->put(data);
	RX_FUNCTION_U0(data);

}

ISR(USART_UDRE_vect)
/*************************************************************************
Function: UART Data Register Empty interrupt
Purpose:  called when the UART is ready to transmit the next byte
**************************************************************************/
{
	if(UART0_STATUS & (1<<UDRE0)){
		if ( tx0_buffer->available){
			UART0_DATA = tx0_buffer->get();
		} else {
			UART0_CONTROL &= ~_BV(UART0_UDRIE);
		}
	}
}


uint16_t calc_crc(uint8_t *buffer, uint16_t siz){
	/*
	 * Function calculates crc for provided array only for first "siz" bytes
	 */
	uint16_t crc = 0;
	for (uint16_t i = 0; i < siz; ++i) {
		crc = _crc(crc, buffer[i]);
	}
	return crc;
}




