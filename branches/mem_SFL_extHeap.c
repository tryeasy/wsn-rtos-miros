/**
 * @file mem_SFL_extHeap.c
 * 
 * @brief	MIROS segregated free list (SFL) allocator.
 *			When a partition is memory overflowed, the allocation will be done in the extended heap.
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

#if	MEM_SFL
#include "mem_SFL.h"
#include "mem_SFL_extHeap.h"
/* === TYPES =============================================================== */


/* === MACROS ============================================================== */


/* === GLOBALS ============================================================= */
/* A list to link all the free memory in the extended heap. */
sfl_extHpHdr_t *hpFreeQ = NULL;	


/* === PROTOTYPES ========================================================== */


/* === IMPLEMENTATION ====================================================== */
/**
 * @brief MIROS SFL allocation in the extended heap space.
 *
 *	If SFL allocation from a given partition is failed, this allocation will be done continuously inside the extended heap. 
 *	In the extended heap, the SF allocation mechanism will be used. 
 *
 * \param objSz	The size to be allocated from the extended heap.
 * \return			Return the address of the allocated object.
 */
void*
memSFL_extHeap_alloc(uint8_t objSz)
{
	sfl_extHpHdr_t *ck, *alloc = NULL;
	uint8_t splitSz = 0;
	
	/* Compute the required length. In extended heap space, the header is required for each object. */
	objSz = ALIGN(objSz+sizeof(sfl_extHpHdr_t), ALIGN_SIZE);
	/* get the split size. If the free chunk size is larger then this, split it. */
	splitSz = objSz+sizeof(sfl_extHpHdr_t)+MIN_PAYLOAD_SIZE;
	
	/* no available memory left. */
	if(hpFreeQ == NULL)	return NULL;

	/* start allocation from the queue tail. */
	ck = hpFreeQ;
	/* search from the freed chunk queue. */
	do
	{
		/* find an available entry. */
		if((ck->ckSize >= objSz) && (ck->ckSize <= splitSz))
		{
			/* initialization and return. The total size "splitSz" will be allocated. */
			ck->ckSize = splitSz;	
			/* delete ck from the freed chunk queue. */
			dlst_del((dlist **)(&hpFreeQ), (dlist *)ck);
			/* reference not used, return the data payload address directly. */
			return (void *)((uint16_t)ck + sizeof(sfl_extHpHdr_t));
		}
		
		/* split operation is required as the chunk size 
		   is larger than the required one. */
		if(ck->ckSize > splitSz)
		{
			/* split a piece from the bottom of this chunk. */
			alloc = (sfl_extHpHdr_t *)((uint16_t)ck + ck->ckSize - objSz);
			alloc->ckSize = objSz;
			/* new chunk update. */
			ck->ckSize -= objSz;
			/* reference not used, return the data payload address directly. */
			return (void *)((uint16_t)alloc + sizeof(sfl_extHpHdr_t));
		}
		
		/* check the next one */
		ck = ck->next;
	} while(ck != hpFreeQ);
	
	/* not successful. */
	return NULL;
}

/**
 * @brief Free a chunk
 *
 *	If object to be released is inside the extended heap space, then add it into the free list of extHeapFreeQ. 
 *	In this case, if two freed objects are adjacent, coalesce them.
 *	Otherwise, add it into the partition free list ptFreeQ. In this case, no memory coalescence will be required.
 *
 * \param mem  Address of the object to be released.
 * 
 */
void
memSFL_extHeap_free(void *mem)
{
	sfl_extHpHdr_t *chuk, *ck = hpFreeQ;
	/* locate to the chunk header position. */
	chuk = mem - sizeof(sfl_extHpHdr_t);

	/* If the queue is empty. */
	if(hpFreeQ == NULL)  {
		hpFreeQ = chuk;
		hpFreeQ->prev = hpFreeQ->next = hpFreeQ;
		return;
	}
	
	/* locate the insertion position. */
	while(ck < chuk)	{
		/* check next until find the available position. */
		ck = ck->next;
		/* if comes to the ends, then break, and will insert to the queue tail. */
		if(ck == hpFreeQ)	break;
	}
	
	/* insert "chuk" in front of "ck". */
	dlst_insert((dlist *)chuk, (dlist *)ck);
	/* update the queue	head. */
	if(chuk < hpFreeQ)		hpFreeQ =  chuk;
	
	/* merge if two free chunks are adjacent.
	   firstly, upper-address merging, merge "chuk & chuk->next".
	   later, lower-address merging, merge "chuk->prev & chuk". */
	dlst_merge((dlist **)(&hpFreeQ), (dlist *)chuk, (dlist *)chuk->next);
	dlst_merge((dlist **)(&hpFreeQ), (dlist *)chuk->prev, (dlist *)chuk);
}

#endif
