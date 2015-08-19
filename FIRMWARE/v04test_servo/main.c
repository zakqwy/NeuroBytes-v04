/*
 * neuron-v04-servo
 *
 * Created: 7/20/2015
 *  Author: Zach Fredin
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>



ISR(TIM1_COMPA_vect) { //interrupt svc routine called when Timer 1 matches Compare
	PORTB = 0b00000000;
	PORTA = 0b00000000;
}


ISR(TIM1_COMPB_vect) { //End of fast clock cycle: turn all LEDs ON
	PORTB = 0b00000111;
	PORTA = 0b00000100;
	TCNT1 = 0;
	OCR1A += 1;
	if(OCR1A == 964 * 2) {
		OCR1A = 964;
	}
}
void SystemInit(void) {
	DDRA = 0b01000100; //IO config: PA0,1,3,4,5 in (dendrites), PA6 out (axon), PA2 = (servo, K4, bottom center Dendrite terminal)
	PORTA = 0b00000000; //Turns off pull-up resistors on dendrites, sets axon low
	DDRB = 0b00000111; //IO config: PB0,1,2 out (LEDs), all others in
	PORTB = 0b00000111; //Sets PB0,1,2 high to start (LEDs off).
	TCCR1B = 0b00000001; // sets up a timer at 8MHz (or base AVR speed)
	//TCCR1B = 0b00000010; //sets up a timer at 1MHz (or base AVR speed / 8)
	sei(); //enable all interrupts: same as changing bit 7 of SREG to 1
	GIMSK |= (1<<4);//sets the general input mask register's 4th bit to 1 to activate the PCIE0 interrupt stuff
	TIMSK1 = 0b00000110; // activates OCIE1A and OCIE1B: output compare match A and B 
	OCR1A = 964;
	OCR1B = 964*5;
}

int main(void)
{
	SystemInit();
	for(;;){
	}
}