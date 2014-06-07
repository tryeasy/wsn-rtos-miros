/**
 * @file demoTasks.c
 *
 * @brief  demo examples to evaluate the functionalities of the MIROS.
 *
 * @author	  Xing Liu  (http://edss.isima.fr/sites/smir/)
 * @author    LIMOS Laboratory - UMR CNRS 6158: http://edss.isima.fr
 * @author    Support email: liu@isima.fr
 */

/* === INCLUDES ============================================================ */
#include "demoTasks.h"
#include "usart.h"
#include "kdebug.h"
#include "avr/delay.h"
#include "mem_SFL.h"
#include "multithreading_sched.h"
#include "evt_driven_sched.h"
#include "mem_reactive_SF.h"

/* === TYPES =============================================================== */



/* === MACROS ============================================================== */
/* definitions for the RT tasks. */
#define RT_TASK_PERIOD1			48000
#define RT_TASK_WORKING_TIME1	12

#define RT_TASK_PERIOD2			12000
#define RT_TASK_WORKING_TIME2	3

#define RT_TASK_PERIOD3			16000
#define RT_TASK_WORKING_TIME3	6

/* definitions for the non-RT tasks. */
#define NON_RT_TASK_TIME1		2

/* definitions for the memory allocation in the demo. */
#define THRD1_MEM			31
#define THRD2_MEM			23
#define THRD3_MEM			15
#define NON_RT_TASK_MEM		7

/* === GLOBALS ============================================================= */
/* software timer used by the tasks. */
timer_t tskTimer, rtTimer1, rtTimer2, rtTimer3;
/* Thread TCB for the RT tasks. */
thrd_tcb_t *rtTskThrd1 = NULL, *rtTskThrd2 = NULL, *rtTskThrd3 = NULL;

/* === PROTOTYPES ========================================================== */



/* === IMPLEMENTATION ====================================================== */

/**
 * @brief Sensor data collection task.
 * This task will sample the sensor devices and transmit the sensor data packet.
 *
 * After the sensing data is collected, they will be filled into a packet and then be transmitted. 
 * Once transmitted, the ACK response will be waited.
 * If ACK is not received within a given time, the packet will be retransmitted.
 *
 * State-machine programming is used for two purpose:
 * 1). to keep high concurrency-intensive in the event-driven scheduling system. 
 * 2). to improve the software reliability (for roll-back recovery). 
 *
 */
uint8_t
dataCollect_Task(void)
{
	static uint8_t tsk_state = SENSING_TASK_INIT, pktRtrsCnt;
	static sensingPkt_t sensingPkt;		/* packet to be sent. */

	/* init operation for this task. */
	if(tsk_state == SENSING_TASK_INIT)
	{
		pktRtrsCnt = 3;	/* maximum retransmission counter. */
		/* set next task state. */
		tsk_state = SENSING_TASK_FRAME_CREATION;
		return 0;
	}

	/* Sample the sensors and fill the sensor packet frame.
	   Split operation is done in this handler before sending the frame out. */
	if(tsk_state == SENSING_TASK_FRAME_CREATION)
	{
		/* sample the sensing data, and fill them into the sensing packet. */
		sensingDataSampling(sensingPkt);
		
		/* Set next task state (FRAME_SENDING), and then yield the processor control.
		   yield the CPU control here because the execution time of a task cannot be too long in event-driven system. */
		tsk_state = SENSING_TASK_FRAME_SENDING;
		taskPost(dataCollect_Task_ID);	
		
		return 0;
	}	

	/* send out the sensing data packet "sensingPkt" through the wireless TX module.
	   Then, set a timer to wait the ACK for 4 seconds,
	   if not received, the sensing packet will be retransmitted. */
	if(tsk_state == SENSING_TASK_FRAME_SENDING)
	{
		/* send packet out */
		MIROS_send(WIRELESS_TX_ID, sensingPkt, sizeof(sensingPkt), 0);
		
		/* start a timer to wait for the ACK. */
		tskTimer.callback = sensingTskAckRslt;
		tskTimer.interval = 4000;	/* 4 seconds. */
		tskTimer.mode = TIMER_ONE_SHOT_MODE;
		startTimer(&tskTimer);
		
		return 0;
	}
	
	/* if received the ACK packet within 4 seconds */
	if(tsk_state == SENSING_TASK_ACK_RECEPTION)
	{
		/* stop the ACK timer. */
		stopTimer(&tskTimer);
		
		/* set next working state, to sample the data again. */
		tsk_state = SENSING_TASK_FRAME_CREATION;
		
		/* start a new timer which will wake up the node when the next working period arrives. */
		tskTimer.callback = sensingTskRestart;
		tskTimer.interval = 60000;	/* 1 minutes. */
		tskTimer.mode = TIMER_ONE_SHOT_MODE;
		startTimer(&tskTimer);
		
		/* node fall asleep. */
		hardware_sleep();
		return 0;
	}
	
	/* if not received the ACK successfully within 4 seconds. */
	if(tsk_state == SENSING_TASK_RETRANSMISSION)
	{
		if(pktRtrsCnt > 0)
		{
			pktRtrsCnt--;
			/* retransmit again. go back to state "SENSING_TASK_FRAME_SENDING". */
			tsk_state = SENSING_TASK_FRAME_SENDING;
			taskPost(dataCollect_Task_ID);	
			return 0;
		}
		else
		{
			/* retransmission maximum times, still failed,
				send an information through the USART to notify this. */
			send(USART_ID, "SEND_FAILED!\n", 15, 0);
			return 1;
		}		
	}
	
	return 0;
}


/**
 * @brief Demo tasks to evaluate the dynamic memory allocation in the MIROS. 
 *
 * Send a command from the PC by the USART port. 
 * Once the command is received, this task will become active and perform a memory allocation. 
 */
uint8_t
memAllocEval_Task(void)
{
	uint8_t t;
	uint16_t *mem = NULL;
	uint8_t schedCmd[4] = {0xAD, evt_sched_debugID, nonRT_tskID, 0xFF};
	
	/* check whether task ID is correct. */
	if(nonRT_tskID > 0x7F)
	{
		sendUsartByte(USART_CHANNEL_1, 'E');
		return;
	}
	
	/* allocate the memory */
	mem = mem_alloc(NON_RT_TASK_MEM);
	memAllocLstUpdate();
	
	/* computation time of this non-RT task is NON_RT_TASK_TIME1. */
	for(t = 0; t < (NON_RT_TASK_TIME1<<1); t++)
	{
		#if KDEBUG_DEMO
		kOutArray(schedCmd, 4);
		_delay_ms(500);
		#endif
	}
	
	/* release the allocated memory */
	if(mem != NULL)
	{
		mem_free(mem);
		memAllocLstUpdate();
	}
}


/**
 * @brief start the RT tasks by creating threads.
 *
 * start all the RT tasks here. 
 */
void
start_RT_tasks(void)
{
	/* create threads for RT tasks.
	   after threads are created, start a timer to control the working statue of this thread. */
	rtTskThrd1 = thread_create((tsk_handler_t)rtTask_usartEval1, RT_TASK_PERIOD1);
	/* start a timer to make this task work in period. 
	   Working  */
	rtTimer1.callback = rtTaskThrdStatus;
	rtTimer1.cb_data = (void *)rtTskThrd1;
	rtTimer1.interval = RT_TASK_PERIOD1;
	rtTimer1.mode = TIMER_REPEAT_MODE;
	startTimer(&rtTimer1);
	
	/* create threads for RT tasks.
	   after threads are created, start a timer to control the working statue of this thread. */	
	rtTskThrd2 = thread_create((tsk_handler_t)rtTask_usartEval2, RT_TASK_PERIOD2);
	/* start a timer to make this task work in period. */
	rtTimer2.callback = rtTaskThrdStatus;
	rtTimer2.cb_data = (void *)rtTskThrd2;
	rtTimer2.interval = RT_TASK_PERIOD2;
	rtTimer2.mode = TIMER_REPEAT_MODE;
	startTimer(&rtTimer2);

	/* create threads for RT tasks.
	   after threads are created, start a timer to control the working statue of this thread. */	
	rtTskThrd3 = thread_create((tsk_handler_t)rtTask_usartEval3, RT_TASK_PERIOD3);
	/* start a timer to make this task work in period. */
	rtTimer3.callback = rtTaskThrdStatus;
	rtTimer3.cb_data = (void *)rtTskThrd3;
	rtTimer3.interval = RT_TASK_PERIOD3;
	rtTimer3.mode = TIMER_REPEAT_MODE;
	startTimer(&rtTimer3);
	
	/* assign every thread an ID, which will be used for the demo. */
	#if KDEBUG_DEMO
	thrd_tcb_t *th;
	uint8_t i;
	for(i = 10, th = thrd_lstQ; th != NULL; th = th->next)
	{
		th->thrd_id = i--;
	}
	#endif
};


/**
 * @brief update the thread status of the RT tasks. 
 *
 * When RT tasks need to work in a new cycle, update the related thread status,
 * and then performance the thread switch.
 */
void
rtTaskThrdStatus(void *data)
{
	/* active the thread when the working timer is fired. */
	active_Thread((thrd_tcb_t *)data);
}


/**
 * @brief RT tasks defined for the test of MIROS multi-threaded scheduler. 
 *
 * After a RT task starts execution, it will send the debugging code to the GPIO.
 * These debugging codes will finally be forwarded to the local server. 
 * On the local server, these codes will be parsed and illustrated by the graph. 
 * By this way, the run-time process of the MIROS can be monitored.
 *
 */
uint8_t
rtTask_usartEval1(void)
{
	/* yield the thread, wait for the working time to arrive. */
	yield_Thread(curThrd);
	
	/* when timer is fired, the thread will be active.
	   Then, the following process will be performed. */
	rtDemoTask_process(RT_TASK_WORKING_TIME1, curThrd->thrd_id);
}

uint8_t
rtTask_usartEval2(void)
{
	/* yield the thread, wait for the working time to arrive. */
	yield_Thread(curThrd);
	
	/* when timer is fired, the thread will be active.
	   Then, the following process will be performed. */
	rtDemoTask_process(RT_TASK_WORKING_TIME2, curThrd->thrd_id);
}

uint8_t
rtTask_usartEval3(void)
{
	/* yield the thread, wait for the working time to arrive. */
	yield_Thread(curThrd);
	
	/* when timer is fired, the thread will be active.
	   Then, the following process will be performed. */
	rtDemoTask_process(RT_TASK_WORKING_TIME3, curThrd->thrd_id);
}

void
rtDemoTask_process(uint8_t time, uint8_t thrd_id)
{
	HAS_CRITICAL_SECTION;
	uint8_t t;
	uint8_t schedCmd[4] = {0xAD, thrd_sched_debugID, thrd_id, 0xFF};
	uint16_t *mem = NULL;
	
	/* thread will be active during computation time, 
	   and be suspended during idle time. */
	while(true)
	{
		/* allocate the memory for this thread when the thread become active */
		#if KDEBUG_DEMO
		#if MEM_REACTIVE_SF
			ENTER_CRITICAL_SECTION;
			/* thread 1 */
			if(thrd_id == 10)		mem = mem_alloc(THRD1_MEM);
			/* thread 2 */
			else if(thrd_id == 9)	mem = mem_alloc(THRD2_MEM);
			/* thread 3 */
			else if(thrd_id == 8)	mem = mem_alloc(THRD3_MEM);
			LEAVE_CRITICAL_SECTION;
			/* if allocation success, then update the trace information. */
			memAllocLstUpdate();
		#endif
		#endif
		
		/* computation time of this RT task is 1 seconds. */
		for(t = 0; t < (time<<1); t++)
		{
			#if KDEBUG_DEMO
			kOutArray(schedCmd, 4);
			//kOutArray(schedCmd, 4);	
			#endif
			_delay_ms(500);
		}
		
		/* for memory allocation demo */
		#if KDEBUG_DEMO
		#if MEM_REACTIVE_SF
			if(mem != NULL)
			{
				ENTER_CRITICAL_SECTION;
				mem_free(mem);
				LEAVE_CRITICAL_SECTION;
				/* udpate the trace information. */
				memAllocLstUpdate();
			}
		#endif
		#endif
		
		/* yield the thread.
		   thread will be suspended if not within its computation time. */
		yield_Thread(curThrd);
	}
}


#if KDEBUG_DEMO
void
memAllocLstUpdate(void)
{
	reSF_chk_hdr_t *p;
	
	/* send the header firstly */
	kDebug8bit(0xAD);
	kDebug8bit(memAlloc_debugID);
	/* send the body code */
	for(p = reSF_allocQ; p != NULL; p = p->next)
	{
		/* fill the debug trace command: (address offset: length: thrd_id). */
		kDebug8bit((uint16_t)p-(uint16_t)heapSaddr);// memory starting address offset.
		kDebug8bit(p->ckSize&0xFF);					// (low byte): memory size.
		kDebug8bit(p->thrd_id);					// (high byte): thread id.
	}
	/* send the tail */
	kDebug8bit(0xFF);
}
#endif
