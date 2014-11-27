/*
    Copywrite 2014, Zach Fredin
    zachary.fredin@gmail.com
    Distributed under terms of the GNU General Public License, version 3
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
    along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <avr/io.h>
//#include <avr/interrupt.h>
/*uint8_t status;//input pin status for pushbutton interrupt
uint8_t pwmCount = 0;//current position in PWM timing loop
uint8_t pwmTotal = 100;//length of PWM timing loop

uint8_t ledBright1 = 50;
uint8_t ledBright2 = 50;
uint8_t ledBright3 = 50;
uint8_t update_interval = 255;

ISR(PCINT0_vect) { //interrupt svc routine called when PCINT0 changes state
  status = PINB;
}*/

void SystemInit(void) {
  DDRB = 0b11111111; //IO config: PB0,1,2 out, all others in
  PORTB = 0b00000000; //Sets PB0,1,2 low to start (lights LEDs).
//  PCMSK |= (1<<PCINT1); //listen to portB bit 2
//  PCMSK |= (2<<PCINT1); //listen to portB bit 3
//  GIMSK |= (1<<PCIE); //enable PCINT interrupt
//  TCCR1 |= (1 << CS10); //sets up timer equal to clock rate (1MHz)
//  sei(); //enable all interrupts
}

int main(void) {
  SystemInit();
  for(;;) {
//    updateLEDs(update_interval, ledBright1, ledBright2, ledBright3);
  }
}

/*void updateLEDs(uint8_t interval_uS, uint8_t led1, uint8_t led2, uint8_t led3) {
  if (TCNT1 >= interval_uS) {//executes every 10uS
    if (pwmCount == led1) {//toggles LEDs if necessary
      PORTB ^= (1 << 0);
    }
    if (pwmCount == led2) {
      PORTB ^= (1 << 1);
    }
    if (pwmCount == led3) {
      PORTB ^= (1 << 2);
    }
    pwmCount++;
    if (pwmCount == pwmTotal) { 
      PORTB ^= (1 << 0);//resets LEDs
      PORTB ^= (1 << 1);
      PORTB ^= (1 << 2);
      pwmCount = 0;
    }
    TCNT1 = 0;
  }
}
*/
