#define  F_CPU 1000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdbool.h>
#include <util/delay.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <avr/interrupt.h>
#include "lcd.h"
#include "nrf24l01.h"


void setup_timer(void);
nRF24L01 *setup_rf(void);

volatile bool rf_interrupt = false;
volatile bool send_message = false;
static volatile int pulse = 0;
static volatile int i = 0;
volatile char inbuffer[20];

int main(void) {
	
	char     outbuffer[20];
	DDRC = 0xFF;
	DDRB = 0xFF;
	DDRD = 0b11111011;
	_delay_ms(50);
	lcd_init(LCD_DISP_ON);
	EICRA |= (1<<ISC01)|(0<<ISC00);
	EIMSK |= (1<<INT0);
	TCCR1A = 0;
	//timer1 at 1MHz
	TCCR1B = (0 << CS12)|(0 << CS11)|(1 << CS10);
	int16_t COUNTA = 0;
	lcd_puts("Distance Measurement");
	_delay_ms(1000);
	lcd_clrscr();
	//sei();
	
	uint8_t to_address[5] = { 0x01, 0x01, 0x01, 0x01, 0x01 };
	bool on = false;
	sei();
	nRF24L01 *rf = setup_rf();
	setup_timer();
	nRF24L01_begin(rf);
	while(true)
	{
		PORTD|=(1<<PIND0);
		TCNT1=0;
		_delay_us(15);
		pulse=0;
		PORTD &=~(1<<PIND0);
		while (pulse == 0)
		{
			_delay_us(10);
		}
		COUNTA = pulse/58;
		lcd_gotoxy(0,0);
		lcd_puts(outbuffer);
		itoa(COUNTA,outbuffer,10);
		sprintf(outbuffer, "Distance = %5u", COUNTA);
	}

	while (true) {
		if (rf_interrupt) {
			rf_interrupt = false;
			int success = nRF24L01_transmit_success(rf);
			if (success != 0)
			nRF24L01_flush_transmit_message(rf);
		}

		if (send_message) {
			send_message = false;
			on = !on;
			nRF24L01Message msg;
			if (on) memcpy(msg.data, "ON", 3);
			else memcpy(msg.data, "OFF", 4);
			msg.length = strlen((char *)msg.data) + 1;
			//			nRF24L01_transmit(rf, to_address, &msg);
		}
	}
	//	}
	return 0;

}

nRF24L01 *setup_rf(void) {
	nRF24L01 *rf = nRF24L01_init();
	rf->ss.port = &PORTB;
	rf->ss.pin = PB2;
	rf->ce.port = &PORTB;
	rf->ce.pin = PB1;
	rf->sck.port = &PORTB;
	rf->sck.pin = PB5;
	rf->mosi.port = &PORTB;
	rf->mosi.pin = PB3;
	rf->miso.port = &PORTB;
	rf->miso.pin = PB4;
	//interrupt on falling edge of INT0 (PD2)
	EICRA |= _BV(ISC01);
	EIMSK |= _BV(INT0);
	
	return rf;
}

// setup timer to trigger interrupt every second when at 1MHz
void setup_timer(void) {
	TCCR1B |= _BV(WGM12);
	TIMSK1 |= _BV(OCIE1A);
	OCR1A = 15624;
	TCCR1B |= _BV(CS10) | _BV(CS11);
}

// each one second interrupt
ISR(TIMER1_COMPA_vect) {
	send_message = true;
}

// nRF24L01 interrupt
ISR(INT0_vect) {
	pulse=TCNT1;
	if (pulse == 0)
	{
		pulse = 1;
		//rf_interrupt = true;
	}
	rf_interrupt = true;
}
