#define BLINK_GPIO 2
#define LED_PIN 12
#define NUM_LED CONFIG_NUM_LED
#define LIGHT_ID CONFIG_LIGHT_ID

#define MAIN_TAG "PixLedModule_Main"

#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sys.h"
#include "WS2812.h"


void blink_task(void *delay_ms);
void handle_color_changed(size_t payload_length, char* payload);
void handle_switch(size_t payload_length, char* payload);

void launch_default_mode();
void quit_default_mode();
