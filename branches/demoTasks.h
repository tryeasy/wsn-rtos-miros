/**
 * @file demoTasks.h
 *
 * @brief  header for demoTasks.c
 *
 * @author    LIMOS Laboratory - UMR CNRS 6158: http://edss.isima.fr
 * @author    Supported email: liu@isima.fr
 */

/* Prevent double inclusion */
#ifndef _DEMO_TASKS_H_
#define _DEMO_TASKS_H_ 
 
/* === Includes ============================================================= */
#include "board.h"
#include "kernel.h"
#include "usart.h"


/* === Macros =============================================================== */
/* task states for dataCollect_Task. */
typedef enum
{
	SENSING_TASK_INIT,
	SENSING_TASK_FRAME_CREATION,
	SENSING_TASK_FRAME_SENDING,
	SENSING_TASK_ACK_RECEPTION,
	SENSING_TASK_ACK_NO_SUCCESS,
	SENSING_TASK_RETRANSMISSION
} sensingTask_state_enum_t;


/* === Types ================================================================ */
/* structure for the sensing data packet. */
__ALIGNED2 typedef struct sensingPkt
{
	uint8_t     messageType;
	uint8_t     nodeType;
	uint64_t	macAddr;
	uint16_t	ipAddr;
	uint32_t    channelMask;
	uint16_t    panID;
	uint8_t     workingChannel;
	uint16_t	parentShortAddr;
	uint8_t     lqi;
	int8_t      rssi;
	struct {
		uint16_t battery;
		uint16_t temperature;
		uint16_t humidity;
		uint16_t light;
		uint16_t decagon;
		uint16_t watermark;
	} sensingData;
} sensingPkt_t;



/* === GLOBALS ============================================================= */
extern thrd_tcb_t *rtTskThrd1, *rtTskThrd2, *rtTskThrd3;
extern timer_t tskTimer, rtTimer1, rtTimer2, rtTimer3;


/* === Prototypes =========================================================== */
extern uint8_t dataCollect_Task(void);
extern uint8_t memAllocEval_Task(void);

extern void start_RT_tasks(void);
extern void rtTaskThrdStatus(void *data);
extern uint8_t rtTask_usartEval1(void);
extern uint8_t rtTask_usartEval2(void);
extern uint8_t rtTask_usartEval3(void);
extern void rtDemoTask_process(uint8_t t, uint8_t data);
extern void memAllocLstUpdate(void);

#endif
