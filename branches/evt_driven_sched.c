/**
 * @file evt_driven_sched.c
 *
 * @brief  Event driven scheduler in MIROS.
 *
 *			MIROS event-driven scheduler is typical in that only the non-RT tasks need to be scheduled by it. 
 *			Thus, the real-time performance is not a critical factor for the design of the MIROS event-driven scheduler. 
 *			Due to this reason, the flag polling mechanism other than the scheduling queue mechanism 
 *			is used in the MIROS event scheduling system. 
 *
 * @author    Xing Liu  (http://edss.isima.fr/sites/smir/)
 * @author    LIMOS Laboratory - UMR CNRS 6158: http://edss.isima.fr
 * @author    Supported email: liu@isima.fr
 */

/* === INCLUDES ============================================================ */
#include "evt_driven_sched.h"
#include "timer.h"
#include "kernel.h"
#include "kdebug.h"
#include "board.h"
#include "demoTasks.h"

/* === TYPES =============================================================== */


/* === MACROS ============================================================== */


/* === GLOBALS ============================================================= */
/* Each bit in the task_flags corresponds to one MIROS task. */
uint16_t task_flags = 0;

/* Task control block tables for all the non-RT tasks.
   The order of the tasks listed in this table should match the order in the taskID_enum. 
   This table can also be put into the FLASH (by adding the modifier "PROGMEM"). */
task_TCB_t tsk_hd_table[] =
{
	{ dataCollect_Task, NULL },
	{ memAllocEval_Task, NULL },
};

#if KDEBUG_DEMO
uint8_t nonRT_tskID = 0xFF;
#endif

/* === PROTOTYPES ========================================================== */


/* === IMPLEMENTATION ====================================================== */
/**
 * @brief Event-driven scheduler. To schedule all the non-RT tasks in MIROS.
 *
 * In MIROS, every task has a corresponding task flag (one-bit). 
 * Once an event is generated, the related flag will be set. 
 * The event scheduler will poll the flags in loop. 
 * If a flag is observed to be set, the related task will be executed. 
 *
 * Task have priorities, and the priorities are statically assigned offline. 
 * If no tasks are active, the sleeping directive will be executed to make the sensor node fall asleep.
 *
 */
void
event_driven_scheduling(void)
{
	uint8_t i;
	uint16_t fb;
	tsk_handler_t tsk_exec = NULL;

	/* In case that no tasks are active, enter the idle status. */
	if(task_flags == 0)
	{
		hardware_sleep();
		return;
	}

	/* Poll the task flags and execute the tasks if the task flag is set.
	   Clear the task flag after a task runs to completion. */
	for(i = 0, fb = 0x01; i < 16; i++, fb = fb << 1)
	{
		if((task_flags&fb) != 0)
		{
			/* get the task handler address from the task table. */
			tsk_exec = (tsk_handler_t)((uint16_t)tsk_hd_table + 4*i);
			/* record the task ID for demo debug */
			#if KDEBUG_DEMO
			nonRT_tskID = i;
			#endif
			/* execute the task handler. */
			tsk_exec();
			/* clear the task flag. */
			task_flags &= ~(0x01 << i);
			/* break here, thus the high-priority task will always be executed in advance. */
			break;
		}
	}
}


/**
 * @brief Post an event by setting the related flag.
 */
INLINE void
taskPost(uint8_t task_ID)
{
	task_flags ^= (0x01 << task_ID);
}