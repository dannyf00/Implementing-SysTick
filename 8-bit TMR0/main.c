//using timer0 to generate systick

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
//TMR0 prescaler
#define TMR0PS_2		0
#define TMR0PS_4		1
#define TMR0PS_8		2
#define TMR0PS_16		3
#define TMR0PS_32		4
#define TMR0PS_64		5
#define TMR0PS_128		6
#define TMR0PS_256		7

#define systick()		(systick_ovr | TMR0)	//return systick counts

//global variables
volatile uint32_t systick_ovr;				//tick counter's MSB / overflow counter

//isr
void interrupt isr(void) {
	//for tmr0 overflow / tick generator
	if (TMR0IF) {
		TMR0IF = 0;							//clear the flag
		systick_ovr+=0x100;					//advance the overflow counter
	}	
}

//initialize the tick generator
void systick_init(uint8_t ps) {
	systick_ovr=0;							//reset the tick counter

	//set up timer0
	TMR0 = 0;								//reset the tmr
	TMR0IF = 0;								//clear the flag
	T0CS = 0;								//0->use F_OSC/4, 1-> external clock on T0CKI
	PSA = 0;								//0->prescaler for tmr0; 1->prescaler to watchdog
	OPTION_REG = (OPTION_REG &~TMR0PS_256) | (ps & TMR0PS_256);	//set the prescaler
	//000->2:1, 001->4:1, 010->8:1, ..., 111->256:1
	
	//enable timer0 interrupt
	TMR0IE = 1;								//1->enable the interrupt
}

	
int main(void) {
	uint32_t cnt;							//match point
		
	mcu_init();							    //initialize the mcu
	//led as output, active low
	IO_CLR(LED_PORT, LED); IO_OUT(LED_DDR, LED);
	ei();

	//initialize the tick generator
	systick_init(TMR0PS_4);					//initialize the tick, 0x01=4:1
	
	cnt=systick();							//initialize counter
	while (1) {
		if ( systick() > cnt + LED_DLY) {	//enough elapsed?
			cnt+= LED_DLY;					//advance to the next match point
			IO_FLP(LED_PORT, LED);			//flip the pin
		}	
	}
}
