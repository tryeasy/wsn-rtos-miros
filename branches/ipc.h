/**
 * @file ipc.h
 *
 * @brief  header for file ipc.c.
 *
 * @author	  Xing Liu  (http://edss.isima.fr/sites/smir/)
 * @author    LIMOS Laboratory - UMR CNRS 6158: http://edss.isima.fr
 * @author    Supported email: liu@isima.fr
 */

/* Prevent double inclusion */
#ifndef _IPC_H_
#define _IPC_H_

/* === INCLUDES ============================================================ */


/* === MACROS ============================================================== */
/* Every software or hardware IPC component has a unique ID.
   The "send" and "recv" operations will put the messages to the corresponded message queue
   in terms of this ID. */
typedef enum ipcID
{
	USART_ID,
	WIRELESS_TX_ID,
} ipcID_t;

/* Status of the IPC queue. When the queue is free, send the messages immediately.
   When the queue if busy, store the messages in the buffering queues. */
typedef enum ipc_status
{
	IPC_INIT,
	IPC_BUSY,
	IPC_FREE,
} ipc_status_t;

/* === TYPES =============================================================== */
/* structure for the message sending and receiving operations from the IPC. */
typedef struct ipc_msgQ
{
	struct ipc_msgQ *next;
	uint8_t *data;		/* message to be sent or has been received. */
	uint8_t size;		/* message length. */
	uint16_t option;	/* send or recv option. */
} ipc_msgQ_t;

/* structure to define the IPC send and recv handlers. */
typedef uint8_t (*ipcHandler_t)(void);
typedef struct ipc_register
{
	uint8_t *status;
	ipcHandler_t send_handler;
	void *ipcSendQ;
	ipcHandler_t recv_handler;
	void *ipcRecvQ;
} ipc_register_t;

/* === GLOBALS ============================================================= */
extern uint8_t send(ipcID_t id, void *msg, uint8_t size, uint16_t opt);
extern void ipc_sendNextMsg(ipcID_t id);
extern uint8_t recv(ipcID_t id, void *msg, uint8_t size, uint16_t opt);
extern void *ipc_msgQ_alloc(void *msg, uint8_t size, uint16_t opt);



/* === PROTOTYPES ========================================================== */



#endif

