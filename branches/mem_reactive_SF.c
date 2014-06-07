/**
 * @file mem_reactive_SF.c
 *
 * @brief  MIROS sequential fit allocator.
 *			In MIROS, the fragment assembling mechanism is implemented for the sequential fit allocator. 
 *			Currently, two assembling approaches have been realized: the proactive fragment assembling and 
 *			the reactive fragment assembling.
 *
 *			In the reactive fragment assembling mechanism, the fragments are not assembled every time an allocated object is released.
 *			Instead, they are assembled only when the 1st time SF allocation is failed, that is, 
 *			when there is no enough continuous free memory left for the new allocation. 
 *
 *			The same as proactive fragment assembling mechanism, reference pointers need to be used in 
 *			reactive fragment assembling mechanism as well, 
 *			and the update to the related reference pointers must be performed after the fragments are assembled.
 *
 * @author    Xing Liu  (http://edss.isima.fr/sites/smir/)
 * @author    LIMOS Laboratory - UMR CNRS 6158: http://edss.isima.fr
 * @author    Supported email: liu@isima.fr
 */

/* === INCLUDES ============================================================ */
#include "board.h"
#include "kernel.h" 
#include "sys_config.h"
#include "kdebug.h"

#if MEM_REACTIVE_SF
#include "mem_reactive_SF.h"
/* === TYPES =============================================================== */


/* === MACROS ============================================================== */


/* === GLOBALS ============================================================= */
/* references for the allocated chunks. Reference is 16-bit on AVR platform (uint16_t).
   Reference will point to the data payload starting address (exclude the header) */
uint16_t* reSF_Ref[REF_NUM];

/* free memory list for reactive SF allocator. */
reSF_chk_hdr_t* reSF_freeQ = NULL;	

#if KDEBUG_DEMO
reSF_chk_hdr_t* reSF_allocQ = NULL;
#endif


/* === PROTOTYPES ========================================================== */



/* === IMPLEMENTATION ====================================================== */
/**
 * @brief Memory allocated by MIROS sequential fit (reactive fragments assembled).
 *			If the 1st time SF allocation is failed, that is, when there is no enough continuous free memory left for the new allocation,
 *			the assembling operation will be done to the different memory fragments. 
 *			And then, the new allocation will be tried for another time.
 *			References need to be updated after the fragments are assembled.
 *
 * \param objSz		Required size to be allocated.
 * \return			Address of the reference header for the allocated chunk.
 */
uint16_t*
mem_alloc(uint8_t objSz)
{
	reSF_chk_hdr_t *alloc = NULL;
	uint8_t id;

	/* allocate a reference for this chunk firstly. */
	for(id = 0; id < REF_NUM; id++)
		if(reSF_Ref[id] == 0)	break;
	/* references are used up, maximum allocation. */
	if(id == REF_NUM)	return NULL;

	/* Compute the required length. */
	objSz = ALIGN(objSz+sizeof(reSF_chk_hdr_t), ALIGN_SIZE);

	/* 1st time allocation from the free memory list.
	   If failed, need to assemble all the memory fragments. */
	if((alloc = mem_alloc_proc(objSz)) == NULL)
	{
		/* assemble all the fragments. */
		fragment_assemble();
	
		/* 2nd time allocation, after fragments are assembled. 
		   From the header of free memory list "reSF_freeQ" directly. */
		if(reSF_freeQ->ckSize < objSz)	return NULL;
		else
		{
			/* update reSF_freeQ. */
			reSF_freeQ->ckSize -= objSz;
			/* create new allocated chunk. */
			alloc = (void *)reSF_freeQ + reSF_freeQ->ckSize - objSz;
			alloc->ckSize = objSz;
			alloc += sizeof(reSF_chk_hdr_t);
		}
	}
		
	/* allocation successfully, init reference and return.
	   Reference will point to the starting address of data payload. */
	reSF_Ref[id] = (void *)alloc + sizeof(reSF_chk_hdr_t);
	alloc->ckRef = (uint16_t *)&reSF_Ref[id];
	
	/* Add this allocated object into the list */
	#if KDEBUG_DEMO
	/* store the thread id into the high byte of "ckSize".
	   Note that "ckRef" in "reSF_chk_hdr_t" is needed for the memory free operation. */
	alloc->thrd_id = curThrd->thrd_id;
	/* if reSF_allocQ is empty. */
	if(reSF_allocQ == NULL)
	{
		reSF_allocQ =  alloc;
		alloc->next = NULL;
	}
	else
	{
		/* insert into the header */
		alloc->next = reSF_allocQ;
		reSF_allocQ = alloc;
	}
	#endif	// KDEBUG_DEMO
	
	return alloc->ckRef;
}


/**
 * @brief Memory allocated by the MIROS sequential fit (reactive fragments assembled).
 *		  1st time SF allocation from the free memory list.
 *
 * \param objSz		Required size to be allocated.
 * \return			Starting address of the allocated chunk.
 */
void*
mem_alloc_proc(uint8_t objSz)
{
	reSF_chk_hdr_t *ck, *alloc = NULL;
	uint8_t splitSz = 0;
	
	/* get the split size. If the free chunk size is larger then this, split it. */
	#if !KDEBUG_DEMO
	splitSz = objSz+sizeof(reSF_chk_hdr_t)+MIN_PAYLOAD_SIZE;
	#else
	splitSz = objSz;
	#endif
	
	/* no available memory left. */
	if(reSF_freeQ == NULL)	return NULL;

	/* start allocation from the queue tail. */
	ck = reSF_freeQ->prev;
	/* search from the freed chunk queue. */
	do
	{
		/* find an available entry. */
		#if !KDEBUG_DEMO
		if((ck->ckSize >= objSz) && (ck->ckSize <= splitSz))
		{
			/* initialization and return. The total size "splitSz" will be allocated. */
			ck->ckSize = splitSz;	
			/* delete ck from the freed chunk queue. */
			dlst_del((dlist **)(&reSF_freeQ), (dlist *)ck);
			return ck;
		}
		#else
		if(ck->ckSize == objSz)
		{
			/* delete ck from the freed chunk queue. */
			dlst_del((dlist **)(&reSF_freeQ), (dlist *)ck);
			return ck;
		}
		#endif
		
		/* split operation is required as the chunk size 
		   is larger than the required one. */
		if(ck->ckSize > splitSz)
		{
			/* split a piece from the bottom of this chunk. */
			alloc = (reSF_chk_hdr_t *)((uint16_t)ck + ck->ckSize - objSz);
			alloc->ckSize = objSz;
			/* new chunk update. */
			ck->ckSize -= objSz;
			return alloc;
		}
		
		/* check the next one */
		ck = ck->prev;
	} while(ck != reSF_freeQ->prev);
	
	return NULL;
}


/**
 * @brief fragment assembling for the MIROS SF allocation
 *	Assemble all the fragments and update the references.
 */
void
fragment_assemble(void)
{
	HAS_CRITICAL_SECTION;
	reSF_chk_hdr_t *frgmCk = reSF_freeQ->prev, *p;
	uint8_t *mvFrom, *mvTo;
	uint8_t mvSize, i;
	
	/* if no fragments, return directly. */
	if(frgmCk == reSF_freeQ)	return;
	
	/* assign initialized "mvTo" address. */
	mvTo = (uint8_t *)((uint16_t)frgmCk + frgmCk->ckSize - 1);
	
	/* assemble all fragments one by one. */
	for(; frgmCk != reSF_freeQ; )
	{
		/* assign mvStart. */
		mvFrom = (uint8_t *)((uint16_t)frgmCk - 1);
		
		/* get the size to be moved. */
		mvSize = (uint16_t)frgmCk - ((uint16_t)frgmCk->prev + frgmCk->prev->ckSize);
		
		ENTER_CRITICAL_SECTION;
		/* chunks will be moved. 
		   Starting from chunk "frgmCk - mvSize", Ending to chunk "frgmCk".
		   update the references of these chunks. 
		   Update value is the size between "mvTo" and "frgmCk", but not the "mvSize". */
		for(p = (reSF_chk_hdr_t *)((uint16_t)frgmCk - mvSize); p != frgmCk; p =(reSF_chk_hdr_t *)((uint16_t)p + p->ckSize))
			*(p->ckRef) = *(p->ckRef) + ((uint16_t)mvTo + 1 - (uint16_t)frgmCk);
		
		/* get next free chunk before data shifting. */
		frgmCk = frgmCk->prev;
		
		/* shift the chunks to clean up the memory fragments. 
		   don't change the value of "to" since it will be used for the next step.
		   value of "mvTo" has been updated after this shifting.  */		
		for(i = 0; i < mvSize; i++)
			*mvTo-- = *mvFrom--;
		LEAVE_CRITICAL_SECTION;
	}
}


/**
 * @brief Free a chunk
 *		Add the freed chunk into the free list reSF_freeQ. 
 *		If two freed objects are adjacent, coalesce them.
 *		Release the reference after a chunk is released.
 *
 * \param memRF  Reference of the object to be released.
 * 
 */
void
mem_free(uint16_t *memRF)
{
	reSF_chk_hdr_t *chuk, *ck = reSF_freeQ;

	/* locates the chunk header position. */
	chuk = (reSF_chk_hdr_t *)(*memRF - sizeof(reSF_chk_hdr_t));

	/* remove this one from the allocated list "reSF_allocQ". */
	#if KDEBUG_DEMO
	reSF_chk_hdr_t *lst;
	/* delete from the header */
	if(reSF_allocQ == chuk)	reSF_allocQ = chuk->next;
	/* delete the free item */
	for(lst = reSF_allocQ; lst != NULL; lst = lst->next)
	{
		if(lst->next == chuk)
			lst->next = chuk->next;	
	}
	#endif	// KDEBUG_DEMO
	

	/* If the queue is empty. */
	if(reSF_freeQ == NULL)  {
		reSF_freeQ = chuk;
		reSF_freeQ->prev = reSF_freeQ->next = reSF_freeQ;
		return;
	}
	
	/* locate the insertion position. */
	while(ck < chuk)	{
		/* check next until find the available position. */
		ck = ck->next;
		/* if comes to the ends, then break, and will insert to the queue tail. */
		if(ck == reSF_freeQ)	break;
	}
	
	/* insert "chuk" in front of "ck". */
	dlst_insert((dlist *)chuk, (dlist *)ck);
	/* update the queue	head. */
	if(chuk < reSF_freeQ)		reSF_freeQ =  chuk;
	
	/* debug information, to clear the data of the freed chunk. */
	#if DEBUG_SUPPORT
		memSFfree_debug(chuk);
	#endif
	
	/* coalesce two free chunks if they are adjacent.
	   firstly, upper-address merging, merge "chuk & chuk->next".
	   later, lower-address merging, merge "chuk->prev & chuk". */
	dlst_merge((dlist **)(&reSF_freeQ), (dlist *)chuk, (dlist *)chuk->next);
	dlst_merge((dlist **)(&reSF_freeQ), (dlist *)chuk->prev, (dlist *)chuk);
	
	/* release the reference. */
	*memRF = 0;
}


#if DEBUG_SUPPORT
#if MEM_REACTIVE_SF
/**
 * @brief Debug for SF allocation, clear the data in the new freed chunk.
 */
void
memSFfree_debug(reSF_chk_hdr_t *chuk)
{
	uint8_t i, *p = (uint8_t *)((uint16_t)chuk + sizeof(reSF_chk_hdr_t));
	for(i = 0; i < (chuk->ckSize - sizeof(reSF_chk_hdr_t)); i++)
		*p++ = 0;
};

#endif
#endif

#endif
