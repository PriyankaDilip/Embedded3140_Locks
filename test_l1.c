#include "3140_concur.h"
#include "utils.h"
#include "lock.h"

/* Produces deadlock between p1 and p2 and verifies that program enters error case.
*  Nested locks produce the deadlock. The error is identified with the use of a
*  blocked_counter checked by process_select. If there are no more processes in the
*  ready queue but there are processes in the blocked queue, an error has occured
*  and the scheduler does not exit gracefully. Instead it enters an infinite while
*  loop representing an error case.
*
*  Expected behavior: red + green (yellow) then off with NO GREEN LIGHT at the end
*  if deadlock is caught by process_select. See lab report for diagram of expected
*  behavior.
*/


lock_t a;
lock_t b;

void p1(void){
	/*NCS*/
	LEDRed_Toggle();
	delay();
	/*CS*/
	l_lock(&a);							// Lock a first
	delay();
	LEDRed_Toggle();
	l_lock(&b);							// Lock b second
	LEDBlue_Toggle();
	delay();
	LEDBlue_Toggle();
	delay();
	l_unlock(&a);
	l_unlock(&b);
}

void p2(void){
	/*NCS*/
	LEDGreen_Toggle();
	delay();
	/*CS*/
	l_lock(&b);							// Lock b first
	delay();
	LEDGreen_Toggle();
	l_lock(&a);							// Lock a second
	LEDBlue_Toggle();
	delay();
	LEDBlue_Toggle();
	delay();
	l_unlock(&b);
	l_unlock(&a);
}

int main(void){
	LED_Initialize();           /* Initialize the LEDs           */	

	l_init (&a);
	l_init (&b);
	
	if (process_create (p1,20) < 0) {
	 	return -1;
	}
	if (process_create (p2,20) < 0) {
	 	return -1;
	}
	
	process_start();
	LEDGreen_On();							// Illuminate the green LED IF AND ONLY IF the scheduler exits without error

	while(1);
	return 0;	
}

