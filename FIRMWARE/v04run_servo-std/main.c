/* 
    Update 8/18/2015: Neuron is now NeuroBytes, which is part of NeuroTinker, LLC, a
    company formed by Joe Burdo and Zach Fredin. I need to make sure I'm doing everything
    right with the license, so I am leaving the stuff above this line intact. Everything that
    started getting posted to github.com/zakqwy/NeuroBytes-v04 on or after today is covered by
    the following (with 'copyright' now spelled correctly):

	Copyright 2015, Zach Fredin
	zach@neurotinker.com

This file is a Motor NeuroBytes program that acts somewhat like a standard NeuroBytes board. The devices has three excitatory and three inhibitory inputs as marked; however, from left to right the dendrites are scalled 1, 2, and 4. The Fire system is scrapped and the LED is held at a constant yellow. Timing is adjusted so that the Axon outputs current membrane potential to a servo, such that a potential of 0 outputs 1.5ms (servo at halfway point), and inhibitory/excitatory outputs move that output between 1ms (fully inhibited) and 2ms (fully excited). Limit stops are set initially to prevent over-rotation of the servo. Decay functions as with a standard NeuroBytes board.

    This file, NeuroBytes-v04/FIRMWARE/v04run_standard, is the current v04 runtime software
    for a standard NeuroBytes board. As of today, these boards feature a 70%-per-input weighting
    matched across six dendrites (five excitatory, one inhibitory). 

    File Name: /NeuroBytes-v04/FIRMWARE/v04run_servo-std
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

#include <avr/io.h>
#include <avr/interrupt.h>

//Preset LEDs to offset their phases. Right now this is 0 (ground) to 100 (brightest).
//Signed because it has to do math with potential (and that's signed) and Zach is
//worried about different variable types getting fussy with each other. Note that this
//also controls the timer2Overflow (SLOW loop timer), since the fade value gets compared
//to that loop's counter. 
int16_t ledGreenfade = 1;
int16_t ledRedfade = 0;
int16_t ledBluefade = 0;

//debounce value for inputs and six counters so we can have multiple inputs debounced properly
//if they arrive near each other. 8 bit unsigned means the longest debounce time available
//is 256 ticks of the SLOW loop, or (if I didn't change the loop time) roughly 1/4 second.
//probably adequate even for my shitty switches.
uint8_t debounceValue = 5;
uint8_t exDebounceCount1 = 0;//excitatory 1, which is K1, pin 8, PA5, and PCINT5.
uint8_t exDebounceCount2 = 0;//excitatory 2, which is K3, pin 10, PA3, and PCINT3.
uint8_t exDebounceCount3 = 0;//excitatory 3, which is K5, pin 12, PA1, and PCINT1.
uint8_t inDebounceCount1 = 0;//inhibitory 1, which is K2, pin 9, PA4, and PCINT4.
uint8_t inDebounceCount2 = 0;//inhibitory 2, which is K4, pin 11, PA2, and PCINT2.
uint8_t inDebounceCount3 = 0;//inhibitory 3, which is K6, pin 13, PA0, and PCINT0.

int16_t potentialTotal = 0;
int16_t inGroundState = 0;
int16_t inGroundStatePrevious = 0;
int16_t exGroundState = 0;
int16_t exGroundStatePrevious = 0;
int16_t decayPotential = 0;

//decay timer variables: overflow value controls the decay rate.
//rate = (chip clock speed) / (timer1Overflow * timer2Overflow * potentialTimerOverflow)
uint8_t potentialTimerOverflow = 5; 
uint8_t potentialTimerCounter = 0;

//fast timer variables: set timer1Overflow to execute infrequently enough that it
//doesn't cause timing issues elsewhere (due to the amount of code in the
//timer1Overflow loop). timer2Overflow defines the PWM resolution. PWM freq
//is (chip clock speed) / (timer1Overflow * timer2Overflow).

//[6 changes--last one is changing timer2Overflow from 800 to 1600; PWM out to 17.5 ms again]

uint16_t timer1Overflow = 10;//FAST loop overflow
uint16_t timer2Overflow = 1600;//SLOW loop overflow
uint16_t timer2Counter = 0;

uint8_t inputMagnitude = 10;//amount each input increases/decreases potential
uint8_t inputStatus = 0b00000000;//current input status (read when stuff changes!)

uint8_t fireTimerOverflow = 2;//how long should LED pulses last?
uint8_t fireTimerCounter = 2;
uint8_t fireDelayOverflow = 20;//after firing, how long until sending a pulse?
uint8_t fireDelayCounter = 0;

ISR(PCINT0_vect) { 	//interrupt svc routine called when PCINT0 changes state
			//note that this is different than the ATtiny45 version
	inputStatus = PINA;
}

//This function updates the LEDs based on their fade values (i.e. duty cycles).
//PWM frequency equals the speed of the SLOW loop. PWM resolution is the
//value of timer2Overflow.
void updateLEDs(uint16_t Counter, uint16_t Red) {
/*[5 changes across two sections (getting rid of green/blue entirely)] 8.75ms!!!!
	if (Counter >= Green){
		PORTB |= (1<<0);//wahoo, bitwise logic! turns
				//the green LED on (low) 
				//for the first part of the
				//PWM waveform
	}
	else {
		PORTB &= ~(1<<0);//more bitwise logic. turns
				//the green LED off (high)
				//for the second part of the
				//PWM waveform.
	}
*/
	if (Counter >= Red){
		PORTA |= (1<<6);
		PORTB &= ~(1<<0);
		PORTB &= ~(1<<1);//see above. this could
				//probably be simplified into
				//a swanky function of some
				//type.
	}
	else {
		PORTA &= ~(1<<6);
		PORTB |= (1<<0);
		PORTB |= (1<<1);
	}
/*
	if (Counter >= Blue){
		PORTB |= (1<<2);
	}
	else {
		PORTB &= ~(1<<2);
	}
*/
}


void SystemInit(void) {
	DDRA = 0b01000000; //IO config: PA0-5 in (dendrites), PA6 out (axon)
	PORTA = 0b00000000; //Turns off pull-up resistors on dendrites, sets axon low
	DDRB = 0b00000111; //IO config: PB0,1,2 out (LEDs), all others in
	PORTB = 0b00000111; //Sets PB0,1,2 high to start (LEDs off).
	TCCR1B = 0b00000001; // sets up a timer at 1MHz (or base AVR speed)
	sei(); //enable all interrupts: same as changing bit 7 of SREG to 1
	GIMSK |= (1<<4);//sets the general input mask register's 4th bit to 1
			//to activate the PCIE0 interrupt stuff
	PCMSK0 = 0b00111111;	//sets the six dendrites to active hardware interrupts (0-5)
	inputStatus = PINA;
	exDebounceCount1 = debounceValue;
	exDebounceCount2 = debounceValue;
	exDebounceCount3 = debounceValue;
	inDebounceCount1 = debounceValue;
	inDebounceCount2 = debounceValue;
	inDebounceCount3 = debounceValue;
}

int main(void) {
	SystemInit();
	for(;;) {
		if (TCNT1 >= timer1Overflow) {//FAST 100 kHz loop
			if (timer2Counter >= timer2Overflow) {//SLOW 1 kHz loop			
//updates the current magnitude of high excitatory inputs to exGroundState. also stores the
//previous magnitude of excitatory inputs that are active to exGroundStatePrevious.
				exGroundStatePrevious = exGroundState;
				exGroundState = 0;
				if (((inputStatus & 0b00100000) > 0) & (exDebounceCount1 == debounceValue)) {
					exGroundState += inputMagnitude;
				}
				if (((inputStatus & 0b00001000) > 0) & (exDebounceCount2 == debounceValue)) {
					exGroundState += inputMagnitude * 2;
				}
				if (((inputStatus & 0b00000010) > 0) & (exDebounceCount3 == debounceValue)) {
					exGroundState += inputMagnitude * 4;
				}

//updates the current magnitude of high inhibitory inputs to inGroundState. also stores the
//previous magnitude of inhibitory inputs that are active to inGroundStatePrevious.
				inGroundStatePrevious = inGroundState;
				inGroundState = 0;
				if (((inputStatus & 0b00000001) > 0) & (inDebounceCount1 == debounceValue)) {
					inGroundState -= inputMagnitude * 4;
				}
				if (((inputStatus & 0b00000100) > 0) & (inDebounceCount2 == debounceValue)) {
					inGroundState -= inputMagnitude * 2;
				}
				if (((inputStatus & 0b00010000) > 0) & (inDebounceCount3 == debounceValue)) {
					inGroundState -= inputMagnitude;
				}

//checks to see if an excitatory input has moved from high to low, and if so, adds the magnitude
//of the reduction from exGroundState to decayPotential so it can start fading.
				if (exGroundState < exGroundStatePrevious) {
					decayPotential += exGroundStatePrevious - exGroundState;
				}

//checks to see if an inhibitory input has moved from high to low, and ijf so, adds the magnitude
//of the reduction from inGroundState to decayPotential so it can start fading.
				if (inGroundState > inGroundStatePrevious) {
					decayPotential -= inGroundState - inGroundStatePrevious; 
				}

//drives the decayPotential portion of the overall potential towards zero.
				if (potentialTimerCounter >= potentialTimerOverflow) {
					decayPotential = (decayPotential * 95) / 100;
				}

//these statements update the debounce loop, then hold it at the debounce threshold once
//reached (to prevent them from overflowing). a good candiate for code consolidation...
				if (exDebounceCount1 < debounceValue) {
					exDebounceCount1++;
				}
				if (exDebounceCount2 < debounceValue) {
					exDebounceCount2++;
				}
				if (exDebounceCount3 < debounceValue) {
					exDebounceCount3++;
				}
				if (inDebounceCount1 < debounceValue) {
					inDebounceCount1++;
				}
				if (inDebounceCount2 < debounceValue) {
					inDebounceCount2++;
				}
				if (inDebounceCount3 < debounceValue) {
					inDebounceCount3++;
				}
/*[2 changes] Now 14.00 ms!			
				if (fireTimerCounter < fireTimerOverflow) {
					fireTimerCounter++;
				}
				if (fireDelayCounter < fireDelayOverflow) {
					fireDelayCounter++;
				}
*/	
				potentialTimerCounter++;
				timer2Counter = 0;//reset SLOW loop
			}

			potentialTotal = decayPotential + inGroundState + exGroundState;
			
/*[4 changes] 10.80 ms 
			if (potentialTotal >= 100) {
				fireTimerCounter = 0;
				decayPotential -= inputMagnitude * 4;
			}
*/
			ledRedfade = 1470 - potentialTotal;
/*[1 change] Got rid of this section and the PWM waveform changed to 14.25 ms total length! Suspicious...
//This section converts the current potential value (signed variable called 'potential') into 
//red, green, and blue fade values for the fast PWM loop.
			if ((potentialTotal == 0) & (fireTimerCounter == fireTimerOverflow)) {
				ledRedfade = 733;//modified to calibrate 1.5ms (servo zero); 733 = 1.5ms
				ledGreenfade = 100;
				ledBluefade = 0;
			}
			if ((potentialTotal > 0) & (fireTimerCounter == fireTimerOverflow)) {
				ledRedfade =  733 - potentialTotal;
				ledGreenfade = 100 - potentialTotal;
				ledBluefade = 0;
			}
			if ((potentialTotal < 0) & (potentialTotal >= -100) & (fireTimerCounter == fireTimerOverflow)) {
				ledRedfade = 733 - potentialTotal;
				ledGreenfade = 100 - (-potentialTotal);
				ledBluefade = -potentialTotal;
			}
			if ((potentialTotal < -100) & (fireTimerCounter == fireTimerOverflow)){
				ledRedfade = 0;
				ledGreenfade = 0;
				ledBluefade = 100;
			}
*/
//[0 changes] PWM waveform: 17.5 ms total length. LED fade between 0 and 800.

/*[3 changes] 11.2 ms!!! Holy crap! 
			if (fireTimerCounter < fireTimerOverflow) {
				ledRedfade = 800;
				ledGreenfade = 800;
				ledBluefade = 800;
				fireDelayCounter = 0;
			}
			if ((fireDelayCounter < fireDelayOverflow) & (fireDelayCounter > fireDelayOverflow / 2)) {
				PORTA |= (1<<6);
			}
			if (fireDelayCounter == fireDelayOverflow) {
				PORTA &= ~(1<<6);
			}
*/
			updateLEDs(timer2Counter, ledRedfade);//, ledGreenfade, ledBluefade);
			timer2Counter++; //increment SLOW loop
			TCNT1 = 0; //reset FAST loop
		}
	}
}
