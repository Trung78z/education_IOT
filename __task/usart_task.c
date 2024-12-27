/*
 * usart_task.c
 *
 *  Created on: Dec 23, 2024
 *      Author: Trung
 */

#include "usart_task.h"

#include "FreeRTOS.h"
#include "task.h"
#include "em_gpio.h"
#include "em_cmu.h"
#include "em_usart.h"
#include "string.h"
#include "dht_task.h"
#include "stdio.h"
#include "___spi_lcd/app_lcd.h"
#include "app.h"
#include "stdint.h"
#include <stdlib.h>
#include "stdbool.h"
#define UART_STACK_SIZE 2048

#define BSP_GPIO_LED1_PORT gpioPortD
#define BSP_GPIO_LED1_PIN 3
#define BSP_GPIO_LED0_PORT gpioPortD
#define BSP_GPIO_LED0_PIN 2

#define BSP_TXPORT gpioPortA
#define BSP_RXPORT gpioPortA
#define BSP_TXPIN 5
#define BSP_RXPIN 6
#define BSP_ENABLE_PORT gpioPortD
#define BSP_ENABLE_PIN 4
#define BAUD_RATE 115200

#define BUFFER_SIZE 1024

static uint16_t periodDHT = 1;
static uint16_t periodADV = 1;

#define BUFLEN 256

static void usart_task(void *arg);

void usart_init(void)
{
  TaskHandle_t xHandle = NULL;
  static StaticTask_t xTaskBuffer;
  static StackType_t xStack[UART_STACK_SIZE];
  xHandle = xTaskCreateStatic(usart_task, "uart task",
                              UART_STACK_SIZE,
                              (void *)NULL,
                              tskIDLE_PRIORITY + 5,
                              xStack, &xTaskBuffer);
  EFM_ASSERT(xHandle != NULL);
  // xTaskCreate(usart_task, "USART Task", UART_STACK_SIZE, NULL, 5, NULL);
}
bool flag = false;
uint8_t buffer[BUFFER_SIZE];
int buffer_index = 0;

// void USART0_RX_IRQHandler(void)
// {
//   uint8_t received_char = USART0->RXDATA;
//   USART_IntClear(USART0, USART_IF_RXDATAV);

//   if (received_char == '\n')
//   {
//     flag = true;
//     buffer[buffer_index] = '\0';
//     buffer_index = 0;
//   }
//   else if (buffer_index < BUFLEN - 1)
//   {
//     buffer[buffer_index++] = received_char;
//   }
//   else
//   {

//     buffer_index = 0;
//   }
// }

static void usart_task(void *arg)
{
  (void)arg;
  init_USART_GPIO();
  init_USART0();
  send_usart_data("\nStarted\n");

  while (1)
  {
    memset(buffer, 0, BUFFER_SIZE);
    buffer_index = 0;
    while (!(USART0->STATUS & USART_STATUS_RXDATAV))
    {
    }
    while (USART0->STATUS & USART_STATUS_RXDATAV && buffer_index < BUFFER_SIZE - 1)
    {
      buffer[buffer_index++] = USART_Rx(USART0);
      if (buffer[buffer_index - 1] == '\n' || buffer[buffer_index - 1] == '\r')
      {
        buffer[buffer_index] = '\0';
        break;
      }
    }

    handle_command_usart((char *)buffer);
  }
}

void send_usart_data(const char *data)
{
  while (!(USART0->STATUS & USART_STATUS_TXBL))
  {
  }
  for (size_t i = 0; i < strlen(data); i++)
  {
    USART_Tx(USART0, data[i]);
  }
}

void handle_command_usart(const char *data)
{
  char *command = strtok((char *)data, " ");
  char *parameter = strtok(NULL, " ");
  if (strcmp(command, "dht") == 0)
  {
    int param_value = atoi(parameter);
    if (param_value < 1000)
    {
      periodDHT = 1000;
    }
    else if (param_value > 15000)
    {
      periodDHT = 15000;
    }
    else
    {
      periodDHT = param_value;
    }
    char message[100];
    sprintf(message, "OK, periodDHT: %dms\r\n", periodDHT);
    send_usart_data(message);
  }
  else if (strcmp(command, "adv") == 0)
  {
    int param_value = atoi(parameter);
    if (param_value < 1000)
    {
      periodADV = 1000;
    }
    else if (param_value > 10000)
    {
      periodADV = 10000;
    }
    else
    {
      periodADV = param_value;
    }
    sl_bl_timing_set_cb(periodADV, periodADV);
    char message[100];
    sprintf(message, "OK, periodADV: %dms\r\n", periodADV);
    send_usart_data(message);
  }
  else if (strcmp(command, "name") == 0)
  {
    printf("OK, Name_ble: %s\n", parameter);
    updateName(parameter);
    char messagecase5[100];
    sprintf(messagecase5, "OK, name__Ble: %s\r\n", parameter);
    send_usart_data(messagecase5);
  }
  else if (strcmp(command, "led") == 0)
  {
    char messagecase6[100];
    sprintf(messagecase6, "OK, LED blinking 5 times\r\n");
    send_usart_data(messagecase6);
    for (int i = 0; i < 5; i++)
    {
      GPIO_PinOutSet(BSP_GPIO_LED0_PORT, 2);
      vTaskDelay(pdMS_TO_TICKS(500));
      GPIO_PinOutClear(BSP_GPIO_LED0_PORT, 2);
      vTaskDelay(pdMS_TO_TICKS(500));
    }
  }
  else if (strcmp(command, "data") == 0)
  {
    float temperature = data_temperature();
    float humidity = data_humidity();
    char message2[100];
    sprintf(message2, "Temperature: %.2fC, Humidity: %.2f%%\r\n", temperature, humidity);
    send_usart_data(message2);
  }
  else
  {
    send_usart_data("ERROR, Unknow word\n");
  }
}

int data_periodDHT()
{
  return periodDHT;
}
int data_periodADV()
{
  return periodADV;
}

void init_USART_GPIO(void)
{
  //  CMU_ClockEnable(cmuClock_USART0, true);
  GPIO_PinModeSet(BSP_TXPORT, BSP_TXPIN, gpioModePushPull, 1);
  GPIO_PinModeSet(BSP_RXPORT, BSP_RXPIN, gpioModeInput, 0);
  GPIO_PinModeSet(BSP_ENABLE_PORT, BSP_ENABLE_PIN, gpioModePushPull, 1);
}
void init_USART0(void)
{
  USART_InitAsync_TypeDef init;
  init.enable = usartEnable;
  init.refFreq = 0;
  init.baudrate = BAUD_RATE;
  init.oversampling = usartOVS16;
  init.databits = usartDatabits8;
  init.parity = USART_FRAME_PARITY_NONE;
  init.stopbits = usartStopbits1;
  init.mvdis = false;
  init.prsRxEnable = false;
  init.prsRxCh = 0;
  init.autoCsEnable = false;
  init.csInv = false;
  init.autoCsHold = 0;
  init.autoCsSetup = 0;
  init.hwFlowControl = usartHwFlowControlNone;
  GPIO->USARTROUTE[0].TXROUTE = (BSP_TXPORT << _GPIO_USART_TXROUTE_PORT_SHIFT) | (BSP_TXPIN << _GPIO_USART_TXROUTE_PIN_SHIFT);
  GPIO->USARTROUTE[0].RXROUTE = (BSP_RXPORT << _GPIO_USART_RXROUTE_PORT_SHIFT) | (BSP_RXPIN << _GPIO_USART_RXROUTE_PIN_SHIFT);
  GPIO->USARTROUTE[0].ROUTEEN = GPIO_USART_ROUTEEN_RXPEN | GPIO_USART_ROUTEEN_TXPEN;
  USART_InitAsync(USART0, &init);
}

/*



void USART0_RX_IRQHandler(void)
{
    uint8_t received_char = USART0->RXDATA;
    USART_IntClear(USART0, USART_IF_RXDATAV); //  if (received_char == '\n')
    {
        flag = true;
        buffer[buffer_index] = '\0';
        buffer_index = 0;
    }
    else if (buffer_index < BUFLEN - 1)
    {
        buffer[buffer_index++] = received_char;
    }
    else
    {      memset(buffer, 0, BUFLEN);
        buffer_index = 0;
    }
}

void process_received_data(void)
{  char response[255];
    snprintf(response, sizeof(response), "data: %s\n", (char *)buffer);  send_usart_data(response);
    memset(buffer, 0, BUFLEN);
    buffer_index = 0;
}
static void usart_task(void *arg)
{
  (void)arg;
  init_USART_GPIO();
  init_USART0();
  send_usart_data("\nStarted\n");
  uint8_t command[1];
  while (1)
  {
     if (flag)
        {
            flag = false;  // Reset cờ
            process_received_data();  // Xử lý dữ liệu nhận được
        }
  }
}


*/
