
#include "3140_concur.h"
#include "utils.h"
#include "lock.h"
#include "cond.h"

/* Two different reader functions use separate condition variables but readers
*  must still exclude each other. Two separate "number of readers" variables
*  are updated and a given reader will only signal waiting readers of its type.
*  A reader will wait if a reader of another type is reading. Writer must check
*  for the presence of both types of readers and exclude both of them. Two
*  separate waiting queues must be maintained but a single blocked queue is
*  maintained.
*
*  Expected behavior: red twice then green four times then blue four times
*  then green forever. See lab report for diagram of expected behavior.
*/

lock_t l;
cond_t cr1;
cond_t cr2;
cond_t cw;

unsigned int nr1= 0;
unsigned int nr2= 0;
unsigned int nw= 0;

void p1 (){										// Reader1 flashes green
	int i;
	for (i=0; i < 8; i++) {
		LEDGreen_Toggle();
		delay();
	}
}

void p2 (){										// Reader2 flashes blue
	int i;
	for (i=0; i < 8; i++) {
		LEDBlue_Toggle();
		delay();
	}
}

void reader1 (void){	
	l_lock(&l);
	
	if(nw!=0 || nr2 != 0){
		c_wait(&l,&cr1);
	}
	nr1++;
	if(c_waiting(&l,&cr1)){								// Check for process waiting on cr1
		c_signal(&l,&cr1);
	}
	else{
		l_unlock(&l);
	}
	
	/*start reading*/
	p1();
	/*end reading*/
	
	l_lock(&l);
	
	nr1--;
  	if(c_waiting(&l,&cw) && nr1 == 0 && nr2 == 0){
  		c_signal(&l,&cw);
  	}
  	else if(c_waiting(&l,&cr1)){				// Check for process waiting on cr1
  		c_signal(&l,&cr1);
  	}
		else if(c_waiting(&l,&cr2)){				// Check for process waiting on cr2
  		c_signal(&l,&cr2);
  	}
  	else{
  		l_unlock(&l);			
  	}
}

void reader2 (void){	
	l_lock(&l);
	
	if(nw!=0 || nr1 != 0){
		c_wait(&l,&cr2);
	}
	nr2++;
	if(c_waiting(&l,&cr2)){								// Check for process waiting on cr1
		c_signal(&l,&cr2);
	}
	else{
		l_unlock(&l);
	}
	
	/*start reading*/
	p2();
	/*end reading*/
	
	l_lock(&l);
	
	nr2--;
  	if(c_waiting(&l,&cw) && nr1 == 0 && nr2 == 0){
  		c_signal(&l,&cw);
  	}
  	else if(c_waiting(&l,&cr2)){				// Check for process waiting on cr1
  		c_signal(&l,&cr2);
  	}
		else if(c_waiting(&l,&cr1)){				// Check for process waiting on cr2
  		c_signal(&l,&cr1);
  	}
  	else{
  		l_unlock(&l);			
  	}
}

void writer (void) {
	l_lock(&l);
	
	if(nw!=0 || nr1!=0 || nr2 != 0){
		c_wait(&l,&cw);
	}
	nw++;
	
	l_unlock(&l);
	
	/*start writing*/
	LEDRed_Toggle();
	
	delay();
	
	LEDRed_Toggle();
	delay();
	
	l_lock(&l);
	
	nw--;
  	if(c_waiting(&l,&cw) && nr1 == 0 && nr2 == 0){
  		c_signal(&l,&cw);
  	}
  	else if(c_waiting(&l,&cr1) && nr2 == 0){				// Check for process waiting on cr1
  		c_signal(&l,&cr1);
  	}
  	else if(c_waiting(&l,&cr2) && nr1 == 0){				// Check for process waiting on cr2
  		c_signal(&l,&cr2);
  	}
  	else{
  		l_unlock(&l);		
  	}
}

int main (void){
	LED_Initialize();   
	
	l_init (&l);
	c_init (&l,&cr1);
	c_init (&l,&cr2);
	c_init (&l,&cw);
	
	if (process_create (writer,20) < 0) {
	 	return -1;
	}
	if (process_create (reader1,20) < 0) {
	 	return -1;
	}
	if (process_create (reader2,20) < 0) {
	 	return -1;
	}
	if (process_create (writer,20) < 0) {
	 	return -1;
	}

	process_start();
	LEDGreen_On();							// Illuminate the green LED IF AND ONLY IF the scheduler exits without error
	
	while (1) ;

	return 0;
}