/**
 * @file mem_SFL.c
 * 
 * @brief	MIROS segregated free list (SFL) allocator.
 *			Segregated free list (SFL) allocator divides the memory space into segregated partitions. 
 *			Each partition holds a set of specified size blocks, and a free list is used for the allocation in each partition. 
 *			Upon allocation, a block is deleted from the free list which matches the allocation size. 
 *			Upon releasing, the released block will be added to the matching free list.
 *		
 *			To avoid the heap insufficiency problem, the sizes of the partitions are not reserved to 
 *			the largest sizes that they may be required in the MIROS. 
 *			Instead, the moderate sizes by which the requirements of most WSN applications can be met are assigned for the partitions. 
 *			And in case a partition is memory overflowed, the allocation will not be failed but continue to be allocated 
 *			in the extended heap space.
 *
 * @author    Xing Liu  (http://edss.isima.fr/sites/smir/)
 * @author    LIMOS Laboratory - UMR CNRS 6158: http://edss.isima.fr
 * @author    Supported email: liu@isima.fr
 */

/* === INCLUDES ============================================================ */
#include "typedef.h"
#include "kernel.h"
#include "kdebug.h"
#include "multithreading_sched.h"
#include "timer_ACV.h"
#include "timer_RCV.h"
#include "mem_SFL.h"
#include "mem_SFL_extHeap.h"
#include "ipc.h"

#if	MEM_SFL

/* === TYPES =============================================================== */


/* === MACROS ============================================================== */
/* create partitions here: MEM_PARTITION_CREATE(name, struct_name, blkNum) */
MEM_PARTITION_CREATE(timer, timer_t, 2);	/* create partition for software timer, with 10 blocks. */
MEM_PARTITION_CREATE(ipc, ipc_msgQ_t, 2);	/* create partition for IPC sending/recving, with 30 blocks. */

/* === GLOBALS ============================================================= */


/* === PROTOTYPES ========================================================== */


/* === IMPLEMENTATION ====================================================== */
/**
 * @brief Initialization of the MIROS SFL partition.
 *
 *	This function is used to link all the free blocks to the free list
 *	after a partition is created.
 *
 * \param *pt	Header of a partition.
 * \return NULL.
 */
void
memSFL_partition_init(partition_t *pt)
{
	uint8_t	i;
	memblk_t *blk;
	/* link all the free blocks at the init stage. */
	for(i = 0, blk = pt->ptFreeQ; i < pt->blk_num; i++, blk = blk->next)
	{
		blk->next = (void *)blk + pt->blk_size + sizeof(memblk_t);
		blk->pt = pt;
		/* the last one */
		if(i == (pt->blk_num -1))
			blk->next = NULL;
	}
}


/**
 * @brief MIROS SFL allocation.
 *
 *	If SFL allocation from a given partition is failed, this allocation will be done continuously inside the extended heap. 
 *	In the extended heap, the SF allocation mechanism will be used. 
 *
 * \param pt	The header of a given partition.
 * \return		Return the address of the allocated object.
 */
void*
mem_alloc(partition_t *pt)
{
	memblk_t *mem_blk = NULL;

	/* If partition free list is not NULL, allocate from this partition. */
	if(pt->ptFreeQ != NULL)
	{
		/* delete from the header directly, allocation can be completed in constant time. */
		mem_blk = pt->ptFreeQ;
		pt->ptFreeQ = pt->ptFreeQ->next;
		pt->blk_num--;
		/* return. */
		return ((void *)mem_blk + sizeof(memblk_t));
	}
	
	/* If partition free list is NULL, allocate from the extended heap space. */
	else
		return memSFL_extHeap_alloc(pt->blk_size);
}


/**
 * @brief Free a chunk
 *
 *	If object to be released is inside the extended heap space, add it into the free list of hpFreeQ.  
 *	If two freed objects are adjacent, coalesce them.
 *	If object to be released is inside a partition, add it into the partition free list ptFreeQ.
 *
 * \param chkMem  Address of the chunk to be released.
 * 
 */
void
mem_free(void *chkMem)
{
	memblk_t *mem_blk = NULL;
	
	/* if object locates insides a partition, add this object to the header of partition free list. */
	if(chkMem < heapSaddr)
	{
		mem_blk = (memblk_t *)((void *)chkMem - sizeof(memblk_t));
		/* insert into the header of list ptFreeQ. */
		mem_blk->next = mem_blk->pt->ptFreeQ;
		mem_blk->pt->ptFreeQ = mem_blk;
		/* update the available number. */
		mem_blk->pt->blk_num++;
	}
	
	/* if object locates in the extended heap, add this object to the free list hpFreeQ. */
	else
		memSFL_extHeap_free(chkMem);
}

#endif
