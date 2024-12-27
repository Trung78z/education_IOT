/*
 * lcd.c
 *
 *  Created on: Nov 4, 2024
 *      Author: Phat_Dang
 */
#include <___spi_lcd/app_lcd.h>
#include <___spi_lcd/dmd.h>
#include <___spi_lcd/glib.h>
#include <stdio.h>

#include "sl_board_control.h"
#include "em_assert.h"
#include "app_lcd.h"
#include "em_timer.h"
#include "em_cmu.h"
#include "em_core.h"
#include "sli_sleeptimer.h"
#ifndef LCD_MAX_LINES
#define LCD_MAX_LINES 11
#endif

/*******************************************************************************
 ***************************  LOCAL VARIABLES   ********************************
 ******************************************************************************/
static GLIB_Context_t glibContext;
static int currentLine = 0;

/*******************************************************************************
 **************************   GLOBAL FUNCTIONS   *******************************
 ******************************************************************************/

/***************************************************************************/
/**
 * Initialize example.
 ******************************************************************************/

void display_init(void)
{
  uint32_t status;

  /* Enable the memory lcd */
  status = sl_board_enable_display();
  EFM_ASSERT(status == SL_STATUS_OK);

  /* Initialize the DMD support for memory lcd display */
  status = DMD_init(0);
  EFM_ASSERT(status == DMD_OK);

  /* Initialize the glib context */
  status = GLIB_contextInit(&glibContext);
  EFM_ASSERT(status == GLIB_OK);

  glibContext.backgroundColor = White;
  glibContext.foregroundColor = Black;

  GLIB_clear(&glibContext);

  GLIB_setFont(&glibContext, (GLIB_Font_t *)&GLIB_FontNarrow6x8);

  GLIB_drawStringOnLine(&glibContext, "PROJECT FINAL IOT", currentLine++,
                        GLIB_ALIGN_CENTER, 5, 5,
                        true);
  GLIB_drawStringOnLine(&glibContext, "20200381 - TRUNG", currentLine++,
                        GLIB_ALIGN_LEFT, 5, 5,
                        true);
  GLIB_drawStringOnLine(&glibContext, "21200374 - LY", currentLine++,
                        GLIB_ALIGN_LEFT, 5, 5,
                        true);
  GLIB_drawStringOnLine(&glibContext, "21200337 - QUAN", currentLine++,
                        GLIB_ALIGN_LEFT, 5, 5,
                        true);
  GLIB_drawStringOnLine(&glibContext, "21200342 - QUIN", currentLine++,
                        GLIB_ALIGN_LEFT, 5, 5,
                        true);
  GLIB_drawStringOnLine(&glibContext, "19200479 - THANG", currentLine++,
                        GLIB_ALIGN_LEFT, 5, 5,
                        true);
  GLIB_drawStringOnLine(&glibContext, "DHT11_TASK", 7,
                        GLIB_ALIGN_CENTER, 5, 5,
                        true);
  GLIB_drawStringOnLine(&glibContext, "Period", 10,
                        GLIB_ALIGN_CENTER, 5, 5,
                        true);
}
void memlcd_app_init(int hours, int minutes, int seconds)
{

  char timeString[9];
  sprintf(timeString, "%02d:%02d:%02d", hours, minutes, seconds);

  GLIB_drawStringOnLine(&glibContext, timeString, 8, GLIB_ALIGN_LEFT,
                        5, 5,
                        true);
  DMD_updateDisplay();
}

void memlcd_dht_app_init(uint8_t temperature, uint8_t humidity)
{

  char timeString[24];
  sprintf(timeString, "Temp: %d - Humid: %d", temperature, humidity);

  GLIB_drawStringOnLine(&glibContext, timeString, 9, GLIB_ALIGN_LEFT,
                        5, 5,
                        true);
  DMD_updateDisplay();
}

void memlcd_period_app_init(uint8_t periodDHT, uint8_t periodADV)
{

  char timeString[24];
  sprintf(timeString, "DHT: %ds - ADV: %ds", periodDHT, periodADV);

  GLIB_drawStringOnLine(&glibContext, timeString, 11, GLIB_ALIGN_LEFT,
                        5, 5,
                        true);
  DMD_updateDisplay();
}

/**
 * Ticking function.
 ******************************************************************************/
void memlcd_app_process_action(void)
{
  return;
}
