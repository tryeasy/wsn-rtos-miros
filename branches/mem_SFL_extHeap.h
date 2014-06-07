/**
 * @file mem_SFL_extHeap.h
 *
 * @brief  header for mem_SFL_extHeap.c
 *
 * @author    Xing Liu  (http://edss.isima.fr/sites/smir/)
 * @author    LIMOS Laboratory - UMR CNRS 6158: http://edss.isima.fr
 * @author    Supported email: liu@isima.fr
 */

/* Prevent double inclusion */
#ifndef _mem_SFL_EXTHEAP_H_
#define _mem_SFL_EXTHEAP_H_ 
 
/* === Includes ============================================================= */
#include "board.h"
#include "evt_driven_sched.h"
#include "qlist_proc.h"

#if	MEM_SFL
/* === Macros =============================================================== */
/* the minimum size of a new split chunk should be larger than MIN_PAYLOAD_SIZE. */
#define MIN_PAYLOAD_SIZE	8


/* === Types ================================================================ */
typedef __ALIGNED2 struct sfl_extHeapHdr
{
	struct sfl_extHeapHdr *prev;		/* to previous free chunk */
	struct sfl_extHeapHdr *next;		/* to next free chunk */
	uint16_t ckSize;					/* size of chunk, including the chuk_hdr */
} sfl_extHpHdr_t;


/* === GLOBALS ============================================================= */
extern sfl_extHpHdr_t *hpFreeQ;
extern void* heapSaddr;


/* === Prototypes =========================================================== */
/* memory management */
extern void* memSFL_extHeap_alloc(uint8_t objSz);
extern void memSFL_extHeap_free(void *mem);

#endif
#endif
