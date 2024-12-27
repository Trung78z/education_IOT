/*
 * lcd_app.h
 *
 *  Created on: Nov 4, 2024
 *      Author: Phat_Dang
 */

#ifndef APP_LCD_H_
#define APP_LCD_H_
#include <stdint.h>
void
memlcd_app_init (int hours, int minutes, int seconds);
void memlcd_dht_app_init(float temperature, float humidity);
void
display_init (void);

void
memlcd_app_process_action (void);
void memlcd_period_app_init(uint8_t periodDHT, uint8_t periodADV);


#endif /* APP_LCD_H_ */
