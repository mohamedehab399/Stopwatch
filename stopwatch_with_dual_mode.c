/*
 * stopwatch_with_dual_mode.c
 *
 *  Created on: Aug 30, 2025
 *      Author: Mohamed Ehab
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

// define the modes
#define COUNT_UP   1
#define COUNT_DOWN 0

// define a global variable for each number in the display
short sec_ones=0, sec_tens=0;
short min_ones=0, min_tens=0;
short hour_ones=0, hour_tens=0;

// define the mode of the stopwatch and initialize with COUNT_UP
unsigned char mode = COUNT_UP;

// write the initialization functions for the interrupts
// timer1 ctc mode initialization function
void timer1_ctc(void){
	TCNT1 = 0;
	OCR1A = 15625;            // calculate the 1 second at 16MHz / 1024
	TIMSK |= (1<<OCIE1A);     // enable compare A interrupt
	TCCR1A = (1<<FOC1A);      // enable it for ctc mode (non-PWM mode)
	TCCR1B = (1<<WGM12) | (1<<CS12) | (1<<CS10);  // set WGM12=1  for CTC mode and  CS12=1, CS10=1 for prescaler = 1024
}

//interrupt 0 initialization function
void INT0_Init (void){
	DDRD  &= ~(1<<PD2);
	PORTD |= (1<<PD2);       // enable pull-up
	MCUCR |= (1<<ISC01);     // set falling edge
	GICR  |= (1<<INT0);
}

//interrupt 1 initialization function
void INT1_Init (void){
	DDRD  &= ~(1<<PD3);
	MCUCR |= (1<<ISC11)|(1<<ISC10);    // set rising edge
	GICR  |= (1<<INT1);
}

//interrupt 2 initialization function
void INT2_Init (void){
	DDRB  &= ~(1<<PB2);
	PORTB |= (1<<PB2);        // enable pull-up
	MCUCSR &= ~(1<<ISC2);     // set falling edge
	GICR  |= (1<<INT2);
}


// set Timer1 ISR (Interrupt service routine)
ISR(TIMER1_COMPA_vect){
	if (mode == COUNT_UP) {  // count up mode
		sec_ones++;
		if (sec_ones == 10) {
			sec_ones = 0;
			sec_tens++;
			if (sec_tens == 6) {
				sec_tens = 0;
				min_ones++;
				if (min_ones == 10) {
					min_ones = 0;
					min_tens++;
					if (min_tens == 6) {
						min_tens = 0;
						hour_ones++;
						if (hour_ones == 10) {
							hour_ones = 0;
							hour_tens++;
						}
					}
				}
			}
		}
	}
	else if(mode == COUNT_DOWN) {  // countdown mode
		if(!(sec_ones == 0 && sec_tens == 0 && min_ones == 0 && min_tens == 0 && hour_ones == 0 && hour_tens == 0)) {
			sec_ones--;
			if (sec_ones < 0) {
				sec_ones = 9;
				sec_tens--;
				if (sec_tens < 0) {
					sec_tens = 5;
					min_ones--;
					if (min_ones < 0) {
						min_ones = 9;
						min_tens--;
						if (min_tens < 0) {
							min_tens = 5;
							hour_ones--;
							if (hour_ones < 0) {
								hour_ones = 9;
								hour_tens--;
							}
						}
					}
				}
			}
		}
		else {
			PORTD |= (1<<PD0);   // buzzer on that Stop timer at zero and clear bits
			TCCR1B &= ~((1<<CS12)|(1<<CS11)|(1<<CS10));
		}
	}
}

//set external Interrupts ISR
ISR(INT0_vect){   // use interrupt 0 for reset
	sec_ones=0; sec_tens=0;
	min_ones=0; min_tens=0;
	hour_ones=0; hour_tens=0;
	PORTD &= ~(1<<PD0); // buzzer off
}

ISR(INT1_vect){   // use interrupt 1 for pause and stop Timer1
	TCCR1B &= ~((1<<CS12)|(1<<CS11)|(1<<CS10));
}

ISR(INT2_vect){   // use interrupt 2 for  resume (CS12=1 and CS10=1 makes prescaler = 1024)
	TCCR1B |= (1<<CS12) | (1<<CS10);
}


int main(void){
	// set directions of the ports
	DDRC |= (1<<PC0)|(1<<PC1)|(1<<PC2)|(1<<PC3); // BCD to 7-segment (7447)
	DDRA |= (1<<PA0)|(1<<PA1)|(1<<PA2)|(1<<PA3)|(1<<PA4)|(1<<PA5); // 7-segment enables
	DDRD |= (1<<PD4)|(1<<PD5)|(1<<PD0);      // mode_LEDs and buzzer

	// set buttons
	DDRB &= ~((1<<PB7)|(1<<PB1)|(1<<PB0)|(1<<PB4)|(1<<PB3)|(1<<PB5)|(1<<PB6)|(1<<PB2));  // set direction as input
	PORTB |= (1<<PB7)|(1<<PB1)|(1<<PB0)|(1<<PB4)|(1<<PB3)|(1<<PB5)|(1<<PB6)|(1<<PB2);    // enable internal pull-ups for switches
	PORTD |= (1<<PD2);  // enable internal pull-up for INT0

	// set LEDs count-up mode as default (red-LED on)
	PORTD &= ~(1<<PD5);
	PORTD |= (1<<PD4);
	PORTC = 0;

	// set Flags
	unsigned char mode_toggle=0, decrement_sec=0, increment_sec=0;
	unsigned char decrement_min=0, increment_min=0;
	unsigned char decrement_hour=0, increment_hour=0;

	// call initialization functions
	INT0_Init();
	INT1_Init();
	INT2_Init();
	timer1_ctc();
	sei();        // enable global interrupt by function

	while(1)
	{
		// toggle between modes (count-up or count-down)
		if (!(PINB & (1 << PB7))) {
			if(mode_toggle==0){
				mode = (mode == COUNT_UP) ? COUNT_DOWN : COUNT_UP;
				PORTD &= ~(1<<PD0); // buzzer off
				mode_toggle=1;
			}
		}
		else {
			mode_toggle=0;
		}

		// LEDs show mode
		if (mode == COUNT_UP){  // for count up mode (red LED)
			PORTD = (PORTD | (1 << PD4)) & ~(1 << PD5);
		}
		else{ // for count down mode (yellow LED)
			PORTD = (PORTD | (1 << PD5)) & ~(1 << PD4);
		}

		// seconds change
		if (!(PINB & (1 << PB5))){ // decrement seconds
			if(decrement_sec==0){
				sec_ones--;
				if(sec_ones < 0){
					sec_ones=9;
					if(sec_tens>0){
						sec_tens--;
					}
				}
				decrement_sec=1;
			}
		}
		else {
			decrement_sec=0;
		}

		if(!(PINB & (1<<PB6))){ // increment seconds
			if(increment_sec==0){
				sec_ones++;
				if(sec_ones==10){
					sec_ones=0;
					sec_tens++;
					if(sec_tens==6) {
						sec_tens=0;
					}
				}
				increment_sec=1;
			}
		}
		else {
			increment_sec=0;
		}

		// minutes change
		if (!(PINB & (1 << PB3))){ // decrement minutes
			if(decrement_min==0){
				min_ones--;
				if(min_ones < 0){
					min_ones=9;
					if(min_tens>0){
						min_tens--;
					}
				}
				decrement_min=1;
			}
		}
		else{
			decrement_min=0;
		}

		if (!(PINB & (1 << PB4))){ // increment minutes
			if(increment_min==0){
				min_ones++;
				if(min_ones==10){
					min_ones=0;
					min_tens++;
					if(min_tens==6){
						min_tens=0;
					}
				}
				increment_min=1;
			}
		}
		else {
			increment_min=0;
		}

		// hours change
		if (!(PINB & (1 << PB0))){ // decrement hour
			if(decrement_hour==0){
				hour_ones--;
				if(hour_ones < 0){
					hour_ones=9;
					if(hour_tens>0){
						hour_tens--;
					}
				}
				decrement_hour=1;
			}
		}
		else{
			decrement_hour=0;
		}

		if (!(PINB & (1 << PB1))){ // increment hour
			if(increment_hour==0){
				hour_ones++;
				if(hour_ones==10){
					hour_ones=0;
					hour_tens++;
				}
				increment_hour=1;
			}
		}
		else {
			increment_hour=0;
		}

		// multiplexed display that controlled by port a
		// control hours tens and ones
		PORTA=0;
		PORTC = (PORTC & 0xF0) | (hour_tens & 0x0F);   //make sure the first 4 bits in the port is empty and then fill them with BCD bits
		PORTA|=(1<<PA0);      // enable the one 7-segment (first from left hour tens)
		_delay_ms(2);         // delay for flickering (blinking) of the 7-segment displays
		PORTA&=~(1<<PA0);     // disable it

		PORTC = (PORTC & 0xF0) | (hour_ones & 0x0F);
		PORTA|=(1<<PA1);
		_delay_ms(2);
		PORTA&=~(1<<PA1);

		// control minutes tens and ones
		PORTC = (PORTC & 0xF0) | (min_tens & 0x0F);
		PORTA|=(1<<PA2);
		_delay_ms(2);
		PORTA&=~(1<<PA2);

		PORTC = (PORTC & 0xF0) | (min_ones & 0x0F);
		PORTA|=(1<<PA3);
		_delay_ms(2);
		PORTA&=~(1<<PA3);

		// control seconds tens and ones
		PORTC = (PORTC & 0xF0) | (sec_tens & 0x0F);
		PORTA|=(1<<PA4);
		_delay_ms(2);
		PORTA&=~(1<<PA4);

		PORTC = (PORTC & 0xF0) | (sec_ones & 0x0F);
		PORTA|=(1<<PA5);
		_delay_ms(2);
		PORTA&=~(1<<PA5);
	}
}
