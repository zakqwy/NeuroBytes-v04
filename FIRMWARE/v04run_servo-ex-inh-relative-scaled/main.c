/*
    Update 8/18/2015: Neuron is now NeuroBytes, which is part of NeuroTinker, LLC, a
    company formed by Joe Burdo and Zach Fredin. I need to make sure I'm doing everything
    right with the license, so I am leaving the stuff below this statement intact. Everything that
    started getting posted to github.com/zakqwy/NeuroBytes-v04 on or after today is covered by
    the following (with 'copyright' now spelled correctly):
	
	Copyright 2015, Zach Fredin
	zach@neurotinker.com 
												
This is the firmware for a NeuroBytes Motor module. This particular variant, called v04run_servo-ex-inh-relative-scaled, includes both inhibitory and excitatory inputs. While holding the NeuroBytes module with the text readable--that is, the seven connectors facing you and the Axon at the bottom left--the dendrites are as labeled, with excitatory at the top and inhibitory at the bottom. From left to right, the scaling coefficient of each input is 1x, 2x, and 4x, where 'x' is the magnitude of each dendrite. This module is relative, meaning that the servo target position is varied by the scaling coefficient, with inhibitory inputs reducing the current pulse width and excitatory inputs increasing the current pulse width. This standard (inhibitory trending towards 1ms, excitatory trending towards 2ms) is maintained across NeuroBytes instead of attempting to control the servo spin direction (since servos are mounted in various ways). Relaxation speed (i.e. back to resting position at 1.5ms) is controlled by a constant of some kind. 

The ATtiny runs at 8 MHz; since the code was written around v04 boards that use the internal RC oscillator, exact servo start and end positions will vary as the pulse width won't be precisely 1 - 2 uS. As an example, the NeuroBytes v04 board used for development required a timing constant of 8 * 964 to equal one millisecond, suggesting that that particular board was off by 3.6% (at least compared to my never-been-calibrated-under-my-care 30-year-old oscilloscope).
 
    General specifications:
	Servo PWM duty cycle: 10 ms (100 Hz)
	Pulse Width Range: 1 to 2 ms
	Zero point: 1.5 ms
	Inputs: six (3 excitatory, 3 inhibitory) 
	ID: Kapton tape on Axon connector, YELLOW LED color

    File Name: /NeuroBytes-v04/FIRMWARE/v04run_servo_ex-inh-relative-scaled
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
			PA0 = K6 (bottom left) [position = -1x]
			PA1 = K5 (top left) [position = +1x]
			PA2 = K4 (bottom center) [position = -2x]
			PA3 = K3 (top center) [position = +2x]
			PA4 = K2 (bottom right) [position = -4x]
			PA5 = K1 (top right) [position = +4x]
			PA6 = K7 (axon)
*/


#include <avr/io.h>
#include <avr/interrupt.h>

uint8_t inputStatus = 0b00000000;//current input status (read when stuff changes!)
uint8_t inputStatusPrevious = 0b00000000;//previous input status (to make sure it goes up)
uint16_t advanceSpeed = 125; //advance increment (step size in us/pwm cycle, larger is faster)
uint16_t relaxSpeed = 5; //retract increment (step size in us/pwm cycle, larger is faster)
uint16_t homePosition = 1500; //point that the servo should relax to and start at (us)
uint16_t servoOut = 1500; //current position to set, scaled between 1000 and 2000 us. starts at homePosition.
uint16_t stepSize = 100; //step size for input with a scaling factor of 1 (us)
int8_t arrayDendriteScalingFactor[6] = {-1,1,-2,2,-4,4};//array that scales each input value (negative for inhibitory inputs). arranged {PA0,PA1,PA2,PA3,PA4,PA5}.

ISR(PCINT0_vect) { 	//interrupt svc routine called when PCINT0 changes state
//	inputStatusPrevious = inputStatus;
//	inputStatus = PINA;
}


ISR(TIM1_COMPA_vect) { //Called when TCNT1 == OCR1A--this is the long "trough" of the PWM cycle
	PORTB = 0b00000111; //turns off all LEDs
	PORTA = 0b00000000; //sets servo output low
}

	
ISR(TIM1_COMPB_vect) { //Called when TCNT1 == OCR1B--this is the short "peak" of the PWM cycle
	PORTB = 0b00000100; //initial LED colors (three LSBs); B,G,R inverted, 1=off
	PORTA = 0b01000000; //sets servo output high
	TCNT1 = 0;
	if ((servoOut - homePosition) > relaxSpeed) { //decay back to home position
		if (servoOut > homePosition) {
			servoOut -= relaxSpeed;
		}
	}
	else {
		servoOut = homePosition;
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
	OCR1A = 1500;
	OCR1B = 15000;
}

int main(void)
{
	SystemInit();
	for(;;){
/*
		if(inputStatus > inputStatusPrevious) { //runs when a new input is excited/inhibited
			servoOut = 2000;

		}
*/
/*
		if ((inputStatus & 0b00101010) > 0) {
			servoOut = 1750;
		}
		else if ((inputStatus & 0b00010101) > 0) {
			servoOut = 1250;
		}
*/
		if ((PINA & 0b01111111) > 0) {
			inputStatus = PINA;
		}
		if (inputStatus > inputStatusPrevious) {
			servoOut += 100;
		}
		inputStatusPrevious = inputStatus;
		OCR1A = servoOut;
	}	
}
