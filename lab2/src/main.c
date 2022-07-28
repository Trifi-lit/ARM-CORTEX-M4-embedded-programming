#include <platform.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <uart.h>
#include <stdbool.h>
#include <gpio.h>
#include "leds.h"
#include "queue.h"
#include "timer.h"
#include "delay.h"

Queue rx_queue; //queue to handle uart receiving characters
int lightsOn=0; 
bool blue=false;
bool vowel;  // boolean for vowels comparison 

// ISR for character receive (receiving char is in argument)
void uart_rx_isr(unsigned char rx) {
	if ((rx >= 'a' && rx <= 'z') || (rx >= 'A' && rx <= 'Z') || rx==0x7F || rx==0x0A ) {
		   queue_enqueue(&rx_queue, rx); // Store the received character
	}
}

// ISR for red&green leds using a timer
void timer_callback_isr(void) {
	if (lightsOn){
		if(vowel)
			leds_set(1,0,0); //Red light RGB values=(Red bit,Green Bit,BlueBit)
		else
			leds_set(0,1,0); //Green light RGB values=(Red bit,Green Bit,BlueBit)
		lightsOn=0;
	}
}

// ISR for button press
void button_press_isr(){
  if(!blue){
		blue=true;
	  leds_set(0,0,1);   // If button is pressed , light LED blue
	}
	else { 
		leds_set(0,0,0);  // Else if LED is already blue, deactivate LED
    blue=false;		
	}
	  //Blue light RGB values=(Red bit,Green Bit,BlueBit)	
}
int main(){
  
	queue_init(&rx_queue, 128);
	uart_init(9600); 	//Initialize UART protocol
	uart_set_rx_callback(uart_rx_isr);  // set callback receiving ISR function 
	uart_enable();
	leds_init(); 	//Initialize LEDs
	
	timer_init(CLK_FREQ / 100);
	timer_set_callback(timer_callback_isr);
	timer_enable();


	__enable_irq(); 	// Initialize Interrupt Service Routines
	
	gpio_set_mode(P_SW, PullUp);
  gpio_set_callback(P_SW, button_press_isr);
	gpio_set_trigger(P_SW, Rising);
	
	char c;
	int surname_index;
	unsigned char rx_char;
	
	
	uart_print("\r");
	
	while(1) {
    delay_ms(1000); //wait one second to ask for surname again
		uart_print("Enter your surname and press enter: ");
		surname_index=0;
		while(1) {
			while (!queue_dequeue(&rx_queue, &rx_char)) //queue_dequeue returns 0 if queue is empty
				__WFI(); 
			if (rx_char == 0x7F) { // Handle backspace character
				if (surname_index > 0) { //avoid empty user input
					surname_index--;
					uart_tx(rx_char);
				}
			} 
			else if (rx_char==0x0A){ //user presses enter 
				lightsOn=1;
				if(vowel==1)
						uart_tx('R'); 	//Light red LED  
				else   
						uart_tx('G');	//Light green LED
				break;
			}
			else{
				surname_index++;
				c=rx_char;  // Store the last received character back 1 position before 0x0A ASCII value 
				vowel = (c == 'a' || c == 'e' || c == 'i' || c == 'o' || c == 'u'|| c == 'A' || c == 'E' || c == 'I' || c == 'O' || c == 'U');
			}
		} 
	}
}



