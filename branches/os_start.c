/**
 * @file os_start.c
 *
 * @brief  main entry and software init.
 *
 * Do initialization, and then start the event-driven scheduler.
 *
 * 
 * @author    Xing Liu  (http://edss.isima.fr/sites/smir/)
 * @author    LIMOS Laboratory - UMR CNRS 6158: http://edss.isima.fr
 * @author    Supported email: liu@isima.fr
 */

/* === INCLUDES ============================================================ */
#include "sys_config.h"
#include "board.h"
#include "kernel.h"
#include "os_start.h"
#include "usart.h"
#include "demoTasks.h"
#include "kdebug.h"
#include "gpio.h"
#include "mem_proactive_SF.h"
#include "mem_reactive_SF.h"
#include "mem_SFL.h"
#include "mem_SFL_extHeap.h"
#include "avr/delay.h"


/* === TYPES =============================================================== */


/* === MACROS ============================================================== */
/* declare all the system tasks that need to be initialized . */
TASK_INIT_DECLARE(&dataCollect_Task);

/* === GLOBALS ============================================================= */
/* memory heap space starting address.
   heap locates after the system BSS/DATA and thread stacks. */
void* heapSaddr = NULL;

/* In order to make the scheduler switch be efficient and easy-managed, 
 * the event-driven scheduling in MIROS is also implemented as a thread, 
 * named "common_thread", and the run function of this thread is to schedule the non-RT tasks one by one. */
thrd_tcb_t common_thread, *curThrd = NULL;

/* === PROTOTYPES ========================================================== */
#if DEBUG_SUPPORT
/* timer for debugging. */
timer_t debugTimer =
{
	interval: 5000,
	mode: TIMER_REPEAT_MODE,
	callback: rtTaskThrdStatus,
	cb_data: "timer_test!\n",
};

/* memory allocation. */
#if MEM_SFL
void *p1 = NULL, *p2 = NULL, *p3 = NULL, *p4 = NULL, *p5 = NULL, *p6 = NULL;
#endif
#if MEM_REACTIVE_SF
uint16_t *r1, *r2, *r3, *r4, *r5, *r6;
#endif
#if MEM_PROACTIVE_SF
uint16_t *t1, *t2, *t3;
#endif

#endif


/* === IMPLEMENTATION ====================================================== */
/**
 * @brief Main entry function
 */
int
main(void)
{
	/* low-level hardware Initialization */
	lowlevel_init();

	/* software initialization (process init, heap init, memory allocation init, etc.) */
	software_init();

	#if DEBUG_SUPPORT
		#if MEM_SFL
			p1 = mem_alloc(&timer_pt);
			memAlloc_debug(p1, sizeof(timer_t), 1);
			p2 = mem_alloc(&ipc_pt);
			memAlloc_debug(p2, sizeof(ipc_msgQ_t), 2);
			p3 = mem_alloc(&timer_pt);
			memAlloc_debug(p3, sizeof(timer_t), 3);
			p4 = mem_alloc(&timer_pt);
			memAlloc_debug(p4, sizeof(timer_t), 4);
			mem_free(p3);
			p5 = mem_alloc(&ipc_pt);
			memAlloc_debug(p5, sizeof(ipc_msgQ_t), 5);
			p3 = mem_alloc(&timer_pt);
			memAlloc_debug(p3, sizeof(timer_t), 3);
			p6 = mem_alloc(&ipc_pt);
			memAlloc_debug(p6, sizeof(ipc_msgQ_t), 6);
		#endif
		
		#if MEM_REACTIVE_SF
			/* memory allocation */
			r1 = mem_alloc(10);
			memAlloc_debug(r1, 10, 1);
			r2 = mem_alloc(17);
			memAlloc_debug(r2, 18, 2);
			r3 = mem_alloc(25);
			memAlloc_debug(r3, 26, 3);
			r4 = mem_alloc(36);
			memAlloc_debug(r4, 36, 4);
			r5 = mem_alloc(27);
			memAlloc_debug(r5, 28, 5);
			r6 = mem_alloc(55);
			memAlloc_debug(r6, 56, 6);
			/* memory releasing */
			mem_free(r1);
			mem_free(r3);
			/* set heap size to 260, and then
			   fragment assembling will be done here. */
			#if	FRAG_ASSMBL_DEBUG
			r1 = mem_alloc(38);
			#endif
			mem_free(r2);
			mem_free(r5);
			mem_free(r6);
			mem_free(r4);
		#endif
		
		#if MEM_PROACTIVE_SF
			t1 = mem_alloc(20);
			memAlloc_debug(t1, 20, 1);
			t2 = mem_alloc(30);
			memAlloc_debug(t2, 30, 2);
			t3 = mem_alloc(38);
			memAlloc_debug(t3, 38, 3);
			mem_free(t2);
			t2 = mem_alloc(50);
			memAlloc_debug(t2, 50, 5);
			mem_free(t1);
			mem_free(t3);
		#endif

		#if	TIMER_DEBUG
			startTimer(&debugTimer);
			while(1);
		#endif
	#endif


	/* Event-driven scheduler loop.
	 *
	 * If common_thread (event-driven scheduling) is executing and any RT thread becomes active during this process, 
	 * the common_thread will be preempted and then the OS will switch from the event-driven scheduling model 
	 * to the multithreaded scheduling model. 
	 *
	 * If all the RT threads are inactive, the common_thread will be scheduled by the multithreaded scheduler, 
	 * then the OS will switch back to the event-driven scheduling model once more.
	 *
	 */
	while(1) {
		event_driven_scheduling();
	}
}
 

/**
 * @brief software initialization.
 *
 * Initialization operations:
 * initialize the heap starting and ending address,
 * memory allocator initialization, and
 * system processes initialization.
 */
void
software_init(void)
{
	/* In order to make the scheduler switch be efficient and easy-managed,
	 * the event-driven scheduling in MIROS is also implemented as a thread,
	 * named "common_thread", and the run function of this thread is to schedule the non-RT tasks one by one.
	 */
	curThrd = &common_thread;
	#if KDEBUG_DEMO
	curThrd->thrd_id = 1;
	#endif
	
	/* thread stacks starting memory address.
	   The symbol "_sys_data_end" is defined in the link script,
	   it indicates the ending address of system DATA/BSS section. */
	heapSaddr = &_sys_data_end;
	
	/* init the GPIO ports */
	kDebug_init();

	/* start RT tasks by creating the RT threads. */
	start_RT_tasks();

	/* non-RT tasks initialization. */
	sysTasks_init();
	
	/* init the "hpFreeQ" and "reSF_freeQ" for MIROS memory allocator. */
	mem_init();
}

/**
 * @brief Initialization of MIROS allocator.
 *
 *	This function is used to initialize the "hpFreeQ" or "reSF_freeQ" in MIROS memory allocation. 
 */
void
mem_init(void)
{
	/* for MIROS SFL allocator. */
	#if MEM_SFL
		/* initialize hpFreeQ in SFL allocator. */
		hpFreeQ = (sfl_extHpHdr_t *)heapSaddr;
		hpFreeQ->ckSize = HEAP_EADDR - (uint16_t)heapSaddr;
		hpFreeQ->next = hpFreeQ->prev = (sfl_extHpHdr_t *)heapSaddr;
		/* init the partitions. */
		mem_init_partitions();
	#endif
	
	/* for MIROS reactive SF allocator. */
	#if MEM_REACTIVE_SF
		/* init the free heap. */
		reSF_freeQ = (reSF_chk_hdr_t *)heapSaddr;
		reSF_freeQ->ckSize = HEAP_EADDR - (uint16_t)heapSaddr;
		
		#if KDEBUG_DEMO	// used for debug demo, set to 112 bytes (8 bytes for each unit).
		reSF_freeQ->ckSize = 112;
		#endif		
		#if	FRAG_ASSMBL_DEBUG
		/* set heap size to 260 for fragment assembling test. */
		reSF_freeQ->ckSize = 260;
		#endif
		
		reSF_freeQ->next = reSF_freeQ->prev = (reSF_chk_hdr_t *)heapSaddr;
		reSF_freeQ->ckRef = NULL;
		/* init all the references */
		uint8_t i;
		for(i = 0; i < REF_NUM; i++)
			reSF_Ref[i] = 0;
	#endif
	
	/* for MIROS proactive SF allocator. */
	#if MEM_PROACTIVE_SF
		leftHpSaddr = heapSaddr;
		/* init all the references */
		uint8_t i;
		for(i = 0; i < REF_NUM; i++)
			proSF_Ref[i] = 0;
	#endif
}

/**
 * @brief Initialization for all memory partitions.
 */
void
mem_init_partitions(void)
{
	/* init all the created partitions here. */
	#if MEM_SFL
	memSFL_partition_init(&timer_pt);
	memSFL_partition_init(&ipc_pt);
	#endif
}

/**
 * @brief Initialization for all the declared system tasks.
 */
void
sysTasks_init(void)
{
	int i;
	tsk_handler_t sys_initTsk;

	/* initialize all the necessary tasks. */
	for(i = 0; sys_taskQ[i] != NULL; i++) {
		sys_initTsk = sys_taskQ[i];
		sys_initTsk();
	}
}

/**
 * @brief Debug for SF allocation, fill in debug data to the new allocated chunk.
 */
#if DEBUG_SUPPORT
void
memAlloc_debug(uint16_t *mRef, uint8_t size, uint8_t data)
{
	uint8_t i, *p;
	
	#if MEM_PROACTIVE_SF
		p = (uint8_t *)*mRef;
	#endif
	#if MEM_REACTIVE_SF
		p = (uint8_t *)*mRef;
	#endif	
	#if MEM_SFL
		p = (uint8_t *)mRef;
	#endif
	
	/* fill in the data into the allocated space. */
	for(i = 0; i < size; i++)
		*p++ = data;
}
#endif
