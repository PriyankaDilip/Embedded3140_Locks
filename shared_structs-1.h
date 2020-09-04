#ifndef __SHARED_STRUCTS_H__
#define __SHARED_STRUCTS_H__

#include <stdbool.h>
#include "3140_concur.h" 

/** Implement your structs here */

/**
 * This structure holds the process structure information
 */

typedef struct process_state {
	
	unsigned int * sp; 							// The stack pointer
	unsigned int * sp_orig; 				// The original stack pointer
	int n; 													// The size of the stack frame
	struct process_state * next; 		// The next process
	struct process_state * prev; 		// The previous process
	bool blocked;										// Indicated whether process is blocked

} process_t;


/**
 * This defines the lock structure
 */
typedef struct lock_state {
	
	process_t * blocked_head; 		// The head of the blocked queue
	process_t * blocked_tail; 		// The tail of the blocked queue
	bool locked;									// Indicates whether lock is in use

} lock_t;

/**
 * This defines the conditional variable structure
 */
typedef struct cond_var {
	
	process_t * cond_head; 		// The head of the condition waiting queue
	process_t * cond_tail; 		// The tail of the condition waiting queue
	lock_t * lock;						// The associated lock
	
} cond_t;

/* Enqueue a process into a given queue */
void enqueue(process_t ** proc, process_t ** head, process_t ** tail);

/* Dequeue a process from a given queue */
process_t * dequeue(process_t ** head, process_t ** tail);

/* Block a process by adding it to the blocked queue */
void block_process(lock_t * l);

/* Unblock a process by adding it to the ready queue */
void unblock_process(lock_t * l);

#endif
