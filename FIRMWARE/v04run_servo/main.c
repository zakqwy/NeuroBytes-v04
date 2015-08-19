/*
 * 	v04run_servo\main.c
 *
 *  Created: 8/11/2015
 *  Author: Zach Fredin
 *	NeuroTinker, LLC
 *	zach@neurotinker.com
 
 This is the firmware for a simple NeuroBytes Motor module. The ATtiny runs at 8 MHz; since the code was written around v04 boards
 that use the internal RC oscillator, exact servo start and end positions will vary as the pulse width won't be precisely 1 - 2 uS.
 As an example, the NeuroBytes v04 board used for development required a timing constant of 8 * 964 to equal one millisecond, suggesting
 that that particular board was off by 3.6% (at least compared to my never-been-calibrated-under-my-care 30-year-old oscilloscope).
 
 General specifications:
	Servo PWM duty cycle: 10 ms (100 Hz)
	Pulse Width Range: 1 to 2 ms
 
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>

uint8_t inputStatus = 0b00000000;//current input status (read when stuff changes!)
uint16_t servoPosition = 0; //scaled between 0 and 1000; should really just use 100-900 to account for RC variations
uint16_t scalingFactor = 1000; //number of milliseconds in an IRL second (used to calibrate NeuroBytes to a servo; nominally 1000
uint16_t relaxSpeed = 35;

ISR(PCINT0_vect) { 	//interrupt svc routine called when PCINT0 changes state
	inputStatus = PINA;
}


ISR(TIM1_COMPA_vect) { //Called when TCNT1 == OCR1A
	PORTB = 0b00000111;
	PORTA = 0b00000000;
}

	
ISR(TIM1_COMPB_vect) { //Called when TCNT1 == OCR1B
	PORTB = 0b00000001;
	PORTA = 0b01000000;
	TCNT1 = 0;
	if (servoPosition > relaxSpeed) {
		servoPosition -= relaxSpeed;
	}
}
void SystemInit(void) {
	DDRA = 0b01000000; //IO config: PA0,1,2,3,4,5 in (dendrites), PA6 out (servo)
	PORTA = 0b00000000; //Turns off pull-up resistors on dendrites, sets axon low
	DDRB = 0b00000111; //IO config: PB0,1,2 out (LEDs), all others in
	PORTB = 0b00000111; //Sets PB0,1,2 high to start (LEDs off).
	TCCR1B = 0b00000010; //sets up a timer at 1MHz (or base AVR speed / 8)
	sei(); //enable all interrupts: same as changing bit 7 of SREG to 1
	GIMSK |= (1<<4);//sets the general input mask register's 4th bit to 1 to activate the PCIE0 interrupt stuff
	TIMSK1 = 0b00000110; // activates OCIE1A and OCIE1B: output compare match A and B 
	PCMSK0 = 0b00111111;	//sets the six dendrites to active hardware interrupts (0-5)
	OCR1A = scalingFactor;
	OCR1B = scalingFactor*20;
}

int main(void)
{
	SystemInit();
	for(;;){
		OCR1A = scalingFactor + servoPosition;
		if ((PINA & 0b00111111) > 0)  {
			servoPosition = 1000;
		}
	}
}