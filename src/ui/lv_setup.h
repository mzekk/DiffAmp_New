#pragma once
#include "esp_lcd_panel_interface.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_io.h"

extern esp_lcd_panel_handle_t panel_handle_copy;
extern esp_lcd_panel_io_handle_t io_handle;


void lv_begin();
void lv_handler();