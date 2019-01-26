#include "esp_err.h"
#include "main.h"

#include "mqtt_config.h"
#include "wifi_config.h"

extern "C" {
  void app_main();
}

static WS2812 strip = WS2812((gpio_num_t) LED_PIN, NUM_LED, RMT_CHANNEL_0);
static pixel_t last_color = { };
static bool on;

void blink_task(void *delay_ms)
{
  uint32_t delay = *((uint32_t*) delay_ms);
  // ESP_LOGI(MAIN_TAG, "Blink");
    gpio_pad_select_gpio((gpio_num_t) BLINK_GPIO);
    /* Set the GPIO as a push/pull output */
    gpio_set_direction((gpio_num_t) BLINK_GPIO, GPIO_MODE_OUTPUT);
    // ESP_LOGI(MAIN_TAG, "Blink : %i", delay);
    while(1) {
        /* Blink off (output low) */
        gpio_set_level((gpio_num_t) BLINK_GPIO, 0);
        vTaskDelay(delay / portTICK_PERIOD_MS);
        /* Blink on (output high) */
        gpio_set_level((gpio_num_t) BLINK_GPIO, 1);
        vTaskDelay(delay / portTICK_PERIOD_MS);
    }
}

void handle_color_changed(size_t payload_length, char* payload) {
    char* ptr;
    char color_str[payload_length + 1];
    for (int i = 0; i < payload_length + 1; i++) {
      color_str[i] = payload[i];
    }
    color_str[payload_length] = '\0';
    long color = strtol(color_str, &ptr, 10);

    uint32_t int_color = color  & 0xffffffff;
    last_color.red = (int_color >> 16) & 0xff;
    last_color.green = (int_color >> 8) & 0xff;
    last_color.blue = int_color & 0xff;
    ESP_LOGI(MAIN_TAG, "Set color : %i, %i, %i", last_color.red, last_color.green, last_color.blue);
    if (on) {
      for (int i = 0; i < NUM_LED; i++) {
        strip.setPixel(i, last_color);
      }
      strip.show();
    }
}

void handle_switch(size_t payload_length, char* payload) {
    char switch_str[payload_length + 1];
    for (int i = 0; i < payload_length + 1; i++) {
      switch_str[i] = payload[i];
    }
    switch_str[payload_length] = '\0';
    if (strcmp(switch_str, "ON") == 0) {
      ESP_LOGI(MAIN_TAG, "Switch On");
      on = true;
      for (int i = 0; i < NUM_LED; i++) {
        strip.setPixel(i, last_color);
      }
      strip.show();
    }
    else {
      ESP_LOGI(MAIN_TAG, "Switch Off");
      on = false;
      for (int i = 0; i < NUM_LED; i++) {
        strip.setPixel(i, 0, 0, 0);
      }
      strip.show();
    }
}

void app_main()
{
    ESP_LOGI(MAIN_TAG, "Color topic : %s", color_topic);
    ESP_LOGI(MAIN_TAG, "Init strip");
    last_color.red = 10;
    last_color.green = 10;
    last_color.blue = 10;
    for (int i = 0; i < NUM_LED; i++) {
      strip.setPixel(i, last_color.red, last_color.green, last_color.blue);
    }
    strip.show();

    ESP_LOGI(MAIN_TAG, "Init nvs");
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);
    nvs_handle nvs_config_handle;
    ESP_ERROR_CHECK(nvs_open("conf", NVS_READWRITE, &nvs_config_handle));
    ESP_ERROR_CHECK(nvs_erase_all(nvs_config_handle));

    ESP_ERROR_CHECK(nvs_set_str(nvs_config_handle, "wifi_ssid", WIFI_SSID));
    ESP_ERROR_CHECK(nvs_set_str(nvs_config_handle, "wifi_pw", WIFI_PASS));
    ESP_ERROR_CHECK(nvs_commit(nvs_config_handle));
    nvs_close(nvs_config_handle);
    ESP_LOGI(MAIN_TAG, "Wifi info written to nvs. ssid : %s , pw: %s", WIFI_SSID, WIFI_PASS);

    ESP_LOGI(MAIN_TAG, "Init WiFi");
    wifi_init_sta(WIFI_SSID, WIFI_PASS, MAIN_WIFI_EVENT_HANDLER);
}
