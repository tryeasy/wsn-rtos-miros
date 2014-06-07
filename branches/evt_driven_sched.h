/**
 * @file evt_driven_sched.h
 *
 * @brief  header for evt_driven_sched.c
 *
 * @author    Xing Liu  (http://edss.isima.fr/sites/smir/)
 * @author    LIMOS Laboratory - UMR CNRS 6158: http://edss.isima.fr
 * @author    Supported email: liu@isima.fr
 */

/* Prevent double inclusion */
#ifndef _EVENT_DRIVEN_SCHED_H_
#define _EVENT_DRIVEN_SCHED_H_
 
/* === Includes ============================================================= */
#include "board.h"
#include "typedef.h"
#include "os_start.h"

/* === Macros =============================================================== */



/* === Types ================================================================ */
/* enum for the task flags. Every task has one corresponding bit.
   the smaller the index is, the higher the task priority will be.
   Since task_flags is 16 bits, maximum 16 tasks can be defined. */
typedef enum taskID_enum
{
	dataCollect_Task_ID,
	nonRT_usartTask_evaluation_ID,
} taskID_enum_t;

/* Task control block (TCB) for the non-RT tasks.
   The RT tasks are executed by threads, and the thread control blocks are defined for these tasks. */
typedef uint8_t (*tsk_handler_t)(void);
typedef struct
{
	tsk_handler_t tsk;
	void *data;
}task_TCB_t;

/* ID for debugging */
extern uint8_t nonRT_tskID;

/* === Prototypes =========================================================== */
extern void event_driven_scheduling(void);
extern void taskPost(uint8_t task_ID);


#endif
