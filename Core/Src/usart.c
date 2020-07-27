#include "usart.h"
#include <string.h>

UART_RxBuffer USART1_Buffer, USART2_Buffer;

void call_on_USART_IRQHandler(UART_HandleTypeDef *huart, UART_RxBuffer *rxbuff) {
	HAL_UART_Receive(huart, rxbuff->pRx_Data, 1, RECV_TIMEOUT);

	if (*(rxbuff->pRx_Data) == RECV_EOF) {
		USART_Rx_Callback(huart, rxbuff);
		rxbuff->pRx_Data = rxbuff->uRx_Data;
		rxbuff->uLength = 0;
	}
	else {
		rxbuff->pRx_Data++;
		rxbuff->uLength++;
	}
}

__weak void USART_Rx_Callback(UART_HandleTypeDef *huart, UART_RxBuffer *rxbuff) {
	if (huart == (&huart1)) {
		// Deal With Rx 1
	}
	else if (huart == (&huart2)) {
		// Deal With Rx 2
	}
	else {
		// Not Rx
	}
}

// stdio in uart
unsigned int uart_fputs(UART_HandleTypeDef *huart, const char *msg) {
	return HAL_UART_Transmit(huart, (uint8_t *)msg, strlen(msg), 10);
}
