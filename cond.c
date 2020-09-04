
#include "cond.h"
//#include "lock.h"
#include "3140_concur.h" 
#include "shared_structs.h"

/**
 * Initialises the conditional variable structure
 *
 * @param l ignored
 * @param c pointer to conditional variable to be initialised
 */
void c_init(lock_t* l, cond_t* c) {
	c -> lock = l;									// Lock variable is l
	c -> cond_head = NULL;					// Condition waiting queue head initialized to null
	c -> cond_tail = NULL;					// Condition waiting queue tail initialized to null
}

/**
 * wait until condition is true
 *
 * @param l pointer to lock/mutex for conditional variable
 * @param c pointer to conditional variable
 */
void c_wait(lock_t* l, cond_t* c) {
	PIT->CHANNEL[0].TCTRL = 1;																				// Disable interrupts

	enqueue(&current_process, &(c -> cond_head), &(c -> cond_tail));	// Add this process to the blocked queue for this lock
	l -> locked = 0;																									// Set the lock variable low
	current_process -> blocked = 1;																		// Mark process as blocked in PCB
	process_blocked();																								// Call the scheduler

	PIT->CHANNEL[0].TCTRL = 3;																				// Enable interrupts
}

/**
 * Check if processes are waiting on conditional variable
 *
 * @param l Pointer to lock/mutex. Not used, but mutex must be acquired
 * before calling to ensure atomicity
 * @param c pointer to conditional variable
 *
 * @return 0 if no processes waiting
 * @return 1 if processes waiting
 */
bool c_waiting(lock_t* l, cond_t* c) {
	if(c -> cond_head == NULL) { 					// If there are no waiting processes...
		return false;
	} else {
		return true;
	}
}

/**
 * Signal that condition is met
 *
 * @warning Will misbehave if no processes are waiting and is signalled
 * 
 * @param l pointer to lock (is released after signalling)
 * @param c pointer to conditional variable
 */
void c_signal(lock_t* l, cond_t* c) {
	PIT->CHANNEL[0].TCTRL = 1;																										// Disable interrupts
	
	if(c -> cond_head == NULL) { 																									// If there are no waiting processes...
		l -> locked = 0;																														// Set the lock variable low
	} else {
		process_t * process_move = dequeue(&(c -> cond_head), &(c -> cond_tail));		// Dequeue the head of the condition waiting queue for this lock
		if(process_move) {																													// If a process was dequeued...
			process_move -> blocked = 0;																							// Mark process as unblocked in PCB
			enqueue(&process_move, &process_queue, &process_tail); 		 								// Enqueue this process into the ready queue
			l -> locked = 0;																													// Set the lock variable low
		}
	}
	
	PIT->CHANNEL[0].TCTRL = 3;																										// Enable interrupts	
}


