/*
 * dht11.h
 *
 *  Created on: Dec 23, 2024
 *      Author: Trung
 */

#ifndef DHT11_DHT11_FINAL_H_
#define DHT11_DHT11_FINAL_H_

#include "em_gpio.h"

enum dht11_final_status
{
    DHT11_CRC_ERROR = -2,
    DHT11_TIMEOUT_ERROR,
    DHT11_OK
};

struct dht11_reading
{
    int status;
    int temperature;
    int humidity;
};

void DHT11_init(GPIO_Port_TypeDef port,
                unsigned int pin);

struct dht11_reading DHT11_read();

#endif /* DHT11_DHT11_FINAL_H_ */
