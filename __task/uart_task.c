/*
 * uart_task.c
 *
 *  Created on: Dec 23, 2024
 *      Author: Trung
 */

#include "uart_task.h"

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
#define BLINK_TASK_STACK_SIZE 2048

#define BSP_GPIO_LED1_PORT gpioPortD
#define BSP_GPIO_LED1_PIN 3

#define BSP_TXPORT gpioPortA
#define BSP_RXPORT gpioPortA
#define BSP_TXPIN 5
#define BSP_RXPIN 6
#define BSP_ENABLE_PORT gpioPortD
#define BSP_ENABLE_PIN 4
#define BAUD_RATE 115200

uint8_t command[1];

static void gpio_init()
{
  CMU_ClockEnable(cmuClock_GPIO, true);
  CMU_ClockEnable(cmuClock_USART0, true);
  GPIO_PinModeSet(BSP_TXPORT, BSP_TXPIN, gpioModePushPull, 1);
  GPIO_PinModeSet(BSP_RXPORT, BSP_RXPIN, gpioModeInput, 0);
  GPIO_PinModeSet(BSP_ENABLE_PORT, BSP_ENABLE_PIN, gpioModePushPull, 1);
}

void initUSART0(void)
{
  USART_InitAsync_TypeDef init;
  init.enable = usartEnable;
  init.refFreq = 0;
  init.baudrate = 115200;
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

static void uart_task(void *arg);
void uart_init(void)
{
  TaskHandle_t xHandle = NULL;

  static StaticTask_t xTaskBuffer;
  static StackType_t xStack[BLINK_TASK_STACK_SIZE];

  xHandle = xTaskCreateStatic(uart_task, "uart task",
                              BLINK_TASK_STACK_SIZE,
                              (void *)NULL,
                              tskIDLE_PRIORITY + 5,
                              xStack, &xTaskBuffer);

  EFM_ASSERT(xHandle != NULL);
}

/*******************************************************************************
 * Blink task.
 ******************************************************************************/
static uint8_t periodDHT = 1;
static uint16_t periodADV = 1;

// static const char *message = "Hello, UART!\r\n";
// static const char *message_ok = "OK\r\n";
void handle_command(uint8_t *command)
{

  switch (command[0])
  {
  case '0':
    periodDHT = 1;
    char messagecase0[100];
    sprintf(messagecase0, "OK, periodDHT: %d\r\n", periodDHT);
    for (size_t i = 0; i < strlen(messagecase0); i++)
    {
      while (!(USART0->STATUS & USART_STATUS_TXBL))
      {
        // Chờ đến khi TX Buffer trống
      }

      USART_Tx(USART0, messagecase0[i]); // Gửi ký tự qua UART
      vTaskDelay(pdMS_TO_TICKS(10));
    }
    break;
  case '1':

    if (periodDHT >= 15)
    {
      periodDHT = 1;
    }
    else
    {
      periodDHT++;
    }

    char messagecase1[100];
    sprintf(messagecase1, "OK, periodDHT: %d\r\n", periodDHT);
    for (size_t i = 0; i < strlen(messagecase1); i++)
    {
      while (!(USART0->STATUS & USART_STATUS_TXBL))
      {
        // Chờ đến khi TX Buffer trống
      }

      USART_Tx(USART0, messagecase1[i]); // Gửi ký tự qua UART
      vTaskDelay(pdMS_TO_TICKS(10));
    }
    break;

  case '2':
    periodADV = 1;
    char messagecase2[100];
    sprintf(messagecase2, "OK, periodADV: %d\r\n", periodADV);
    sl_bl_timing_set_cb(periodADV * 800, periodADV * 800);
    for (size_t i = 0; i < strlen(messagecase2); i++)
    {
      while (!(USART0->STATUS & USART_STATUS_TXBL))
      {
        // Chờ đến khi TX Buffer trống
      }

      USART_Tx(USART0, messagecase2[i]); // Gửi ký tự qua UART
      vTaskDelay(pdMS_TO_TICKS(10));
    }
    break;

  case '3':
    if (periodADV >= 12)
    {
      periodADV = 12;
      sl_bl_timing_set_cb(periodADV * 800, periodADV * 800);
    }
    else
    {
      periodADV++;
      sl_bl_timing_set_cb(periodADV * 800, periodADV * 800);
    }
    char messagecase3[100];
    sprintf(messagecase3, "OK, periodADV: %d\r\n", periodADV);

    for (size_t i = 0; i < strlen(messagecase3); i++)
    {
      while (!(USART0->STATUS & USART_STATUS_TXBL))
      {
        // Chờ đến khi TX Buffer trống
      }

      USART_Tx(USART0, messagecase3[i]); // Gửi ký tự qua UART
      vTaskDelay(pdMS_TO_TICKS(10));
    }
    break;
  case '9':
  {
    int temperature = data_temperature();
    int humidity = data_humidity();
    char message2[100];
    size_t len = sprintf(message2, "Temperature: %dC, Humidity: %d%%\r\n", temperature, humidity);
    for (size_t i = 0; i < len; i++)
    {
      while (!(USART0->STATUS & USART_STATUS_TXBL))
      {
      }
      USART_Tx(USART0, message2[i]); // Gửi ký tự qua UART
      vTaskDelay(pdMS_TO_TICKS(10)); // Thêm thời gian trễ 10ms
    }
  }
  break;

  default:

    return;
  }
}

static void uart_task(void *arg)
{
  (void)arg;

  gpio_init();
  initUSART0();

  while (1)
  {

    while (!(USART0->STATUS & USART_STATUS_RXDATAV))
    {
    }
    *command = USART_Rx(USART0);

    handle_command(command);
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