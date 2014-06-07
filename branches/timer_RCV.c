/**
 * @file timer_RCV.c
 *
 * @brief  software timers implemented by using RCV (relative counter value).
 *
 * The timers can be classified into two modes: one-slot timer and periodic timer.
 * The one-slot timer will be deleted once it is expired.
 * The periodic timer will be reset and restarted after expired.
 *
 * @author	  Xing Liu  (http://edss.isima.fr/sites/smir/) 
 * @author    LIMOS Laboratory - UMR CNRS 6158: http://edss.isima.fr
 * @author    Supported email: liu@isima.fr
 */


/* === INCLUDES ============================================================ */
#include "typedef.h"
#include "sys_config.h"
#include "evt_driven_sched.h"
#include "multithreading_sched.h"
#include "kdebug.h"

#if TIMER_RCV
#include "timer_RCV.h"
/* === TYPES =============================================================== */


/* === MACROS ============================================================== */


/* === GLOBALS ============================================================= */
timer_t *sysTimerQhead = NULL; // head of the system timer queue.

/* === PROTOTYPES ========================================================== */


/* === IMPLEMENTATION ====================================================== */

/**
 * @brief Hardware Periodical Interrupt Timer (PIT) handler
 *
 * This is the interrupt service routine for hardware PIT.
 * It is triggered when a timer expires.
 */
ISR(TIMER4_COMPA_vect)
{
	HAS_CRITICAL_SECTION;
	ENTER_CRITICAL_SECTION;	
	/* SysTimer Service. */
	timerService();
	LEAVE_CRITICAL_SECTION;
}

/**
 * @brief Interrupt service routine for the system timers.
 *
 * This is the interrupt service routine for timer.
 * It checks all the timers in the system timer queue, 
 * if a timer is fired, the related timer callback will be executed.
 */
void 
timerService(void)
{
	timer_t *t;

	/* search for expired timers and take actions */
	for(t = sysTimerQhead; t != NULL; t = t->next)
	{
		/* if counter value smaller than APPTIMERINTERVAL, timer will be fired. */
		if(t->interval < APPTIMERINTERVAL)
		{
			/* remove this fired timer from the timer queue. */
			stopTimer(t);
			/* if the timer is a periodical one, add it into the timer queue again. */
			if (t->mode == TIMER_REPEAT_MODE)	startTimer(t);
	
			/* When timer is fired, call the timer callback function. */
			t->callback(t->cb_data);
		}
		else  /* decrease the counter value. */
			t->interval -= APPTIMERINTERVAL;
	}
}

/**
 * @brief Starts a timer.
 */
int 
startTimer(timer_t *Timer)
{
	if (!Timer)
	return -1;
	if (true == isAlreadyInQueue((sQList *)sysTimerQhead, (sQList *)Timer))
	return 0;

	/* insert the new timer to the head of timer queue. */
	Timer->next = sysTimerQhead;
	sysTimerQhead = Timer;

	return 0;
}

/**
 * @brief Stops the timer.
 */
int 
stopTimer(timer_t *Timer)
{
	timer_t *prev = 0;
	timer_t **t = &Timer;

	if (!Timer)	return -1;
	
	/* when "Timer" is not header, get its previous one. */
	if (sysTimerQhead != *t)
	{
		if (!(prev = (timer_t *)findPrevEntry((sQList *)sysTimerQhead, (sQList *)Timer)))
		return -1;
	}
	RemoveEntryFromQ((sQList **)(&sysTimerQhead), (sQList *)prev, (sQList *)Timer);
	return 0;
}
#endif	/* #if TIMER_RCV */
