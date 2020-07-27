#ifndef INC_USART_H_
#define INC_USART_H_

#include "main.h"

#define MAX_UART_BUFF_SIZE 1024
struct _UART_RxBuffer {
	unsigned char uRx_Data[MAX_UART_BUFF_SIZE];
	unsigned char *pRx_Data;
	int uLength;
};
typedef struct _UART_RxBuffer UART_RxBuffer;

extern UART_RxBuffer USART1_Buffer, USART2_Buffer;

#define RECV_TIMEOUT 10
#define RECV_EOF '\n'

void call_on_USART_IRQHandler(UART_HandleTypeDef *huart, UART_RxBuffer *rxbuff);

__weak void USART_Rx_Callback(UART_HandleTypeDef *huart, UART_RxBuffer *rxbuff);

unsigned int uart_fputs(UART_HandleTypeDef *huart, const char *msg);

#endif /* INC_USART_H_ */
