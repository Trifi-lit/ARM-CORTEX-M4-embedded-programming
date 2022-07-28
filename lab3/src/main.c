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
#define TIMEOUT 100000
#define pin PA_0

// INIT BUFFERVAR TO RECEIVE DATA
uint8_t idx = 0;
uint8_t counter=0;
uint8_t bits[5];
uint8_t T=2; //temperature sampling period
uint8_t buttonCnt=0; //counts button presses
int temp=0;

int temperature_read(){
	int dur=0; //stores the duration that sensor signal stays at high voltage
	int currentBit=7; //distance(in bits) of current (received) bit from MSB
	int temperature=-1;
	int sum=0;
	unsigned int loopCnt = TIMEOUT;
			// EMPTY BUFFER
	for (int i=0; i< 5; i++) {
		bits[i] = 0;
	}

	gpio_set_mode(pin, Output);
		// REQUEST SAMPLE
	gpio_set(pin, 0);
	delay_ms(20); //wait for DHT to detect start signal
	gpio_set(pin, 1);
	delay_us(40); //wait for DHT to respond

	gpio_set_mode(pin, Input); //turn mode to input to receive sensor signal

	
	while(gpio_get(pin) == 0)
		if (loopCnt-- == 0) return -2;

	loopCnt = TIMEOUT;
	while(gpio_get(pin) == 1)
		if (loopCnt-- == 0) return -2;

	// READ THE OUTPUT - 40 BITS => 5 BYTES
	for (int i=0; i<40; i++){
		  dur=0;
			loopCnt = TIMEOUT;
			while(gpio_get(pin) == 0)
				if (loopCnt-- == 0) return -2;
			
			loopCnt = TIMEOUT;
			while(gpio_get(pin) == 1){ //we assume that execution time of this while loop is neglectable
				delay_us(5);
				dur++;
				if (loopCnt-- == 0) return -2;
		  }
			dur=dur*5; //multiply by 5 to get microseconds
			
			if (dur > 45) bits[idx] |= (1 << currentBit); //use bitwise or to modify the current bit
			if (currentBit == 0)   //LSB reached, go to next byte
			{
				currentBit = 7;   
				idx++;      
			}
			else currentBit--;
	}
	
	sum = bits[0] + bits[1] + bits[2] + bits[3];
	if (bits[4] != sum) return -1;
	temperature = bits[2];
	return temperature;
}

void timer_callback_isr(){
	counter++;
	if (counter%2==0){
		temp=temperature_read();
		if(T==2){
			printf("temperature: %d",temp);
			printf("sampling period: 2s");
		}
	}
	if (T==3 && counter%3==0){
		printf("temperature: %d",temp);
		printf("sampling period: 2s");}
	else if (T==4 && counter%4==0){
		printf("temperature: %d",temp);
		printf("sampling period: 2s");}
	else if (T==8 && counter%8==0){
		printf("temperature: %d",temp);
		printf("sampling period: 2s");}
}

void button_press_isr(){  
  buttonCnt++;
  if (buttonCnt==1)
		T=8;
  else if(buttonCnt%2==0)
		T=4;
  else
		T=3;
}

int main(){
		
	 uart_init(9600); 	//Initialize UART protocol
	 uart_enable();
	
   timer_init(1000000);
   timer_set_callback(timer_callback_isr);
   timer_enable();
   
   gpio_set_callback(P_SW, button_press_isr);
   gpio_set_mode(P_SW, PullUp);
   gpio_set_trigger(P_SW, Rising);
	
	 __enable_irq();
	
	 while(1){
		if(temp<20)
			leds_set(0,0,1); //blue led on //uart_print("blue led on");
		else if(temp>25)
			leds_set(1,0,0); //red led on
		else
			leds_set(0,1,0); //green led on
	 } 
		
}