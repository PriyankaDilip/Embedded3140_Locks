
//#include "cond.h"
#include "lock.h"
#include "3140_concur.h" 
#include "shared_structs.h"

/**
 * Initialises the lock structure
 *
 * @param l pointer to lock to be initialised
 */
void l_init(lock_t * l) {
	l -> locked = 0;									// Initialize lock variable to default value (0, unlocked)
  l -> blocked_head = NULL;					// Blocked queue head initialized to null
	l -> blocked_tail = NULL;					// Blocked queue tail initialized to null
}

/**
 * Grab the lock or block the process until lock is available
 *
 * @param l pointer to lock to be grabbed
 */
void l_lock(lock_t * l) {
	PIT->CHANNEL[0].TCTRL = 1;											// Disable interrupts
	
	if(l -> locked) {																// If the lock is high...
		block_process(l);															// Send to blocked queue
	} else {
		l -> locked = 1;															// Set lock variable high
	}
	
	PIT->CHANNEL[0].TCTRL = 3;											// Enable interrupts
}

/**
 * Release the lock along with the first process that may be waiting on
 * the lock. This ensures fairness wrt lock acquisition.
 *
 * @param l pointer to lock to be unlocked
 */
void l_unlock(lock_t * l) {
	PIT->CHANNEL[0].TCTRL = 1;											// Disable interrupts
	
	if(l -> blocked_head == NULL) { 								// There are no blocked processes...
		l -> locked = 0;															// Set lock variable low
	} else {
		unblock_process(l); 													// Set this process' blocked status in the PCB to false
	}

	PIT->CHANNEL[0].TCTRL = 3;											// Enable interrupts
}

/* Block a process by adding it to the blocked queue */
void block_process(lock_t * l) {
	blocked_counter++;																											// Increment the blocked process counter
	current_process -> blocked = 1;																					// Mark process as blocked in PCB
	enqueue(&current_process, &(l -> blocked_head), &(l -> blocked_tail));	// Add this process to the blocked queue for this lock
	process_blocked();																											// Call the scheduler
}

/* Unblock a process by adding it to the ready queue */
void unblock_process(lock_t * l) {
	process_t * process_move = dequeue(&(l -> blocked_head), &(l -> blocked_tail));		// Dequeue the head of the blocked queue for this lock
	if(process_move) {																						// If a process was dequeued...
		process_move -> blocked = 0;																// Mark process as unblocked in PCB
		enqueue(&process_move, &process_queue, &process_tail); 		 	// Enqueue this process into the ready queue
		blocked_counter--;																				  // Decrement the blocked process counter
	}
}
