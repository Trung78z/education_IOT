/***************************************************************************/
/**
 * @file
 * @brief Blink examples functions
 *******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/

#include "FreeRTOS.h"
#include "task.h"
#include "app_log.h"
#include "__dht_lib/dht11.h"
#include "em_cmu.h"
#include "dht_task.h"
#include "uart_task.h"

#define DHT_TASK_STACK_SIZE 4096

#define BSP_GPIO_DHT_PORT gpioPortD
#define BSP_GPIO_DHT_PIN 3

static int temperature;
static int humidity;


 static void gpio_init()
{
  CMU_ClockEnable(cmuClock_GPIO, true);
}
static void
dht_task(void *arg);

void dht_init(void)
{
  TaskHandle_t xHandle = NULL;

  static StaticTask_t xTaskBuffer;
  static StackType_t xStack[DHT_TASK_STACK_SIZE];

  xHandle = xTaskCreateStatic(dht_task, "dht task",
                              DHT_TASK_STACK_SIZE,
                              (void *)NULL,
                              tskIDLE_PRIORITY + 6,
                              xStack, &xTaskBuffer);

  EFM_ASSERT(xHandle != NULL);
}

static void
dht_task(void *arg)
{
  (void)arg;

  gpio_init();

  DHT11_init(BSP_GPIO_DHT_PORT, BSP_GPIO_DHT_PIN);
  struct dht11_reading reading;
  while (1)
  {
    // Wait for specified delay
      int periodDHT= data_periodDHT();
    reading = DHT11_read();
    if (reading.status == DHT11_OK)
    {
      temperature = reading.temperature;
      humidity = reading.humidity;
    }
    else if (reading.status == DHT11_CRC_ERROR)
    {
      app_log("\nCheck sum error");
    }
    else
    {
      app_log("\nTIMEROUT");
    }
    vTaskDelay(pdMS_TO_TICKS(periodDHT*1000));
  }
}
int data_temperature()
{
  return temperature;
}
int data_humidity()
{
  return humidity;
}
