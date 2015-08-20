/*
    Update 8/20/2015: Neuron is now NeuroBytes, which is part of NeuroTinker, LLC, a
    company formed by Joe Burdo and Zach Fredin. I need to make sure I'm doing everything
    right with the license, so I am leaving the stuff below this statement intact. Everything that
    started getting posted to github.com/zakqwy/NeuroBytes-v04 on or after today is covered by
    the following (with 'copyright' now spelled correctly):
	
	Copyright 2015, Zach Fredin
	zach@neurotinker.com 

	This is the firmware for a continuous rotation NeuroBytes Motor module. The ATtiny runes at 
	8 MHz. Normally, the duty cycle is kept around 1.5 ms; each individual module will need to be
	calibrated as NeuroBytes vary a bit (internal RC oscillator) and I'm guessing the CR servos only
	really stop if they get a solid 1.5 ms pulse. In any case, the inhibitory inputs cause a full speed
	CCW rotation for a given amount of time, while the excitatory inputs cause a full speed CW rotation
	for the same amount of time.
 
    General specifications:
	Servo PWM duty cycle: 10 ms (100 Hz)
	Pulse Width Range: 2 (inhibitory) to 1 (excitatory) ms
		[note that CCW rotation is the 2 ms pulse, so that is now inhibitory!]
	Zero point: nominally 1.5 ms, varies by individual NeuroBytes board
	Inputs: six (3 excitatory, 3 inhibitory) 

    File Name: /NeuroBytes-v04/FIRMWARE/v04run_servo-CR
*/
 
/*
Copywrite 2014, Zach Fredin
zachary.fredin@gmail.com
Distributed under terms of the GNU General Public License, version 3

I wrote a decent amount of this code 8 months ago. Glad I (sort of) documented it,
as I haven't gone near AVR-C (or Neurons, for that matter) since then. The only modification
I've made to this program since last time is to change two inhibitory inputs to excitatory inputs.
*/

/*
    This file is part of Neuron.

    Neuron is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Neuron is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Neuron.  If not, see <http://www.gnu.org/licenses/>.
*/

/* 	Common NeuroBytes module info
		DDRA
			PA0 = K6 (bottom left)
			PA1 = K5 (top left)
			PA2 = K4 (bottom center)
			PA3 = K3 (top center)
			PA4 = K2 (bottom right)
			PA5 = K1 (top right)
			PA6 = K7 (axon)
*/


#include <avr/io.h>
#include <avr/interrupt.h>

uint8_t inputStatus = 0b00000000;//current input status (read when stuff changes!)
uint16_t scalingFactor = 1000; //number of milliseconds in an IRL second (used to calibrate NeuroBytes to a servo; nominally 1000
uint16_t relaxTimer = 0;
uint16_t relaxConstant = 30; //number of timer cycles (defined by OCR1B) to keep spinning servo
uint16_t homePosition = 475; //zero/hold still point (varies by NeuroBytes board)
uint16_t servoPosition = 475; //current direction to set, scaled between 0 and 1000. starts at homePosition. LOWER = MORE CLOCKWISE

ISR(PCINT0_vect) { 	//interrupt svc routine called when PCINT0 changes state
	inputStatus = PINA;
}


ISR(TIM1_COMPA_vect) { //Called when TCNT1 == OCR1A
	PORTB = 0b00000111;
	PORTA = 0b00000000;
}

	
ISR(TIM1_COMPB_vect) { //Called when TCNT1 == OCR1B
	PORTB = 0b00000010;
	PORTA = 0b01000000;
	TCNT1 = 0;
	if (relaxTimer > 0) {
		relaxTimer -= 1;
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
		if ((PINA & 0b00101010) > 0) {
			servoPosition = 2 * homePosition;
			relaxTimer = relaxConstant;
		}
		else if ((PINA & 0b00010101) > 0) {
			servoPosition = 0;
			relaxTimer = relaxConstant;
		}
		else if (relaxTimer == 0) {
			servoPosition = homePosition;
		}
	}
}
