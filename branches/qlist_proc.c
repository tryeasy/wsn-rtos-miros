/**
 * @file qlist_proc.c
 *
 * @brief  kernel library for the queue list operation.
 *
 * @author	  Xing Liu  (http://edss.isima.fr/sites/smir/) 
 * @author    LIMOS Laboratory - UMR CNRS 6158: http://edss.isima.fr
 * @author    Supported email: liu@isima.fr
 */

/* === INCLUDES ============================================================ */
#include "board.h"
#include "typedef.h"
#include "kdebug.h"
#include "qlist_proc.h"

/* === TYPES =============================================================== */


/* === MACROS ============================================================== */


/* === GLOBALS ============================================================= */


/* === PROTOTYPES ========================================================== */


/* === IMPLEMENTATION ====================================================== */
/**
 * @brief Double link queue delete operation.
 * \param Qhead	  Pointer to the head of the queue.
 * \param list	  The entry to be deleted from this queue.
 *
 * Update the queue header when required.
 */
INLINE void
dlst_del(dlist **Qhead, dlist *list)
{
	HAS_CRITICAL_SECTION;
	/* if only one exist, the queue becomes empty. */
	if(list->next == list)
	{
		/* update the header. */
		*Qhead = NULL;
		return;
	}
	
	ENTER_CRITICAL_SECTION;
	list->prev->next = list->next;
	list->next->prev = list->prev;
	LEAVE_CRITICAL_SECTION;
	/* update the queue header. */
	if(*Qhead == list)
		*Qhead = (*Qhead)->next;
}

/**
 * @brief Double link queue insertion operation.
 * \param list	  The entry to be inserted into.
 * \param pos	  The position for "list" to be inserted.
 *
 * Insert "list" to the front of "pos".
 * The queue head is not updated in this function, 
 * users should be aware of this.
 */
void
dlst_insert(dlist *list, dlist *pos)
{
	HAS_CRITICAL_SECTION;
	ENTER_CRITICAL_SECTION;
	list->next = pos;
	list->prev = pos->prev;
	pos->prev->next = list;
	pos->prev = list;
	LEAVE_CRITICAL_SECTION;
}

/**
 * @brief Merge two queue lists if they are adjacent.
 * \param Qhead	  Pointer to the head of the queue.
 * \param listA	  The 1st entry to be merged.
 * \param listB	  The 2nd entry to be merged.
 *
 * Merge listB into listA, and then delete listB.
 */
void
dlst_merge(dlist **Qhead, dlist *listA, dlist *listB)
{
	/* merge listB into listA, and then delete listB. */
	if(((uint16_t)listA + listA->size) == (uint16_t)listB)	{
		/* delete "listB" */
		dlst_del(Qhead, (dlist *)listB);
		/* update "listA" size */
		listA->size += listB->size;
	}
}

/**
 * @brief Search "item" from the single link queue with the header "Qhead".
 * \param Qhead	  Pointer to the head of the queue.
 * \param item	  Search this item from this queue.
 * \return   Return true if found, or false if not found.
 *
 */
bool
isAlreadyInQueue(sQList *Qhead, sQList *item)
{
	bool result = false;
	sQList *p = Qhead;

	while (NULL != p) {
		if (p == item)  {
			result = true;
			break;
		}
		p = p->next;
	}
	return result;
}

/**
 * @brief Find the entry previous to "item" in the single link queue "Qhead".
 * \param Qhead	  Pointer to the head of the queue.
 * \param item	  Return the entry ahead of "item" in this queue.
 * \Return		  Return the previous entry.
 */
sQList 
*findPrevEntry(sQList *Qhead,  sQList *item)
{	
	sQList *t = Qhead;

	for (; t ;)
	{
		if (t->next == item)
		return t;
		t = t->next;
	}
	return NULL;
}

/**
 * @brief Removes the entry "item" from the single link queue.
 * \param head	  Pointer to the head of the queue.
 * \param prev	  The previous entry before "item".
 * \param item	  Entry to be removed.
 *
 * Use "findPrevEntry" to get the entry previous to "item" firstly.
 */
void
RemoveEntryFromQ(sQList **head, sQList *prev, sQList *item)
{
	if (item == *head)
		/* removing first element of list */
		*head = item->next;
	else
		prev->next = item->next;
	item->next = 0;
}