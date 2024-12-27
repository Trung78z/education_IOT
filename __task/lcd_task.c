/*
 * lcd_task.c
 *
 *  Created on: Dec 23, 2024
 *      Author: Trung
 */

#include <__task/usart_task.h>
#include "lcd_task.h"
#include "FreeRTOS.h"
#include "task.h"
#include "___spi_lcd/app_lcd.h"
#include "dht_task.h"
#define LCD_STACK_SIZE 2048

static int hours = 23;
static int minutes = 59;
static int seconds = 30;
static void updateClock(void)
{

  seconds++;
  if (seconds >= 60)
  {
    seconds = 0;
    minutes++;
    if (minutes >= 60)
    {
      minutes = 0;
      hours++;
      if (hours >= 24)
      {
        hours = 0;
      }
    }
  }
}

static void lcd_task(void *arg);
void lcd_task_init(void)
{
  TaskHandle_t xHandle = NULL;

  static StaticTask_t xTaskBuffer;
  static StackType_t xStack[LCD_STACK_SIZE];

  xHandle = xTaskCreateStatic(lcd_task, "lcd task",
                              LCD_STACK_SIZE,
                              (void *)NULL,
                              5,
                              xStack, &xTaskBuffer);

  EFM_ASSERT(xHandle != NULL);
}

static void lcd_task(void *arg)
{
  (void)arg;

  while (1)
  {
    int periodDHT = data_periodDHT();
    int periodADV = data_periodADV();
    int temperature = data_temperature();
    int humidity = data_humidity();
    memlcd_dht_app_init(temperature, humidity);

    updateClock();
    memlcd_app_init(hours, minutes, seconds);
    memlcd_period_app_init(periodDHT, periodADV);
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}
