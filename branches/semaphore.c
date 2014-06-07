/**
 * @file semaphore.c
 *
 * @brief  semaphore to access the shared resources among the threads. 
 *
 * @author	  Xing Liu  (http://edss.isima.fr/sites/smir/)
 * @author    LIMOS Laboratory - UMR CNRS 6158: http://edss.isima.fr
 * @author    Supported email: liu@isima.fr
 */

/* === INCLUDES ============================================================ */
#include "evt_driven_sched.h"
#include "semaphore.h"
#include "kernel.h"
#include "multithreading_sched.h"
#include "kdebug.h"

#if	RT_SUPPORT

/* === TYPES =============================================================== */


/* === MACROS ============================================================== */


/* === GLOBALS ============================================================= */


/* === PROTOTYPES ========================================================== */


/* === IMPLEMENTATION ====================================================== */
/**
 * @brief Function to initialize the semaphore
 * \param s			Pointer to a semaphore structure.
 * \param value		Init the semaphore.
 *
 * If the "value" is assigned as 1, then the semaphore becomes a binary semaphore.
 */
void 
sem_init(semaphore_t *s, int value)
{
	s->val = value;
	s->semQ_hdr =  NULL;
}

/** 
 * @brief  Post the semaphore to the next thread that is waiting for it.
 * \param s			The semaphore to be posted to the others.
 * \param action	Whether force the thread switch after this semaphore is posted.
 *
 * Increment the internal value of the semaphore,
 */
uint8_t
sem_post(semaphore_t *s, uint8_t action)
{
	HAS_CRITICAL_SECTION;
	thrd_tcb_t *thrd;
	
	ENTER_CRITICAL_SECTION;
	/* Post a unit and wake-up the next waiting thread. */
	s->val++;

	/* get the next task that is waiting for this resource. */
	thrd = s->semQ_hdr;
	/* set this thread's status to "ACTIVE". */
	if(thrd != NULL)
	{
		/* delete this thread from the semaphore queue */
		s->semQ_hdr = thrd->semQ_next;
		
		/* update this task's status */
		if(thrd->status == THRD_SUSPENDED)
		{
			thrd->status = THRD_ACTIVE;
			LEAVE_CRITICAL_SECTION;
			
			/* force the thread dispatcher now if it is required. */
			if(action == DISPATCHER)
				thread_dispatcher();
		}
	}
	LEAVE_CRITICAL_SECTION;
	
	return 0;
}

/** 
 * @brief Wait on a semaphore.
 * \param s		Try to get this semaphore.
 * \return		Return the acquiring status, SUCCESS if acquire the semaphore successfully.
 *
 * If the semaphore internal value is non-zero, get this resource and decrements the semaphore value.  
 * If the value is zero, then enter the waiting queue and dispatcher the thread.
 */
uint8_t
sem_acquire(semaphore_t *s)
{
	HAS_CRITICAL_SECTION;
	thrd_tcb_t *thrd;
	
	ENTER_CRITICAL_SECTION;
	/* if val is higher than or equal to 1,
	   then this resource can be obtained by this thread. */
	if(s->val >= 1)
	{
		s->val--;
		LEAVE_CRITICAL_SECTION;
		return ERROR;
	}	
	/* If no resources are available then we wait in the queue */
	else {
		/* add into the semaphore queue */
		if(s->semQ_hdr == NULL)
			s->semQ_hdr = curThrd;
		else
		{
			/* comes to the end */
			for(thrd = s->semQ_hdr; thrd->semQ_next == NULL; thrd = thrd->semQ_next);
			/* add this thread into the queue */
			thrd->semQ_next = curThrd;
		}
		/* set current thread to "SUSPENDED" status */
		curThrd->status = THRD_SUSPENDED;
		LEAVE_CRITICAL_SECTION;

		/* call task dispatcher */
		thread_dispatcher();
	}
	
	return 0;
}
#endif	// RT_SUPPORT