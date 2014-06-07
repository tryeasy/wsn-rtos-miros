/**
 * @file ipc.c
 *
 * @brief  Inter-process communication in the MIROS.
 *		   All the communications in the MIROS are done through the universal interfaces "send" and "recv".
 *
 * @author    Xing Liu  (http://edss.isima.fr/sites/smir/)
 * @author    LIMOS Laboratory - UMR CNRS 6158: http://edss.isima.fr
 * @author    Supported email: liu@isima.fr
 */

/* === INCLUDES ============================================================ */
#include "board.h"
#include "timer_ACV.h"
#include "lowlevel_init.h"
#include "usart.h"
#include "gpio.h"
#include "kdebug.h"
#include "demoTasks.h"
#include "ipc.h"
#include "mem_SFL.h"
#include "mem_reactive_SF.h"

/* === TYPES =============================================================== */


/* === MACROS ============================================================== */


/* === GLOBALS ============================================================= */
/* The initial order should match the order in the enum "ipcID_t".
   This table can aslo be put into FLASH (by using modifier "PROGMEM"). */
ipc_register_t IPC_handlers[] =
{
	{&usartSndStatus, (ipcHandler_t)usartSendString, &usartSendQ, (ipcHandler_t)usartRecvString, &uasrtRecvQ},
};

/* === PROTOTYPES ========================================================== */


/* === IMPLEMENTATION ====================================================== */
/**
 * @brief The unified IPC sending interface in the MIROS.
 * \param id   IPC ID, to indicate the IPC sending or receiving component.
 * \param msg  Message to be sent.
 * \param size Message length.
 * \param opt  Sending option, can be a pointer or a 16-bit word.
 *
 * All the IPC sending operations in the MIROS are done through this interface.
 * If the IPC port is free, send the message out. 
 * If the IPC port is busy, put the message into the buffering queue.
 *
 */
uint8_t
send(ipcID_t id, void *msg, uint8_t size, uint16_t opt)
{
	ipc_msgQ_t *msgQ, **sendQ, *q;
	uint8_t *ipcStatus;
	
	/* allocate a memory space for the IPC "ipc_msgQ_t" structure. */
	msgQ = ipc_msgQ_alloc(msg, size, opt);
	if(msgQ == NULL)	return MEM_ALLOC_ERROR;

	/* get the corresponding IPC sending queue. Read from the AVR FLASH. */
	sendQ = IPC_handlers[id].ipcSendQ;

	/* add this message to the IPC sending queue. */
	if(*sendQ == NULL)
		*sendQ = msgQ;
	else
	{
		for(q = *sendQ; q->next != NULL; q = q->next);
		q->next = msgQ;
	}
	
	/* send the message out, and set this IPC status to be BUSY. */
	ipcStatus = IPC_handlers[id].status;
	*ipcStatus = IPC_BUSY;
	IPC_handlers[id].send_handler();
			
	return 0;
}

/**
 * @brief Send next message in the IPC message queue.
 * \param id   IPC unique ID, all the IPC operations are done through this ID.
 *
 * If sending from a hardware port, an interruption will be generated after a message has been sent out.
 * Inside the ISR, this function will be called to send the next message.
 */
void
ipc_sendNextMsg(ipcID_t id)
{
	ipc_msgQ_t *sendQ;
	uint8_t *status;
	
	/* get the IPC send queue. */
	sendQ = IPC_handlers[id].ipcSendQ;
	status = IPC_handlers[id].status;
	
	/* If next message is NULL, set the IPC port to be FREE. 
	   Else, continue sending the next message. */
	if(sendQ == NULL)
		*status = IPC_FREE;
	else
		IPC_handlers[id].send_handler();
}


/**
 * @brief The universal IPC receiving interface in the MIROS.
 * \param id   IPC ID, to indicate the IPC sending or receiving component.
 * \param msg  Message to be received.
 * \param size Message length.
 * \param opt  Sending option, can be a pointer, or a 16-bit word.
 *
 * All the message receiving operations in the MIROS are done through this interface.
 * After the message is received, put it into the related buffering queue.
 *
 */
uint8_t
recv(ipcID_t id, void *msg, uint8_t size, uint16_t opt)
{
	ipc_msgQ_t **recvQ, *q;
	#if	MEM_SFL
		ipc_msgQ_t *msgQ;
	#endif
	#if	MEM_REACTIVE_SF
		uint16_t *refMsg;
	#endif	
		
	/* allocate a memory space for the IPC "ipc_msgQ_t" structure. */
	#if MEM_SFL
		msgQ = ipc_msgQ_alloc(msg, size, opt);
		if(msgQ == NULL)	return MEM_ALLOC_ERROR;
	#endif
	#if MEM_REACTIVE_SF
		refMsg = ipc_msgQ_alloc(msg, size, opt);
		if(refMsg == NULL)	return MEM_ALLOC_ERROR;
	#endif

	/* get the corresponding IPC sending queue. Read from the AVR FLASH. */
	recvQ = IPC_handlers[id].ipcRecvQ;

	/* add this message to the IPC sending queue. */
	if(*recvQ == NULL)
	{
		#if MEM_SFL
			*recvQ = msgQ;
		#endif
		#if MEM_REACTIVE_SF
			*recvQ = (ipc_msgQ_t *)*refMsg;
		#endif
	}
	else
	{
		for(q = *recvQ; q->next != NULL; q = q->next);
		#if MEM_SFL
			q->next = msgQ;
		#endif
		#if MEM_REACTIVE_SF
			q->next = (ipc_msgQ_t *)*refMsg;
		#endif
	}
		
	/* put the message into the buffer, and then call the receiving handler. */
	IPC_handlers[id].recv_handler();
		
	return 0;
}

/**
 * @brief Allocate an IPC message structure and then init it.
 * \param msg  Message to be received.
 * \param size Message length.
 * \param opt  Sending option, can be a pointer, or a 16-bit word.
 * \return	return the allocated memory address.
 *
 */
void *
ipc_msgQ_alloc(void *msg, uint8_t size, uint16_t opt)
{
	/* allocate a message structure, and then init it. */
	#if MEM_SFL
		ipc_msgQ_t *msgQ = NULL;
		msgQ = mem_alloc(&ipc_pt);
		if(msgQ != NULL)
		{
			msgQ->data = msg;
			msgQ->size = size;
			msgQ->option = opt;
			msgQ->next = NULL;
			return msgQ;
		}
	#endif
	#if MEM_REACTIVE_SF
		uint16_t *refMsg = NULL;
		refMsg = mem_alloc(sizeof(ipc_msgQ_t));
		if(refMsg != NULL)
		{
			((ipc_msgQ_t *)(*refMsg))->data = msg;
			((ipc_msgQ_t *)(*refMsg))->size = size;
			((ipc_msgQ_t *)(*refMsg))->option = opt;
			((ipc_msgQ_t *)(*refMsg))->next = NULL;
			return refMsg;
		}
	#endif	

	return NULL;
}
