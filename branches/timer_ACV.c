/**
 * @file timer_ACV.c
 *
 * @brief  software timers implemented by using ACV (absolute counter value).
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

#if TIMER_ACV
#include "timer_ACV.h"
/* === TYPES =============================================================== */


/* === MACROS ============================================================== */


/* === GLOBALS ============================================================= */
timer_t *sysTimerQhead = NULL; // head of the system timer queue.
uint32_t sysAbsTime = 0ul;     // system time, start counting after the node reboots.

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
	/* update the system time */
	sysAbsTime += APPTIMERINTERVAL;
	/* SysTimer Service. */
	timerService();
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
	uint32_t sysTime;

	/* search for expired timers and take actions */
	while ( sysTimerQhead
	&& ((sysTime = GetSysTime()) - sysTimerQhead->sysTimeLabel) >= sysTimerQhead->interval)
	{
		timer_t *p = sysTimerQhead;
		/* remove this fired timer from the timer queue. */
		RemoveEntryFromQ((sQList **)&sysTimerQhead, NULL, (sQList *)p);
		/* if the timer is a periodical one, add it into the timer queue again. */
		if (p->mode == TIMER_REPEAT_MODE)
		{
			p->sysTimeLabel = sysTime;
			AddTimer(&sysTimerQhead, p, sysTime);
		}
		
		/* When timer is fired, call the timer callback function. */
		p->callback(p->cb_data);
	}
}

/**
 * @brief Return system absolute time.
 * Return the system time in ms.
 */
INLINE uint32_t 
GetSysTime(void)  {
	return sysAbsTime;
}

/**
 * @brief Starts a timer.
 */
int 
startTimer(timer_t *Timer)
{
	uint32_t sysTime;

	if (!Timer)
	return -1;
	/* check whether the timer has already been in the timer queue. */
	if (true == isAlreadyInQueue((sQList *)sysTimerQhead, (sQList *)Timer))
	return 0;

	/* add this timer into the timer queue. */
	sysTime = GetSysTime();
	Timer->next = NULL;
	Timer->sysTimeLabel = sysTime;
	AddTimer((timer_t**)(&sysTimerQhead), (timer_t*)Timer, sysTime);
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

	if (!Timer)
	return -1;
	/* locate the position of the timer in the timer queue. */
	if (sysTimerQhead != *t)
	{
		if (!(prev = (timer_t *)findPrevEntry((sQList *)sysTimerQhead, (sQList *)Timer)))
		return -1;
	}
	/* remove this timer. */
	RemoveEntryFromQ((sQList **)(&sysTimerQhead), (sQList *)prev, (sQList *)Timer);
	return 0;
}

/**
 * @brief Adds timer to the timer queue.
 */
void 
AddTimer(timer_t **head, timer_t *newTimer, uint32_t sysTime)
{
#if	0	// ERROR CODE
	if (!*head)  {
		*head = newTimer;
		return;
	}

	timer_t *it, *prev = NULL;
	for (it = *head; it; it = it->next)
	{
		uint32_t remain;
		if (it->sysTimeLabel <= sysTime)
		remain = (uint32_t)(it->sysTimeLabel) + it->interval - sysTime;
		else
		remain = (uint32_t)it->interval - (~it->sysTimeLabel + 1) - sysTime;
		if (remain >= newTimer->interval)
		break;
		prev = it;
	}
	if (it == *head)
	{
		newTimer->next = *head;
		*head = newTimer;
	}
	else
	{
		prev->next = newTimer;
		newTimer->next = it;
	}
#endif


	if (!*head)  {
		*head = newTimer;
		return;
	}

	timer_t *it, *prev = NULL;
	for (it = *head; it; it = it->next)
	{
		uint64_t remain;
		if (it->sysTimeLabel <= sysTime)
		remain = (uint64_t)(it->sysTimeLabel) + it->interval - sysTime;
		else
		remain = (uint64_t)it->interval - (~it->sysTimeLabel + 1) - sysTime;
		if ((remain <= UINT32_MAX) && (remain >= newTimer->interval))
		break;
		prev = it;
	}
	if (it == *head)
	{
		newTimer->next = *head;
		*head = newTimer;
	}
	else
	{
		prev->next = newTimer;
		newTimer->next = it;
	}
}
#endif	/* #if TIMER_ACV */
