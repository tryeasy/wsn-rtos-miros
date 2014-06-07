/**
 * @file qlist_proc.h
 *
 * @brief  header for file qlist_proc.c
 *
 * @author	  Xing Liu  (http://edss.isima.fr/sites/smir/) 
 * @author    LIMOS Laboratory - UMR CNRS 6158: http://edss.isima.fr
 * @author    Supported email: liu@isima.fr
 */
/* Prevent double inclusion */
#ifndef _QLIST_PROC_H_
#define _QLIST_PROC_H_

/* === INCLUDES ============================================================ */
#include "board.h"
#include "typedef.h"
#include "kdebug.h"

/* === TYPES =============================================================== */
/* double link queue list. */
typedef struct dList
{
	struct dList *prev;
	struct dList *next;	
	uint16_t size;
} dlist;

/* single link queue list. */
typedef struct sList
{
	struct sList *next;
} sQList;

/* === MACROS ============================================================== */


/* === GLOBALS ============================================================= */


/* === PROTOTYPES ========================================================== */
extern void dlst_del(dlist **Qhead, dlist *list);
extern void dlst_insert(dlist *list, dlist *pos);
extern void dlst_merge(dlist **Qhead, dlist *listA, dlist *ckB);
extern bool isAlreadyInQueue(sQList *Qhead, sQList *item);
extern sQList *findPrevEntry(sQList *Qhead,  sQList *item);
extern void RemoveEntryFromQ(sQList **head, sQList *prev, sQList *item);
#endif



