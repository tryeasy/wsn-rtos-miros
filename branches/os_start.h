/**
 * @file os_start.h
 *
 * @brief  header for os_start.c
 *
 * @author	  Xing Liu  (http://edss.isima.fr/sites/smir/)
 * @author    LIMOS Laboratory - UMR CNRS 6158: http://edss.isima.fr
 * @author    Supported email: liu@isima.fr
 */

/* Prevent double inclusion */
#ifndef _OS_START_H_
#define _OS_START_H_

/* === Includes ============================================================= */


/* === Macros =============================================================== */
#define TASK_INIT_DECLARE(...)					\
const tsk_handler_t sys_taskQ[] = {__VA_ARGS__, NULL}

/* === Types ================================================================ */



/* === GLOBALS ============================================================= */



/* === Prototypes =========================================================== */
extern void sysTasks_init(void);
extern void software_init(void);
extern void lowlevel_init(void);
extern void mem_init(void);
extern void mem_init_partitions(void);
extern void memAlloc_debug(uint16_t *mRef, uint8_t size, uint8_t data);

#endif

