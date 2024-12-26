/*
 * dht11.c
 *
 *  Created on: Dec 23, 2024
 *      Author: Trung
 *  Note:
 *  A suitable pull-up resistor should be connected to the selected GPIO line
 *
 *  __           ______          _______                              ___________________________
 *    \    A    /      \   C    /       \   DHT duration_data_low    /                           \
 *     \_______/   B    \______/    D    \__________________________/   DHT duration_data_high    \__
 *
 *
 *  Initializing communications with the DHT requires four 'phases' as follows:
 *
 *  Phase A - MCU pulls signal low for at least 18000 us
 *  Phase B - MCU allows signal to float back up and waits 20-40us for DHT to pull it low
 *  Phase C - DHT pulls signal low for ~80us
 *  Phase D - DHT lets signal float back up for ~80us
 *
 *  After this, the DHT transmits its first bit by holding the signal low for 50us
 *  and then letting it float back high for a period of time that depends on the data bit.
 *  duration_data_high is shorter than 50us for a logic '0' and longer than 50us for logic '1'.
 *
 *  There are a total of 40 data bits transmitted sequentially. These bits are read into a byte array
 *  of length 5.  The first and third bytes are humidity (%) and temperature (C), respectively.  Bytes 2 and 4
 *  are zero-filled and the fifth is a checksum such that:
 *
 *  byte_5 == (byte_1 + byte_2 + byte_3 + byte_4) & 0xFF
 *
 */

#include "dht11.h"
#include "sl_udelay.h"
#include "em_cmu.h"

static GPIO_Port_TypeDef dht_gpio_port;
static unsigned int dht_gpio_pin;
static struct dht11_reading last_read;

static void delay_us(uint32_t microseconds)
{
  sl_udelay_wait(microseconds);
}
static int _waitOrTimeout(uint16_t microSeconds, unsigned int level)
{
  int micros_ticks = 0;
  while (GPIO_PinInGet(dht_gpio_port, dht_gpio_pin) == level)
  {
    if (micros_ticks++ > microSeconds)
      return DHT11_TIMEOUT_ERROR;
    delay_us(1);
  }
  return micros_ticks;
}

void DHT11_init(GPIO_Port_TypeDef port, unsigned int pin)
{
  dht_gpio_port = port;
  dht_gpio_pin = pin;

  GPIO_PinModeSet(port, pin, gpioModePushPull, 1);
}

static void _sendStartSignal()
{
  GPIO_PinModeSet(dht_gpio_port, dht_gpio_pin, gpioModePushPull, 0); // Đưa chân DHT11 xuống mức thấp
  GPIO_PinOutClear(dht_gpio_port, dht_gpio_pin);
  delay_us(18000); // Delay 18ms (yêu cầu của DHT11)
  GPIO_PinOutSet(dht_gpio_port, dht_gpio_pin);
  delay_us(20);                                                       // Delay 20-40us
  GPIO_PinModeSet(dht_gpio_port, dht_gpio_pin, gpioModeInputPull, 1); // Chuyển sang chế độ input
}

static int _checkResponse()
{
  /* Wait for next step ~80us*/
  if (_waitOrTimeout(80, 0) == DHT11_TIMEOUT_ERROR)
    return DHT11_TIMEOUT_ERROR;

  /* Wait for next step ~80us*/
  if (_waitOrTimeout(80, 1) == DHT11_TIMEOUT_ERROR)
    return DHT11_TIMEOUT_ERROR;

  return DHT11_OK;
}

static int _checkCRC(uint8_t data[])
{
  if (data[4] == (data[0] + data[1] + data[2] + data[3]))
    return DHT11_OK;
  else
    return DHT11_CRC_ERROR;
}

static struct dht11_reading _timeoutError()
{
  struct dht11_reading timeoutError = {DHT11_TIMEOUT_ERROR, -1, -1};
  return timeoutError;
}

static struct dht11_reading _crcError()
{
  struct dht11_reading crcError = {DHT11_CRC_ERROR, -1, -1};
  return crcError;
}

struct dht11_reading DHT11_read()
{
  uint8_t data[5] =
      {0, 0, 0, 0, 0};
  uint32_t timeOut = 0;

  // 1. Gửi tín hiệu khởi động
  _sendStartSignal();
  // 2. Chờ phản hồi từ DHT11

  if (_checkResponse() == DHT11_TIMEOUT_ERROR)
    return last_read = _timeoutError();

  // 3. Đọc dữ liệu 40-bit
  for (int i = 0; i < 40; i++)
  {
    // Chờ đầu xung
    timeOut = 0;
    while (!GPIO_PinInGet(dht_gpio_port, dht_gpio_pin) && timeOut++ < 100)
      delay_us(1);
    if (timeOut >= 100)
      return last_read = _timeoutError();

    delay_us(28); // Delay để đọc bit

    if (GPIO_PinInGet(dht_gpio_port, dht_gpio_pin)) // Nếu chân ở mức cao
      data[i / 8] |= (1 << (7 - (i % 8)));


  }

  // 4. Kiểm tra checksum
  if (_checkCRC(data) != DHT11_CRC_ERROR)
  {
    last_read.status = DHT11_OK;
    last_read.temperature = data[2];
    last_read.humidity = data[0];
    return last_read;
  }
  else
  {
    return last_read = _crcError();
  }
}
