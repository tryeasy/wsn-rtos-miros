/**
 * @file multithreading_sched.c
 
 * @brief  multi-threading scheduler to schedule the MIROS real-time tasks.
 *
 *	All the RT tasks in the MIROS will be scheduled by the multi-threading scheduler.
 *	If common_thread is executing and any RT thread becomes active, 
 *	the common_thread will be preempted and the OS will switch 
 *	from the event-driven scheduling model to the multithreaded scheduling model. 
 *
 * @author	  Xing Liu  (http://edss.isima.fr/sites/smir/)
 * @author    LIMOS Laboratory - UMR CNRS 6158: http://edss.isima.fr
 * @author    Supported email: liu@isima.fr
 */

/* === INCLUDES ============================================================ */
#include "evt_driven_sched.h"
#include "multithreading_sched.h"
#include "kernel.h"
#include "kdebug.h"
#include "usart.h"

#if RT_SUPPORT
/* === TYPES =============================================================== */


/* === MACROS ============================================================== */


/* === GLOBALS ============================================================= */
/* thread control tables (TCB) to store the thread specific information,.
   Since the thread number in MIROS is small (only for RT tasks), 
   the TCB in MIROS is pre-reserved other than dynamically allocated. */
thrd_tcb_t thrd_TCB[MAX_THREAD_NUM], *thrd_lstQ= NULL;

/* === PROTOTYPES ========================================================== */



/* === IMPLEMENTATION ====================================================== */
/**
 * @brief created a thread.
 *
 * Create a new thread, establish the thread run-time context,
 * and then force the thread switch.
 *
 * \param thrd_tsk	  The RT task which will be executed by the thread.
 *
 */
thrd_tcb_t*
thread_create(tsk_handler_t thrd_tsk, uint16_t tsk_period)
{
	thrd_tcb_t *thrd = NULL;
	
	/* thread context creation. */
	thrd = thrd_contextPrep(thrd_tsk, tsk_period);
	/* force the thread switch. */
	thread_dispatcher();
	
	return thrd;
}

/**
 * @brief Create the thread context.
 *
 * Firstly, get a thread control table (TCB).
 * Then, allocate a stack for this thread.
 * Finally, initialize the thread stack.
 *
 * \param thrd_tsk	  The RT task which will be executed by the thread.
 */ 
thrd_tcb_t*
thrd_contextPrep(tsk_handler_t thrd_tsk, uint16_t tsk_period)
{
	HAS_CRITICAL_SECTION;
	uint8_t i, id;
	thrd_tcb_t *thrd, *thrd_prev;

	/* get a thread control table (TCB) for this thread. */
	for(id = 0; id < MAX_THREAD_NUM; id++)
		if(thrd_TCB[id].status == THRD_UNUSED)	break;
	if(id == MAX_THREAD_NUM)
	{
		/* maximum threads have been created. */
		sendUsartByte(USART_CHANNEL_1, 'M');
		return NULL;
	}

	/* allocate a thread run-time stack. 
	   Note that the stack is used from high address to low
	   thus, should move the stack pointer "thrd_sp" to the stack bottom. */
	heapSaddr += THREAD_CONTEXT_SIZE;
	thrd_TCB[id].thrd_sp = (uint8_t *)(heapSaddr - 1);
	/* init the thread TCB. */
	thrd_TCB[id].next = NULL;
	thrd_TCB[id].thrd_tsk = thrd_tsk;
	thrd_TCB[id].thrd_period = tsk_period;
	thrd_TCB[id].status = THRD_ACTIVE;
	
	/* add this thread into the thread queue in the order of the thread priority.
	   thread with the smallest "thrd_period" (highest priority) will be put at the queue header. */
	if(thrd_lstQ == NULL)	
		thrd_lstQ = &thrd_TCB[id];
	else
	{
		for(thrd_prev = thrd = thrd_lstQ; thrd != NULL; thrd = thrd->next)
		{
			/* insert new thread before "thrd". */
			if(thrd_TCB[id].thrd_period < thrd->thrd_period)
			{
				ENTER_CRITICAL_SECTION;
				/* insert to the head. */
				if(thrd == thrd_lstQ)
				{
					thrd_TCB[id].next = thrd_lstQ;
					thrd_lstQ = &thrd_TCB[id];
				}
				else
				{
					thrd_prev->next = &thrd_TCB[id];
					thrd_TCB[id].next = thrd;
				}
				LEAVE_CRITICAL_SECTION;
				break;
			}
			/* update "thrd_prev", the previous thread after which the new thread will be inserted. */
			thrd_prev = thrd;
		}
		
		/* insert to the tail. */
		if(thrd == NULL)
			thrd_prev->next = &thrd_TCB[id];
	}

	/* curThrd should not be NULL. */
	if(curThrd == NULL)
	{
		/* ERROR here. */
		sendUsartByte(USART_CHANNEL_1, 'E');
		return NULL;
	}

	/*
	* The sequence of the following part should 
	* correspond to that in "thrdContextRestore".
	*/
	asm volatile(					\
	"in %A0, __SP_L__\n\t"				\
	"in %B0, __SP_H__\n\t"				\
	: "=r" (curThrd->thrd_sp) : );		\
		/* save current thread's stack */
			
	asm volatile(					\
	"out __SP_H__, %B0\n\t"			\
	"out __SP_L__, %A0\n\t"			\
	:: "r" (thrd_TCB[id].thrd_sp) );	\
		/* switch to the new created thread's stack */
			
	asm volatile(					\
	"push %A0\n\t"					\
	"push %B0\n\t"					\
	:: "r" (thrd_start_wrapper) );			\
		/* Push address of "thrd_start_wrapper" onto the stack.
		   "thrd_start_wrapper" is the thread's entry function. 
		   After this operation, when the "RET" instruction is executed in "thread_dispatcher",
		   this function "thrd_start_wrapper" will be executed.
		   */
			
	for(i = 0; i < 33; i++)				\
		asm volatile("push __zero_reg__\n\t" ::);	\
		/* Reserve and init the registers. 
		   33 bytes: 32 for the registers and 1 for the sreg.
		   use "__zero_reg__" as the operation of initialization. */
			
	asm volatile(					\
	"in %A0, __SP_L__\n\t"				\
	"in %B0, __SP_H__\n\t"				\
	: "=r" (thrd_TCB[id].thrd_sp) : );			\
		/* save the new created thread's sp pointer into "thrd_sp",
		   it will be used in "thread_dispatcher". */
		
	asm volatile(					\
	"out __SP_H__, %B0\n\t"			\
	"out __SP_L__, %A0\n\t"			\
	:: "r" (curThrd->thrd_sp) );		\
		/* recover the current thread's stack. */
		
	return &thrd_TCB[id];
}


/**
 * @brief Start executing a thread.
 *
 * Call the related handlers for this thread.
 */
void 
thrd_start_wrapper(void)
{
	/* execute the current thread's handler. */
	curThrd->thrd_tsk();
	
	/* thread dispatcher, switch to execute the other threads. */
	thread_dispatcher();
}


/**
 * @brief	get the next thread to be scheduled in terms of the RMS algorithm.
 *
 * The MIROS event-driven scheduler is implemented as a thread named "common_thread".
 * If the "common_thread" is scheduled, the OS will switch back to event-driven scheduling model.
 *
 * \return    Return the next thread to be executed.
 */
thrd_tcb_t* 
getNextThread(void)
{
	thrd_tcb_t *thr;

	/* Implementation of the RMS scheduling algorithm here.
	   Threads are ordered in the thread queue in terms of the priorities (from highest priority to lowest priority).
	   When the thread switch is performed, the first active thread that is found in this queue will be the next one to be scheduled. */
	for(thr = thrd_lstQ; thr != NULL; thr = thr->next)
		if(thr->status == THRD_ACTIVE)	return thr;
	
	/* Switch the scheduler here.
	   If all the threads are inactive, return the common_thread. 
	   Then, MIROS will switch to the event-driven scheduling model. */
	return &common_thread;
}


/**
 * @brief Force the thread switch.
 *
 * Save the current thread's run-time context,
 * and then recover the next thread's context.
 *
 * At the end of this function, the "RET" instruction
 * will be called by default, this instruction will pop the next thread's
 * execution address to the "PC (program counter)", and then
 * the next thread will start execution. 
 */
void
thread_dispatcher(void)
{
	/* save current thread's context. */
	{
		thrdContextSave();
		asm volatile(				\
		"in %A0, __SP_L__\n\t"			\
		"in %B0, __SP_H__\n\t"			\
		: "=r" (curThrd->thrd_sp) : );	\
	}

	/* get the next thread to be scheduled in terms of the RMS scheduling algorithm,
	   and assign this thread to "curThrd". */
	curThrd = getNextThread();

	/* recover the context of the next thread. */
	{
		asm volatile(				\
			"out __SP_H__, %B0\n\t"		\
			"out __SP_L__, %A0\n\t"		\
			:: "r" (curThrd->thrd_sp));
		thrdContextRestore();
	}
	
	/* "RET" directive is executed here in default:
	 * This first time a thread is created, this "RET" will pop out 
	 * the address of "thrd_start_wrapper" to Program Counter, and
	 * then the execution of a thread can be processed.
	 */
}


/**
 * @brief Save the task context
 *
 * Save the task context data over the run-time stack
 * Save SREG
 * Save all the registers
 */
void
thrdContextSave(void)
{
    asm volatile(				\
    "push r24\n\t"				\
    "in r24, __SREG__\n\t"			\
    "cli\n\t"				\
    "push r24\n\t"				\
    );		/* save R24 and sreg */
	
    asm volatile(				\
    "push r31\n\t"				\
    "push r30\n\t"				\
    "push r29\n\t"				\
    "push r28\n\t"				\
    "push r27\n\t"				\
    "push r26\n\t"				\
    "push r25\n\t"				\
    "push r23\n\t"				\
    "push r22\n\t"				\
    "push r21\n\t"				\
    "push r20\n\t"				\
    "push r19\n\t"				\
    "push r18\n\t"				\
    "push r17\n\t"				\
    "push r16\n\t"				\
    "push r15\n\t"				\
    "push r14\n\t"				\
    "push r13\n\t"				\
    "push r12\n\t"				\
    "push r11\n\t"				\
    "push r10\n\t"				\
    "push r9\n\t"				\
    "push r8\n\t"				\
    "push r7\n\t"				\
    "push r6\n\t"				\
    "push r5\n\t"				\
    "push r4\n\t"				\
    "push r3\n\t"				\
    "push r2\n\t"				\
    "push r1\n\t"				\
    "push r0\n\t"				\
    );		/* save all registers */
}


/**
 * @brief Restore the task context
 *
 * the first time this function is called, 
 * it will pop out the initialized values of 
 * the registers that are assigned in "thrd_contextPrep".
 */
void
thrdContextRestore(void)
{
	/* restore the context data */
    asm volatile(				\
    "pop r0\n\t"				\
    "pop r1\n\t"				\
    "pop r2\n\t"				\
    "pop r3\n\t"				\
    "pop r4\n\t"				\
    "pop r5\n\t"				\
    "pop r6\n\t"				\
    "pop r7\n\t"				\
    "pop r8\n\t"				\
    "pop r9\n\t"				\
    "pop r10\n\t"				\
    "pop r11\n\t"				\
    "pop r12\n\t"				\
    "pop r13\n\t"				\
    "pop r14\n\t"				\
    "pop r15\n\t"				\
    "pop r16\n\t"				\
    "pop r17\n\t"				\
    "pop r18\n\t"				\
    "pop r19\n\t"				\
    "pop r20\n\t"				\
    "pop r21\n\t"				\
    "pop r22\n\t"				\
    "pop r23\n\t"				\
    "pop r25\n\t"				\
    "pop r26\n\t"				\
    "pop r27\n\t"				\
    "pop r28\n\t"				\
    "pop r29\n\t"				\
    "pop r30\n\t"				\
    "pop r31\n\t"				\
    "pop r24\n\t"				\
    "out __SREG__, r24\n\t"		\
    "pop r24\n\t"				\
    "sei\n\t"				\
    );
}

/**
 * @brief Set the status of this thread to ACTIVE.
 * \param thrd	The thread TCB to be operated.
 */
INLINE void
active_Thread(thrd_tcb_t *thrd)
{
	if(thrd != NULL)
		thrd->status = THRD_ACTIVE;
	/* yield the control to the others. */
	thread_dispatcher();
}

/**
 * @brief Set the status of this thread to SUSPENDED.
 * \param thrd	The thread TCB to be operated.
 */
INLINE void
yield_Thread(thrd_tcb_t *thrd)
{
	if(thrd != NULL)
		thrd->status = THRD_SUSPENDED;
	/* yield the control to the others. */
	thread_dispatcher();
}
#endif	// RT_SUPPORT
