/*
 * uart_task.h
 *
 *  Created on: Dec 23, 2024
 *      Author: Trung
 */

#ifndef UART_TASK_H_
#define UART_TASK_H_

void usart_init(void);
void send_usart_data(const char *data);
int data_periodDHT();
int data_periodADV();

#endif /* UART_TASK_H_ */
