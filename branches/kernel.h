/**
 * @file kernel.h
 *
 * @author    LIMOS Laboratory - UMR CNRS 6158: http://edss.isima.fr
 * @author    Supported email: liu@isima.fr
 */

/* Prevent double inclusion */
#ifndef _KERNEL_H_
#define _KERNEL_H_ 
 
/* === Includes ============================================================= */
#include "board.h"
#include "evt_driven_sched.h"
#include "multithreading_sched.h"
#include "qlist_proc.h"
#include "timer_ACV.h"
#include "timer_RCV.h"

/* === Macros =============================================================== */
#define ALIGN_SIZE		2


/* === Types ================================================================ */
typedef enum kRuntime_status
{
	ERROR,
	MEM_ALLOC_ERROR,
} kRuntime_status_t;

/* === GLOBALS ============================================================= */
extern uint16_t _sys_data_end;
extern thrd_tcb_t common_thread, *curThrd;
extern void* heapSaddr;


/* === Prototypes =========================================================== */

#endif



