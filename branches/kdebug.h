/**
 * @file kdebug.h
 *
 * @author    Xing Liu  (http://edss.isima.fr/sites/smir/)
 * @author    LIMOS Laboratory - UMR CNRS 6158: http://edss.isima.fr
 * @author    Supported email: liu@isima.fr
 */

/* Prevent double inclusion */
#ifndef _KDEBUG_H_
#define _KDEBUG_H_ 
 
/* === Includes ============================================================= */
#include "board.h"


/* === Macros =============================================================== */
/**
 * void HSDTVI_X_set() sets HSDTVI_X pin to logical 1 level.
 * void HSDTVI_X_clr() clears HSDTVI_X pin to logical 0 level.
 * void HSDTVI_X_make_in makes HSDTVI_X pin as input.
 * void HSDTVI_X_make_out makes HSDTVI_X pin as output.    
 * uint8_t HSDTVI_X_read() returns logical level HSDTVI_X pin.
 * uint8_t HSDTVI_X_state() returns configuration of HSDTVI_X port.
 *
 */
#define HSDTVI_ASSIGN_PIN(name, port, bit) \
INLINE void  HSDTVI_##name##_set()         {PORT##port |= (1 << bit);} \
INLINE void  HSDTVI_##name##_clr()         {PORT##port &= ~(1 << bit);} \
INLINE uint8_t  HSDTVI_##name##_read()     {return (PIN##port & (1 << bit)) != 0;} \
INLINE uint8_t  HSDTVI_##name##_state()    {return (DDR##port & (1 << bit)) != 0;} \
INLINE void  HSDTVI_##name##_make_out()    {DDR##port |= (1 << bit);} \
INLINE void  HSDTVI_##name##_make_in()     {DDR##port &= ~(1 << bit); PORT##port &= ~(1 << bit);} \
INLINE void  HSDTVI_##name##_make_pullup() {PORT##port |= (1 << bit);}\
INLINE void  HSDTVI_##name##_toggle()      {PORT##port ^= (1 << bit);}\

HSDTVI_ASSIGN_PIN(0, E, 0);  	
HSDTVI_ASSIGN_PIN(1, E, 1);
HSDTVI_ASSIGN_PIN(2, E, 2);
HSDTVI_ASSIGN_PIN(3, E, 3);
HSDTVI_ASSIGN_PIN(4, E, 4);
HSDTVI_ASSIGN_PIN(5, E, 5);
HSDTVI_ASSIGN_PIN(6, E, 6);
HSDTVI_ASSIGN_PIN(7, E, 7);
HSDTVI_ASSIGN_PIN(WR, G, 2);


/* === Types ================================================================ */
/* os_start.c */
typedef enum
{
	START_ID,
	thrd_sched_debugID,
	evt_sched_debugID,
	memAlloc_debugID,
	END_ID,
} kdebugCmdId_t;





/* === GLOBALS ============================================================= */





/* === Prototypes =========================================================== */
extern void kDebug_init(void);
extern void kDebug8bit(uint8_t val);
extern void kDebug16bit(uint16_t val);
extern void kDebug32bit(uint32_t val);
extern void kOutArray(void *a, uint8_t len);

#endif
