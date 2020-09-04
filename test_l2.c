#include "3140_concur.h"
#include "utils.h"
#include "lock.h"

/* One process (b1) never gives up its lock. Processes r1 and r2 execute unaffected but b2 is 
*  suspended indefinitely. This error is caught by process_select, meaning the scheduler enters
*  an error case instead of exiting gracefully. Test case shows that processes using different
*  lock variables operate independently and demonstrates error checking for processes which
*  block other processes. Without the check, the scheduler would exit even though b2 never
*  executes because there would be no more processes in the ready queue.
*
*  Expected behavior: red + blue (purple) then red then OFF with NO GREEN LIGHT at the end if
*  the blocked process is identified by process_select. See lab report for diagram of expected
*  behavior.
*/

lock_t r;
lock_t b;

void r1(void){
	/*NCS*/
	//delay();
	/*CS*/
	l_lock(&r);
	delay();
	LEDRed_Toggle();
	delay();
	LEDRed_Toggle();
	delay();
	l_unlock(&r);
}

void r2(void){
	/*NCS*/
	//delay();
	/*CS*/
	l_lock(&r);
	delay();
	LEDRed_Toggle();
	delay();
	LEDRed_Toggle();
	delay();
	l_unlock(&r);
}

void b1(void){
	/*NCS*/
	//delay();
	/*CS*/
	l_lock(&b);
	delay();
	LEDBlue_Toggle();
	delay();
	LEDBlue_Toggle();
	delay();
	//l_unlock(&b);			-->			// B1 never releases its lock!!!
}

void b2(void){								// B2 can never execute because b1 retains lock b
	/*NCS*/
	//delay();
	/*CS*/
	l_lock(&b);
	delay();
	LEDBlue_Toggle();
	delay();
	LEDBlue_Toggle();
	delay();
	l_unlock(&b);
}

int main(void){
	LED_Initialize();           /* Initialize the LEDs           */	

	l_init (&r);
	l_init (&b);
	
	if (process_create (r1,20) < 0) {
	 	return -1;
	}
	if (process_create (r2,20) < 0) {
	 	return -1;
	}
	if (process_create (b1,20) < 0) {
	 	return -1;
	}
	if (process_create (b2,20) < 0) {
	 	return -1;
	}

	
	process_start();
	LEDGreen_On();							// Illuminate the green LED IF AND ONLY IF the scheduler exits without error

	while(1);
	return 0;	
}

