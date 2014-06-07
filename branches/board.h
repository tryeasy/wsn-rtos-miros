/**
 * @file board.h
 *
 * @brief header related to the hardware
 *
 * @author    LIMOS Laboratory - UMR CNRS 6158: http://edss.isima.fr
 * @author    Supported email: liu@isima.fr
 */

/* Prevent double inclusion */
#ifndef _BOARD_H_
#define _BOARD_H_ 
 
/* === Includes ============================================================= */
#include "sys_config.h"
#include "typedef.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/boot.h>
//#include <util/delay.h>
//#include <avr/power.h>

#include <stdint.h>
#include <stdbool.h>

/* === Macros =============================================================== */
/* Macroses to accept memory I/O registers for AVR Mega family */
#define MMIO_BYTE(mem_addr) (*(volatile uint8_t *)(mem_addr))
#define MMIO_WORD(mem_addr) (*(volatile uint16_t *)(mem_addr))


/**
 * @brief interrupt
 */
#define ENABLE_GLOBAL_INTERRUPTS         sei()
#define DISABLE_GLOBAL_INTERRUPTS        cli()

#define HAS_CRITICAL_SECTION       register uint8_t _prev_
#define ENTER_CRITICAL_SECTION  \
asm volatile ( \
	"in %0, __SREG__"   "\n\t" \
	"cli"               "\n\t" \
	: "=r" (_prev_) \
	: )

#define LEAVE_CRITICAL_SECTION \
asm volatile ( \
	"out __SREG__, %0"   "\n\t" \
	: \
	: "r" (_prev_) )


/**
 * @brief put hardware to idle mode
 */
#define hardware_sleep()  {         \
	asm volatile ("sleep");         \
	asm volatile ("nop");           \
	asm volatile ("nop");           \
}

/**
 * @brief put hardware to sleep and ensure no interrupt is handled
 */
#define atomic_hardware_sleep() {   \
	asm volatile ("nop");           \
	asm volatile ("nop");           \
}

/* === Types ================================================================ */


/* === GLOBALS ============================================================= */



/* === Prototypes =========================================================== */



#endif


