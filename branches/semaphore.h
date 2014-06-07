/**
 * @file semaphore.h
 *
 * @brief  header for semaphore.c.
 *
 * @author	  Xing Liu  (http://edss.isima.fr/sites/smir/)
 * @author    LIMOS Laboratory - UMR CNRS 6158: http://edss.isima.fr
 * @author    Supported email: liu@isima.fr
 */

/* Prevent double inclusion */
#ifndef _SEMAPHORE_H_
#define _SEMAPHORE_H_

/* === INCLUDES ============================================================ */
#include "evt_driven_sched.h"
#include "kernel.h"

/* === TYPES =============================================================== */
/** @brief 
  * Semaphore data structure */
typedef struct {
	int8_t val;
	thrd_tcb_t* semQ_hdr;
} semaphore_t;

/** @brief 
   * whether dispatch the tasks after the semaphore resource is released. */
typedef enum sem_action
{
	DISPATCHER,
	NO_DISPATCHER
} sem_action_t;

/* === MACROS ============================================================== */


/* === GLOBALS ============================================================= */


/* === PROTOTYPES ========================================================== */


/* === IMPLEMENTATION ====================================================== */


#endif

