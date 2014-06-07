/**
 * @file mem_proactive_SF.h
 *
 * @brief  header for mem_proactive_SF.c
 *
 * @author    Xing Liu  (http://edss.isima.fr/sites/smir/)
 * @author    LIMOS Laboratory - UMR CNRS 6158: http://edss.isima.fr
 * @author    Supported email: liu@isima.fr
 */

/* Prevent double inclusion */
#ifndef _MEM_PROACTIVE_SF_H_
#define _MEM_PROACTIVE_SF_H_ 
 
/* === Includes ============================================================= */
#include "board.h"
#include "kernel.h"
#include "sys_config.h"
#include "qlist_proc.h"
#include "typedef.h"

#if	MEM_PROACTIVE_SF
/* === Macros =============================================================== */
/* reference number, maximum allocation. */
#define REF_NUM		20


/* === Types ================================================================ */
typedef __ALIGNED2 struct proSF_chk_hdr
{
	uint16_t *chk_ref;			/* pointer linked to reference. */
	uint16_t chk_size;			/* chunk size, including header "proSF_chk_hdr_t", used when chunk removing, etc. */
} proSF_chk_hdr_t;

/* === GLOBALS ============================================================= */
extern void* leftHpSaddr;
extern uint16_t* proSF_Ref[];

/* === Prototypes =========================================================== */
extern uint16_t* mem_alloc(uint8_t reqSize);
extern void mem_free(uint16_t *chkMem);


#endif
#endif



