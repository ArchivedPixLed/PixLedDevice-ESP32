#define BLINK_GPIO CONFIG_BLINK_GPIO
#define LED_PIN CONFIG_LED_PIN
#define NUM_LED CONFIG_NUM_LED

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

void blink_task(void *delay_ms);

void launch_default_mode();
void quit_default_mode();
