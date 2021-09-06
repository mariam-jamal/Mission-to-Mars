/*
 * io.c
 *
 *  Created on: Apr 25, 2016
 *      Author: laurent
 */

#include "stm32f0xx.h"
extern UART_HandleTypeDef huart1;

int __io_putchar(int ch)
{
    return HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, 1);
}

int __io_getchar(void)
{
return 0;
}
