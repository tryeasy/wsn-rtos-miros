/**
 * @file multithreading_sched.h
 *
 * @brief  header for multithreading_sched.c
 *
 * @author    LIMOS Laboratory - UMR CNRS 6158: http://edss.isima.fr
 * @author    Supported email: liu@isima.fr
 */

/* Prevent double inclusion */
#ifndef _MULTITHREADING_SCHED_H_
#define _MULTITHREADING_SCHED_H_
 
/* === Includes ============================================================= */
#include "board.h"
#include "evt_driven_sched.h"


/* === Types ================================================================ */
/* thread status */
typedef enum
{
	THRD_UNUSED,
	THRD_ACTIVE,
	THRD_SUSPENDED,
	THRD_SLEEPING
} thrd_status_t;

/* thread TCB structure */
__ALIGNED2 typedef struct thrd_tcb
{
	struct thrd_tcb *next;
	uint8_t *thrd_sp;			/* stack's run-time address. */
	tsk_handler_t thrd_tsk;	/* pointer to the task executed by this thread. */
	struct thrd_tcb *semQ_next;	/* queue for the resource semaphore. */
	uint16_t thrd_period;		/* period of the task, will determine the priority of this thread. */
	uint8_t status;				/* "UNUSED, SUSPENDED, ACTIVE, etc." */
	#if KDEBUG_DEMO
	uint8_t thrd_id;			/* used for demo. */
	#endif
} thrd_tcb_t;


/* === Macros =============================================================== */
#define MAX_THREAD_NUM		8
#define	THREAD_CONTEXT_SIZE	128


/* === GLOBALS ============================================================= */
extern thrd_tcb_t *thrd_lstQ;


/* === Prototypes =========================================================== */
extern thrd_tcb_t* thread_create(tsk_handler_t thrd_tsk, uint16_t tsk_period);
extern thrd_tcb_t* thrd_contextPrep(tsk_handler_t thrd_tsk, uint16_t tsk_period);
extern void thrd_start_wrapper(void);
extern thrd_tcb_t* getNextThread(void);
extern void thread_dispatcher(void);
extern void thrdContextSave(void);
extern void thrdContextRestore(void);
extern void active_Thread(thrd_tcb_t *thrd);
extern void yield_Thread(thrd_tcb_t *thrd);

#endif
