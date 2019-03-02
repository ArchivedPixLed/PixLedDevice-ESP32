#include "module_config.h"

uint16_t num_led;
WS2812* strip;

void init_strip() {
  if (!load_led_number_from_nvs(&num_led)) {
    // No led number has been specified yet
    num_led = 0;
  }
  ESP_LOGI(MODULE_TAG, "Led number : %i", num_led);
  static WS2812 _strip = WS2812((gpio_num_t) LED_PIN, num_led, RMT_CHANNEL_0);
  strip = &_strip;
}
void save_led_number_to_nvs(uint16_t led_number) {
  // Init NVS connection
  nvs_handle nvs_config_handle;
  ESP_ERROR_CHECK(nvs_open("conf", NVS_READWRITE, &nvs_config_handle));

  // Save id
  ESP_LOGI(MODULE_TAG, "Save led number to nvs : %i", led_number);
  ESP_ERROR_CHECK(nvs_set_u16(nvs_config_handle, "led_number", led_number));
  ESP_ERROR_CHECK(nvs_commit(nvs_config_handle));

  // Close NVS handler
  nvs_close(nvs_config_handle);
}

bool load_led_number_from_nvs(uint16_t* led_number) {
  // Init nvs connection
  nvs_handle nvs_config_handle;
  ESP_ERROR_CHECK(nvs_open("conf", NVS_READONLY, &nvs_config_handle));

  // Load id
  esp_err_t err = nvs_get_u16(nvs_config_handle, "led_number", led_number);
  bool found = true;
  if (err == ESP_ERR_NVS_NOT_FOUND) {
    found = false;
  }

  nvs_close(nvs_config_handle);
  return found;
}

/**
 * Called on color received. Convert the string payload into a 4 bytes long that
 * and send it to the strip.
 * @param[in] payload_length Length of the MQTT message payload
 * @param[in] payload MQTT message payload
 */
void handle_color_changed(long color) {
    uint32_t int_color = color  & 0xffffffff;
    last_color.red = (int_color >> 16) & 0xff;
    last_color.green = (int_color >> 8) & 0xff;
    last_color.blue = int_color & 0xff;
    ESP_LOGI(MODULE_TAG, "Set color : %i, %i, %i", last_color.red, last_color.green, last_color.blue);
    if (on) {
      for (int i = 0; i < num_led; i++) {
        strip->setPixel(i, last_color);
      }
      strip->show();
    }
}

void handle_switch(const char* switch_str) {
    if (strcmp(switch_str, "ON") == 0) {
      ESP_LOGI(MODULE_TAG, "Switch On");
      on = true;
      for (int i = 0; i < num_led; i++) {
        strip->setPixel(i, last_color);
      }
      strip->show();
    }
    else {
      ESP_LOGI(MODULE_TAG, "Switch Off");
      ESP_LOGI(MODULE_TAG, "Strip pointer : %p", strip);
      ESP_LOGI(MODULE_TAG, "Pixel count : %i", num_led);
      on = false;
      for (int i = 0; i < num_led; i++) {
        strip->setPixel(i, 0, 0, 0);
      }
      ESP_LOGI(MODULE_TAG, "Show");
      strip->show();
    }
}
