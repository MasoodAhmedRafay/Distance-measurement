/*
C Program for Distance Measurement using Ultrasonic Sensor and AVR Microocntroller
 */ 

#define  F_CPU 1000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <avr/interrupt.h>
#include "lcd.h"
static volatile int pulse = 0;
static volatile int i = 0;
volatile char inbuffer[20];

int main(void)
{
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
	sei();
	while(1)
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
	
}

ISR(INT0_vect)
{
   pulse=TCNT1;
   if (pulse == 0)
   {
	   pulse = 1;
   }
}


