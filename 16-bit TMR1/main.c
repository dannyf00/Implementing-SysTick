//using timer1 to generate systick

#include "config.h"							//configuration words
#include "gpio.h"                           //we use gpio functions
#include "delay.h"                          //we use software delays

//hardware configuration
#define LED_PORT		GPIO
#define LED_DDR			TRISIO
#define LED				(1<<0)				//pin to flip
#define LED_DLY			(F_CPU / 10)		//cycles between each pin flip
//end hardware configuration

//global defines
//TMR1 prescaler
#define TMR1PS_1		(0 << 4)
#define TMR1PS_2		(1 << 4)
#define TMR1PS_4		(2 << 4)
#define TMR1PS_8		(3 << 4)

#define systick()		(systick_ovr | TMR1)	//return systick counts. needs to read TMR1L first

//global variables
volatile uint32_t systick_ovr;				//tick counter's MSB / overflow counter

//isr
void interrupt isr(void) {
	//for tmr1 overflow / tick generator
	if (TMR1IF) {
		TMR1IF = 0;							//clear the flag
		systick_ovr+=0x10000ul;				//advance the overflow counter
	}	
}

//initialize the tick generator
void systick_init(uint8_t ps) {
	systick_ovr=0;							//reset the tick counter

	//set up timer0
	TMR1ON = 0;								//0->stop the timer, 1->start the timer
	T1OSCEN=0;								//0->disable LP oscillator
	TMR1 = 0;								//reset the tmr
	TMR1CS = 0;								//0->use F_OSC/4, 1-> external clock on T0CKI
	TMR1GE = 0;								//0->disable the gate
	T1CON = (T1CON &~TMR1PS_8) | (ps & TMR1PS_8);
	//00->1:1, 01->2:1, 10->4:1, 11->8:1
	
	//enable timer0 interrupt
	TMR1IF = 0;								//clear the flag
	TMR1IE = 1;								//1->enable the interrupt
	
	//enable peripheral interrupt
	PEIE = 1;
	
	//enable tmr1
	TMR1ON = 1;								//0->stop the timer, 1->start the timer
}

	
int main(void) {
	uint32_t cnt;							//match point
		
	mcu_init();							    //initialize the mcu
	//led as output, active low
	IO_CLR(LED_PORT, LED); IO_OUT(LED_DDR, LED);
	ei();

	//initialize the tick generator
	systick_init(TMR1PS_1);					//initialize the tick
	
	cnt=systick();							//initialize counter
	while (1) {
		if ( systick() > cnt + LED_DLY) {	//enough elapsed?
			cnt+= LED_DLY;					//advance to the next match point
			IO_FLP(LED_PORT, LED);			//flip the pin
		}	
	}
}
