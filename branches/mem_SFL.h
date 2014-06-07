/**
 * @file mem_SFL.h
 *
 * @brief  header for mem_SFL.c
 *
 * @author    Xing Liu  (http://edss.isima.fr/sites/smir/)
 * @author    LIMOS Laboratory - UMR CNRS 6158: http://edss.isima.fr
 * @author    Supported email: liu@isima.fr
 */

/* Prevent double inclusion */
#ifndef _mem_SFL_H_
#define _mem_SFL_H_ 
 
/* === Includes ============================================================= */
#include "board.h"
#include "evt_driven_sched.h"
#include "qlist_proc.h"
#include "mem_SFL_extHeap.h"
#include "timer_ACV.h"
#include "timer_RCV.h"

#if	MEM_SFL


/* === Types ================================================================ */
typedef struct partition partition_t;
/* block structure in each partition.
   Maximum size of each block will be: 0xFF-sizeof(memblk_t) = 251. */
__ALIGNED2 typedef struct memblk {
	struct memblk *next;	/* single link list to link all the free blocks. */
	partition_t *pt;		/* link to the partition header, for block collection when this block is released. */
} memblk_t;

/* header for each partition. */
__ALIGNED2 struct partition {
	uint8_t blk_size;		/* blk_size is commonly the size of the struct. */
	uint8_t blk_num;		/* "blk_num/blk_size" will be used for memory reservation at initialization stage. */
	memblk_t *ptFreeQ;		/* Qhead to link all the free blocks, init to the partition's starting address at the system startup. */
};


/* === Macros =============================================================== */
/**
 * preprocessing macro for concatenating to strings.
 *
 * We need use two macros (CC_CONCAT and CC_CONCAT2) in order to allow
 * concatenation of two #defined macros.
 */
#define CC_CONCAT2(s1, s2) s1##s2
#define CC_CONCAT(s1, s2) CC_CONCAT2(s1, s2)


/*
 * This macro is used to create a new partition for SFL allocation.
 *
 * pre-reserve the partition memory space firstly, 
 * and then define and initialize the partition header.
 *
 * \param name 
 *		The name of this memory partition (later used with memb_init(), memb_alloc() and memb_free()).
 * \param struct_name 
 *		The name of the struct that this memory partition will hold.
 * \param blkNum
 *		The total number of memory blocks in this partition.
 */
#define MEM_PARTITION_CREATE(name, struct_name, blkNum) \
	uint8_t CC_CONCAT(name,_blks)[(sizeof(struct_name)+sizeof(memblk_t))*blkNum]; \
	partition_t CC_CONCAT(name,_pt) = {	\
		sizeof(struct_name), \
		blkNum, \
		(void *)CC_CONCAT(name,_blks)}


/* === GLOBALS ============================================================= */
extern uint8_t timer_blks[];
extern partition_t timer_pt;
extern uint8_t ipc_blks[];
extern partition_t ipc_pt;


/* === Prototypes =========================================================== */
/* memory management */
extern void memSFL_partition_init(partition_t *pt);
extern void* mem_alloc(partition_t *pt);
extern void mem_free(void *chkMem);


#endif
#endif
