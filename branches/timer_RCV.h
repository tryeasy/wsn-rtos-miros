/**
 * @file timer_RCV.h
 *
 * @brief  header for file timer_RCV.c
 *
 * @author    LIMOS Laboratory - UMR CNRS 6158: http://edss.isima.fr
 * @author    Supported email: liu@isima.fr
 */

/* Prevent double inclusion */
#ifndef _SYS_TIMER_RCV_H_
#define _SYS_TIMER_RCV_H_ 
 
/* === Includes ============================================================= */
#include "board.h"
#include "timer.h"
#include "kernel.h"

#if TIMER_RCV
/* === Macros =============================================================== */
/* timer modes.
   There are two timer modes: periodical timer and one-slot timer.
   For one-slot timer, after it is fired, it will be deleted from the timer queue.
   For periodical timer, after fired, it will be added into the timer queue again. */
typedef enum
{
  TIMER_REPEAT_MODE,
  TIMER_ONE_SHOT_MODE,
} TimerMode_t;


/** \brief RCV Timer structure */
typedef void (*time_cb_t)(void *data);
__ALIGNED2 typedef struct _Timer_t
{
	struct _Timer_t *next;
	uint32_t interval;		/* timer counter. */
	time_cb_t callback;		/* callback function when timer is fired. */
	void *cb_data;			/* data used by timer callback. */
	uint8_t mode;			/* timer mode: TIMER_ONE_SHOT_MODE or TIMER_REPEAT_MODE. */
} timer_t;

/* === Types ================================================================ */


/* === GLOBALS ============================================================= */
extern timer_t *timerQhead; // head of Timer list

/* === Prototypes =========================================================== */
extern void timerService(void);
extern bool isTimerAlreadyStarted(timer_t *Timer);
extern int startTimer(timer_t *Timer);
extern int stopTimer(timer_t *Timer);

#endif	/* TIMER_ACV */
#endif




