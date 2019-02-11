

#ifndef _UART_H_
#define _UART_H_

#define UART_BUFFER_SIZE 128

void uart_init(void);
//int uart_send(char *str);
int uart_receive(char **buf);

#endif
