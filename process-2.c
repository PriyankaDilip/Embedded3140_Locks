
#include "3140_concur.h" 
#include "shared_structs.h"
#include <stdlib.h>
#include <MK64F12.h>

/* the currently running process. current_process must be NULL if no process is running,
    otherwise it must point to the process_t of the currently running process
*/
process_t * current_process = NULL;
process_t * process_queue = NULL;				// The head of the ready queue
process_t * process_tail = NULL;				// The tail of the ready queue
int blocked_counter;										// The total number of blobked processes

/* Enqueue a process into a given queue */
void enqueue(process_t ** proc, process_t ** head, process_t ** tail) {
	if (!(* head)) {
		* head = * proc;
	}
	if (* tail) {
		(* tail) -> next = * proc;
	}
	* tail = * proc;
	(* proc) -> next = NULL;
}

/* Dequeue a process from a given queue */
process_t * dequeue(process_t ** head, process_t ** tail) {
	if (!(* head)) return NULL;
	process_t *proc = * head;
	* head = proc -> next;
	if (* tail == proc) {
		* tail = NULL;
	}
	proc -> next = NULL;
	return proc;
}

/* Deallocate memory for a given process */
static void process_free(process_t * proc) {
	process_stack_free(proc -> sp_orig, proc -> n);
	free(proc);
}

/* Create a new process */
int process_create (void (*f)(void), int n) {
	unsigned int *sp = process_stack_init(f, n);
	if (!sp) return -1;
	
	process_t *proc = (process_t*) malloc(sizeof(process_t));
	if (!proc) {
		process_stack_free(sp, n);
		return -1;
	}
	
	proc -> sp = proc -> sp_orig = sp;
	proc -> n = n;
	
	enqueue(&proc, &process_queue, &process_tail);
	return 0;
}

/* Starts up the concurrent execution */
void process_start (void) {
	SIM->SCGC6 |= SIM_SCGC6_PIT_MASK;
	PIT->MCR = 0;
	PIT -> CHANNEL[0].LDVAL = 0x00023000;
	NVIC_EnableIRQ(PIT0_IRQn);
	// Don't enable the timer yet. The scheduler will do so itself
	
	// Initialize number of blocked processes to 0
	blocked_counter = 0;
	
	// Bail out fast if no processes were ever created
	if (!process_queue) return;
	process_begin();
}

/* Called by the runtime system to select another process.
   "cursp" = the stack pointer for the currently running process
*/
unsigned int * process_select (unsigned int * cursp) {

	if (cursp) {
		// Suspending a process which has not yet finished, save state and make it the tail
		current_process -> sp = cursp;
		if(current_process -> blocked == 0) { // Only enqueue a process into the ready queue if it is not blocked
			enqueue(&current_process, &process_queue, &process_tail);
		}
	} else {
		// Check if a process was running, free its resources if one just finished
		if (current_process) {
			process_free(current_process);
		}
	}
	
	// Select the new current process from the front of the queue
	current_process = dequeue(&process_queue, &process_tail);
	
	if (current_process) {
		// Launch the process which was just popped off the queue
		return current_process -> sp;
	} else if (blocked_counter > 0) {
		while(1);
	} else {
		// No process was selected, exit the scheduler
		return NULL;
	}
}
