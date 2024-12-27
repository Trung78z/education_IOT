/*
 * usart_task.h
 *
 *  Created on: Dec 23, 2024
 *      Author: Trung
 */

#ifndef USART_TASK_H_
#define USART_TASK_H_

void usart_init(void);
void send_usart_data(const char *data);
int data_periodDHT();
int data_periodADV();

#endif /* USART_TASK_H_ */
