#include "setup.h"



AvrPin arduino_led		(PIN::pin5, (AvrPort&)PINB, PIN::out);	//Arduino D13
AvrPin relay_sump_pump	(PIN::pin4, (AvrPort&)PINB, PIN::out);	//Arduino D12
AvrPin relay_feed_pump	(PIN::pin3, (AvrPort&)PINB, PIN::out);	//Arduino D11
AvrPin level_sensor_low	(PIN::pin2, (AvrPort&)PINB, PIN::in);	//Arduino D10
AvrPin level_sensor_hi	(PIN::pin1, (AvrPort&)PINB, PIN::in);	//Arduino D09
AvrPin button			(PIN::pin0, (AvrPort&)PINB, PIN::in);	//Arduino D08
AvrPin ext_diode		(PIN::pin7, (AvrPort&)PIND, PIN::out);	//Arduino D07
AvrPin out_compare_0A	(PIN::pin6, (AvrPort&)PIND, PIN::out);	//Arduino D06

#define baud	9600
#define rx_buff_size	20
#define tx_buff_size	40
Usart usart(usart0, baud, rx_buff_size, tx_buff_size);


Timer1 timer1(TccrbClockSelect::pre8);
Tasks<SimpleTask , Timer1> Task_Scheduler(timer1);

char* STR_BUFFER = (char*)malloc(50);


//**********************************************************//
//    CONFIGURE STDOUT/STDERR                               //
//**********************************************************//

static int put(char c, FILE *stream){
	usart.Putchar(c);
	return 0;
}
static FILE uartout = {0};
void setup_stdout_for_printf(){
	fdev_setup_stream(&uartout, put, NULL, _FDEV_SETUP_WRITE);
	stdout = &uartout;
	stderr = &uartout;
}
/****************************************************************/


int printfp(const char* format, ...){
	/*
	 * Uses formated print to attach pgmspace string to message
	 */
	if(strlen_P(format) <70){
		char buff[50];
		va_list arg;
		int done;
		va_start (arg, format);
		done = vsprintf_P(buff, format, arg);
		va_end (arg);
		printf(buff);
		return done;
	}
	return 0;
}

