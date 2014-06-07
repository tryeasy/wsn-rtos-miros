/**
 * @file mem_proactive_SF.c
 *
 * @brief  MIROS sequential fit allocator.
 *			In MIROS, the fragment assembling mechanism is implemented for the sequential fit allocator. 
 *			Currently, two assembling approaches have been realized: the proactive fragment assembling and 
 *			the reactive fragment assembling.
 *
 *			The memory fragments are assembled once they are appeared in the proactive fragment assembling. 
 *			By this way, the fragments can be prevented to occur, 
 *			and the new allocation can be performed immediately with constant allocation time.
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

#if MEM_PROACTIVE_SF
#include "mem_proactive_SF.h"
/* === TYPES =============================================================== */


/* === MACROS ============================================================== */


/* === GLOBALS ============================================================= */
/* references for the allocated chunks. */
uint16_t* proSF_Ref[REF_NUM];	

/* starting address of left heap. */
void* leftHpSaddr;	

/* === PROTOTYPES ========================================================== */


/* === IMPLEMENTATION ====================================================== */
/**
 * @brief Memory allocated by MIROS sequential fit (proactive fragments assembled).
 *			If a new object needs to be allocated, it will be performed from the starting address of the free memory space. 
 *			If an allocated object needs to be de-allocated, a memory coalescence will be done to the adjacent chunks of this object.
 *			As the addresses of some objects will change after the coalescing operation, 
 *			all the allocated objects need to be accessed indirectly by the reference pointers.
 *
 * \param reqSize	Required size to be allocated.
 * \return			Address of the reference header for the allocated chunk.
 */
uint16_t*
mem_alloc(uint8_t reqSize)
{
	proSF_chk_hdr_t* chk_alloc = NULL;
	uint8_t chkSize = reqSize + sizeof(proSF_chk_hdr_t);
	uint8_t id;
	
	/* Check if we have enough memory left for this allocation. */
	if((uint16_t)((uint16_t)leftHpSaddr+chkSize) > HEAP_EADDR)	return NULL;

	/* allocate a reference for this chunk. */
	for(id = 0; id < REF_NUM; id++)
		if(proSF_Ref[id] == NULL)	break;
	/* references are used up, maximum allocation. */
	if(id == REF_NUM)	return NULL;

	/* memory space to be allocated. */
	chk_alloc = (proSF_chk_hdr_t *)leftHpSaddr;
	/* init this new chunk. */
	chk_alloc->chk_size = chkSize;
	proSF_Ref[id] = (void *)chk_alloc + sizeof(proSF_chk_hdr_t);
	chk_alloc->chk_ref = (uint16_t *)&proSF_Ref[id];

	/* update new heap starting address. */
	leftHpSaddr += chkSize;

	/* return chunk address. */
	return chk_alloc->chk_ref;
}

/**
 * @brief Free a chunk
 *		The memory fragments are assembled once they are appeared. 
 *		By this way, the fragment problem can be prevented to occur, 
 *		and the new allocation can be done immediately with constant response time. 
 *
 * \param chkMem  Address of the chunk to be released.
 * 
 */
void
mem_free(uint16_t *chkMem)
{
	proSF_chk_hdr_t *m;
	uint8_t *mvSaddr, *mvTo;
	uint16_t i, sft_size;
	
	/* "*chkMem" will point to the data payload, 
	   assign "mvTo" and "mvSaddr". */
	mvTo = (uint8_t *)(*chkMem - sizeof(proSF_chk_hdr_t));
	sft_size = ((proSF_chk_hdr_t *)mvTo)->chk_size;
	mvSaddr = (uint8_t *)(mvTo + sft_size) ;
	
	/* release the reference. */
	*(((proSF_chk_hdr_t *)mvTo)->chk_ref) = 0;
	
	/* update the references of the other chunks before memory coalescence. */
	for(m = (proSF_chk_hdr_t *)mvSaddr; m != leftHpSaddr; m = (proSF_chk_hdr_t *)((uint16_t)m + m->chk_size))
		*(m->chk_ref) -= ((proSF_chk_hdr_t *)mvTo)->chk_size;
	
	/* memory coalescence. */
	for(i = 0; i < ((uint16_t)leftHpSaddr - (uint16_t)mvSaddr); i++)
		*(mvTo + i) = *(mvSaddr + i);
	
	/* update "leftHpSaddr". */
	leftHpSaddr -= sft_size;
}

#endif
