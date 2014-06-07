/**
 * @file kdebug.c
 *
 * @brief  multi-core platform debugging for MIROS kernel
 *
 * The current debug methods include "printf, serial port sending, debug tool breakpoint, etc."
 * However, they are not effective for the debug on resource-constraint sensor nodes.
 *
 * For MIROS, the OS debug is done by the support of hardware multi-core system.
 * The working board iLive is connected with the debug board Raspberry Pi through the GPIO ports.
 * Every time the iLive node takes some actions, it can send some raw debug code to the GPIO ports, 
 * and then the left debug work will be afforded by the powerful Raspberry board, 
 * e.g., when a memory chunk is allocated on iLive, three bytes debug code "0xAC, 0x20, 0x1A" 
 * will be sent to the GPIO ports. 
 * After Raspberry board receives these codes, it will interpret 
 * "0xAC" as the operation "OS_malloc", "0x20, 0x1A" as the new allocated address 0x201A, 
 * and then transfer these debug results to the PC and display on the PC in the string or graphic format. 
 *
 * @author    Xing Liu  (http://edss.isima.fr/sites/smir/)
 * @author    LIMOS Laboratory - UMR CNRS 6158: http://edss.isima.fr
 * @author    Supported email: liu@isima.fr
 */

/* === INCLUDES ============================================================ */
#include "board.h"
#include "kdebug.h"
#include "avr/delay.h"

/* === TYPES =============================================================== */


/* === MACROS ============================================================== */


/* === GLOBALS ============================================================= */


/* === PROTOTYPES ========================================================== */


/* === IMPLEMENTATION ====================================================== */
/**
 * @brief Init GPIO pin as output.
 */
INLINE void 
kDebug_init(void)
{
	DDRE = 0xFF;	
	PORTE = 0xFF;

	DDRG |= 0x04;
	PORTG |= 0x04;
}

/**
 * @brief 8 bits output unit.
 * \param val  The 8-bit debug code to be sent out from the GPIO.
 *
 * Send 8-bits debug code out from the iLive node GPIO to the debug board.
 */
INLINE void 
kDebug8bit(uint8_t val)
{
	HAS_CRITICAL_SECTION;
	ENTER_CRITICAL_SECTION;

	DDRE = 0xFF;
	PORTE = (val);
	DDRG |= 0x04;
	HSDTVI_WR_toggle();

	LEAVE_CRITICAL_SECTION;
	
	/* short delay to avoid overflow. */
	_delay_ms(10);
}

/**
 * @brief 16 bit output.
 * \param val  The 16-bit debug code to be sent out from the GPIO.
 *
 * Send 16-bits debug code out from the iLive node GPIO to the debug board.
 */
void 
kDebug16bit(uint16_t val)
{
	/* high 8 bits */
	kDebug8bit((uint8_t)((val&0xFF00)>>8));
	/* low 8 bits */
	kDebug8bit(val&0xFF);
}

/**
 * @brief 32 bit output unit.
 * \param val  The 32-bit debug code to be sent out from the GPIO.
 *
 * Send 32-bits debug code out from the iLive node GPIO to the debug board.
 */
void 
kDebug32bit(uint32_t val)
{
	/* high 31-24 bits */
	kDebug8bit((val&0xFF000000)>>24);
	/* high 23-16 bits */
	kDebug8bit((val&0xFF0000)>>16);
	/* low 15-8 bits */
	kDebug8bit((val&0xFF00)>>8);
	/* low 7-0 bits */
	kDebug8bit(val&0xFF);
}

/**
 * @brief array output.
 * \param a    The debug code string to be sent out from the GPIO.
 * \param len  The string length.
 *
 * Send a string debug code out from the iLive node GPIO to the debug board.
 */
void 
kOutArray(void *a, uint8_t len)
{
	uint8_t *p;
	for(p = a; len != 0; len--)
	{
		kDebug8bit(*p++);
	}
}
