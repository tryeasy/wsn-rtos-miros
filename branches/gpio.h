/**
 * @file GPIO.h
 *
 * @author    Xing Liu  (http://edss.isima.fr/sites/smir/)
 * @author    LIMOS Laboratory - UMR CNRS 6158: http://edss.isima.fr
 * @author    Supported email: liu@isima.fr
 */

/* Prevent double inclusion */
#ifndef _GPIO_H
#define _GPIO_H

/******************************************************************************
* void gpioX_set() sets GPIOX pin to logical 1 level.
* void gpioX_clr() clears GPIOX pin to logical 0 level.
* void gpioX_make_in makes GPIOX pin as input.
* void gpioX_make_in makes GPIOX pin as output.
* uint8_t gpioX_read() returns logical level GPIOX pin.
* uint8_t gpioX_state() returns configuration of GPIOX port.
*******************************************************************************/
#define HAL_ASSIGN_PIN(name, port, bit) \
INLINE void  GPIO_##name##_set()         {PORT##port |= (1 << bit);} \
INLINE void  GPIO_##name##_clr()         {PORT##port &= ~(1 << bit);} \
INLINE uint8_t  GPIO_##name##_read()     {return (PIN##port & (1 << bit)) != 0;} \
INLINE uint8_t  GPIO_##name##_state()    {return (DDR##port & (1 << bit)) != 0;} \
INLINE void  GPIO_##name##_make_out()    {DDR##port |= (1 << bit);} \
INLINE void  GPIO_##name##_make_in()     {DDR##port &= ~(1 << bit); PORT##port &= ~(1 << bit);} \
INLINE void  GPIO_##name##_make_pullup() {PORT##port |= (1 << bit);}\
INLINE void  GPIO_##name##_toggle()      {PORT##port ^= (1 << bit);}

/******************************************************************************
                   Inline static functions section
******************************************************************************/
HAL_ASSIGN_PIN(B0, B, 0);
HAL_ASSIGN_PIN(B1, B, 1);
HAL_ASSIGN_PIN(B2, B, 2);
HAL_ASSIGN_PIN(B3, B, 3);
HAL_ASSIGN_PIN(B4, B, 4);
HAL_ASSIGN_PIN(B5, B, 5);
HAL_ASSIGN_PIN(B6, B, 6);
HAL_ASSIGN_PIN(B7, B, 7);

HAL_ASSIGN_PIN(D0, D, 0);
HAL_ASSIGN_PIN(D1, D, 1);
HAL_ASSIGN_PIN(D2, D, 2);
HAL_ASSIGN_PIN(D3, D, 3);
HAL_ASSIGN_PIN(D4, D, 4);
HAL_ASSIGN_PIN(D5, D, 5);
HAL_ASSIGN_PIN(D6, D, 6);
HAL_ASSIGN_PIN(D7, D, 7);

HAL_ASSIGN_PIN(E0, E, 0);
HAL_ASSIGN_PIN(E1, E, 1);
HAL_ASSIGN_PIN(E2, E, 2);
HAL_ASSIGN_PIN(E3, E, 3);
HAL_ASSIGN_PIN(E4, E, 4);
HAL_ASSIGN_PIN(E5, E, 5);
HAL_ASSIGN_PIN(E6, E, 6);
HAL_ASSIGN_PIN(E7, E, 7);

HAL_ASSIGN_PIN(F0, F, 0);
HAL_ASSIGN_PIN(F1, F, 1);
HAL_ASSIGN_PIN(F2, F, 2);
HAL_ASSIGN_PIN(F3, F, 3);
HAL_ASSIGN_PIN(F4, F, 4);
HAL_ASSIGN_PIN(F5, F, 5);
HAL_ASSIGN_PIN(F6, F, 6);
HAL_ASSIGN_PIN(F7, F, 7);

HAL_ASSIGN_PIN(G0, G, 0);
HAL_ASSIGN_PIN(G1, G, 1);
HAL_ASSIGN_PIN(G2, G, 2);
HAL_ASSIGN_PIN(G3, G, 3);
HAL_ASSIGN_PIN(G4, G, 4);
HAL_ASSIGN_PIN(G5, G, 5);

HAL_ASSIGN_PIN(I2C_CLK, D, 0);
HAL_ASSIGN_PIN(I2C_DATA, D, 1);

HAL_ASSIGN_PIN(ADC_INPUT_0, F, 0);
HAL_ASSIGN_PIN(ADC_INPUT_1, F, 1);
HAL_ASSIGN_PIN(ADC_INPUT_2, F, 2);
HAL_ASSIGN_PIN(ADC_INPUT_3, F, 3);
HAL_ASSIGN_PIN(ADC_INPUT_4, F, 4);
HAL_ASSIGN_PIN(ADC_INPUT_5, F, 5);
HAL_ASSIGN_PIN(ADC_INPUT_6, F, 6);
HAL_ASSIGN_PIN(ADC_INPUT_7, F, 7);

HAL_ASSIGN_PIN(USART0_RXD, E, 0);
HAL_ASSIGN_PIN(USART0_TXD, E, 1);
HAL_ASSIGN_PIN(USART0_EXTCLK, E, 2);

HAL_ASSIGN_PIN(USART1_RXD, D, 2);
HAL_ASSIGN_PIN(USART1_TXD, D, 3);
HAL_ASSIGN_PIN(USART1_EXTCLK, D, 5);
HAL_ASSIGN_PIN(USART_RTS, D, 4);
HAL_ASSIGN_PIN(USART_CTS, D, 5);

HAL_ASSIGN_PIN(IRQ_0, D, 0);
HAL_ASSIGN_PIN(IRQ_1, D, 1);
HAL_ASSIGN_PIN(IRQ_2, D, 2);
HAL_ASSIGN_PIN(IRQ_3, D, 3);
HAL_ASSIGN_PIN(IRQ_4, E, 4);
HAL_ASSIGN_PIN(IRQ_5, E, 5);
HAL_ASSIGN_PIN(IRQ_6, E, 6);
HAL_ASSIGN_PIN(IRQ_7, E, 7);

// Macros for the SPI_CS pin manipulation.
HAL_ASSIGN_PIN(SPI_CS, G, 5);
// Macros for the SPI_CS pin manipulation.
HAL_ASSIGN_PIN(HW_SPI_CS, B, 0);
// Macros for the SPI_SCK pin manipulation.
HAL_ASSIGN_PIN(SPI_SCK, B, 1);
// Macros for the SPI_MOSI pin manipulation.
HAL_ASSIGN_PIN(SPI_MOSI, B, 2);
// Macros for the SPI_MISO pin manipulation.
HAL_ASSIGN_PIN(SPI_MISO, B, 3);

#endif /* _GPIO_H */

/* eof gpio.h */
