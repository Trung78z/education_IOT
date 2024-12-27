#include <__task/usart_task.h>
#include "___adv/custom_adv.h"
#include "___spi_lcd/app_lcd.h"
#include "__task/dht_task.h"
#include "__task/lcd_task.h"
#include "em_common.h"
#include "app_assert.h"
#include "sl_bluetooth.h"
#include "gatt_db.h"
#include "app.h"
#include "stdio.h"
#include "app_timer.h"
#include "em_cmu.h"
#include "em_timer.h"
#include "sl_sleeptimer.h"
#include "FreeRTOS.h"
#include "task.h"

volatile uint32_t data_buffer[100];
volatile uint32_t index = 0;

CustomAdv_t sData; // Our custom advertising data stored here

// static app_timer_t update_timer;

static uint8_t advertising_set_handle = 0xff;

uint32_t datapayload = 0x20200381;
extern char *name__Ble;

// extern uint8_t temperature, humidity;

/* *
 * Application Init.
 *****************************************************************************/

// static void
// update_timer_cb(app_timer_t *timer, void *data)
//{
//   (void)data;
//   (void)timer;
static void
update_timer_cb()
{
  while (1)
  {
    int temperature = data_temperature();
    int humidity = data_humidity();
    int periodDHT = data_periodDHT();
    int periodADV = data_periodADV();
    //  datapayload = 0x20200381;
    uint8_t temperature1 = temperature / 10;
    uint8_t temperature2 = temperature % 10;
    uint8_t humidity1 = humidity / 10;
    uint8_t humidity2 = humidity % 10;

    datapayload = ((periodDHT & 0xF) << 20) | ((periodADV & 0xF) << 16) | ((temperature1 & 0xF) << 12) | ((temperature2 & 0xF) << 8) | ((humidity1 & 0xF) << 4) | (humidity2 & 0xF);
    update_adv_data(&sData, advertising_set_handle, datapayload);
    vTaskDelay(pdMS_TO_TICKS(2 * 1000));
  }
}

void update_ble_task_init(void)
{
  TaskHandle_t xHandle = NULL;

  static StaticTask_t xTaskBuffer;
  static StackType_t xStack[2048];

  xHandle = xTaskCreateStatic(update_timer_cb, "update_timer_cb task",
                              2048,
                              (void *)NULL,
                              tskIDLE_PRIORITY + 5,
                              xStack, &xTaskBuffer);

  EFM_ASSERT(xHandle != NULL);
}

SL_WEAK void app_init(void)
{

  usart_init();

  vTaskDelay(pdMS_TO_TICKS(2 * 1000));
  display_init();
  lcd_task_init();
  dht_init();

  update_ble_task_init();

  //  sl_status_t sc;
  //  sc = app_timer_start(&update_timer, 2 * 1000, // ms
  //                       update_timer_cb,
  //                       NULL,
  //                       true);
  //
  //  app_assert_status(sc);
}

/**
 * Application Process Action.
 *****************************************************************************/
SL_WEAK void app_process_action(void)
{
}
void updateName(char *name)
{
  sl_status_t sc;

  sc = sl_bt_advertiser_stop(advertising_set_handle);
  app_assert_status(sc);

  fill_adv_packet(&sData, FLAG, COMPANY_ID, datapayload, name);
  start_adv(&sData, advertising_set_handle);

  char response[64];
  snprintf(response, sizeof(response), "BLE name updated to: %s\r\n", name__Ble);
  send_usart_data(response);
  sc = sl_bt_legacy_advertiser_start(
      advertising_set_handle, sl_bt_advertiser_connectable_scannable);
  app_assert_status(sc);
}

void sl_bl_timing_set_cb(uint16_t new_min_interval, uint16_t new_max_interval)
{

  sl_status_t sc;

  sc = sl_bt_advertiser_stop(advertising_set_handle);
  app_assert_status(sc);

  sc = sl_bt_advertiser_set_timing(advertising_set_handle,
                                   new_min_interval,
                                   new_max_interval,
                                   0,
                                   0);
  app_assert_status(sc);

  sc = sl_bt_legacy_advertiser_start(
      advertising_set_handle, sl_bt_advertiser_connectable_scannable);
  app_assert_status(sc);

  char response[64];
  snprintf(response, sizeof(response), "Advertising timing updated: min = %d, max = %d\n", new_min_interval, new_max_interval);
  send_usart_data(response);
}

void sl_bt_on_event(sl_bt_msg_t *evt)
{
  sl_status_t sc;
  bd_addr address;
  uint8_t address_type;
  uint8_t system_id[8];

  switch (SL_BT_MSG_ID(evt->header))
  {

  case sl_bt_evt_system_boot_id:

    // Extract unique ID from BT Address.
    sc = sl_bt_system_get_identity_address(&address, &address_type);
    app_assert_status(sc);

    // Pad and reverse unique ID to get System ID.
    system_id[0] = address.addr[5];
    system_id[1] = address.addr[4];
    system_id[2] = address.addr[3];
    system_id[3] = 0xFF;
    system_id[4] = 0xFE;
    system_id[5] = address.addr[2];
    system_id[6] = address.addr[1];
    system_id[7] = address.addr[0];

    sc = sl_bt_gatt_server_write_attribute_value(gattdb_system_id, 0,
                                                 sizeof(system_id),
                                                 system_id);
    app_assert_status(sc);

    // Create an advertising set.
    sc = sl_bt_advertiser_create_set(&advertising_set_handle);
    app_assert_status(sc);

    // Set advertising interval to 1s.
    sc = sl_bt_advertiser_set_timing(advertising_set_handle, 1600, // min. adv. interval (milliseconds * 1.6)
                                     1600,                         // max. adv. interval (milliseconds * 1.6)
                                     0,                            // adv. duration
                                     0);                           // max. num. adv. events
    app_assert_status(sc);

    sl_bt_advertiser_set_channel_map(advertising_set_handle, 7);
    app_assert_status(sc);

    fill_adv_packet(&sData, FLAG, COMPANY_ID, datapayload, name__Ble);
    send_usart_data("fill_adv_packet completed\r\n");

    start_adv(&sData, advertising_set_handle);
    send_usart_data("Started advertising\r\n");

    break;

  case sl_bt_evt_connection_opened_id:
    break;
  case sl_bt_evt_connection_closed_id:

    sc = sl_bt_legacy_advertiser_generate_data(
        advertising_set_handle, sl_bt_advertiser_general_discoverable);
    app_assert_status(sc);

    sc = sl_bt_legacy_advertiser_start(
        advertising_set_handle, sl_bt_advertiser_connectable_scannable);
    app_assert_status(sc);
    break;

  default:
    break;
  }
}
