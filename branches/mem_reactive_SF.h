/**
 * @file mem_reactive_SF.h
 *
 * @brief  header for mem_reactive_SF.c
 *
 * @author    Xing Liu  (http://edss.isima.fr/sites/smir/)
 * @author    LIMOS Laboratory - UMR CNRS 6158: http://edss.isima.fr
 * @author    Supported email: liu@isima.fr
 */

/* Prevent double inclusion */
#ifndef _MEM_REACTIVE_SF_H_
#define _MEM_REACTIVE_SF_H_ 
 
/* === Includes ============================================================= */
#include "board.h"
#include "kernel.h"
#include "sys_config.h"
#include "qlist_proc.h"

#if	MEM_REACTIVE_SF
/* === Macros =============================================================== */
/* the minimum size of a new split chunk should be larger than MIN_PAYLOAD_SIZE. */
#define MIN_PAYLOAD_SIZE	8
/* reference number, maximum allocation. */
#define REF_NUM		20
/* debug option for fragment assembling test. */
#define FRAG_ASSMBL_DEBUG	0


/* === Types ================================================================ */
/* double link list is used as memory coalescence is needed. */
typedef __ALIGNED2 struct reSF_chk_hdr
{
	struct reSF_chk_hdr *prev;	/* to previous free chunk */
	struct reSF_chk_hdr *next;	/* to next free chunk */
	uint16_t ckSize;			/* size of whole chunk including the chuk_hdr */
	uint16_t *ckRef;			/* the address of the reference to this chunk. */
	#if KDEBUG_DEMO
	uint8_t thrd_id;
	#endif
} reSF_chk_hdr_t;


/* === GLOBALS ============================================================= */
extern reSF_chk_hdr_t* reSF_freeQ;
extern uint16_t* reSF_Ref[];
extern reSF_chk_hdr_t* reSF_allocQ;

extern uint16_t* mem_alloc(uint8_t objSz);
extern void* mem_alloc_proc(uint8_t objSz);
extern void fragment_assemble(void);
extern void mem_free(uint16_t *memRF);
extern void memSFfree_debug(reSF_chk_hdr_t *chuk);

/* === Prototypes =========================================================== */



#endif
#endif



