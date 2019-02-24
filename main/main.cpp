#include "esp_err.h"
#include "main.h"

#include "mqtt_config.h"
#include "wifi_config.h"
#include "console_config.h"
#include "mode_handler.h"

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

/**
 * Called on color received. Convert the string payload into a 4 bytes long that
 * and send it to the strip.
 * @param[in] payload_length Length of the MQTT message payload
 * @param[in] payload MQTT message payload
 */
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
  ESP_LOGI(MAIN_TAG, "Init nvs");
  esp_err_t err = nvs_flash_init();
  if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    ESP_ERROR_CHECK(nvs_flash_erase());
    err = nvs_flash_init();
  }
  ESP_ERROR_CHECK(err);

  if(!(strcmp(WIFI_SSID, "") == 0)) {
    /* Save and check WiFi info */
    save_wifi_info_to_nvs(WIFI_SSID, WIFI_PASS);

    char* ssid;
    char* password;
    load_wifi_config_from_nvs(&ssid, &password);
    ESP_LOGI(MAIN_TAG, "Wifi info written to nvs. ssid : %s , pw: %s", ssid, password);
    free(ssid);
    free(password);
  }

  if(!(strcmp(MQTT_BROKER_URI, "") == 0)) {
    /* Save and check MQTT info */
    save_mqtt_uri_to_nvs(MQTT_BROKER_URI);
    char* mqtt_uri;
    load_mqtt_uri_from_nvs(&mqtt_uri);
    ESP_LOGI(MAIN_TAG, "MQTT URI written to nvs : %s", mqtt_uri);
    free(mqtt_uri);
  }

  initialize_mode_handler();
  switchMode();
}

void launch_default_mode() {

  char* ssid;
  char* password;
  load_wifi_config_from_nvs(&ssid, &password);
  WifiContext* wifiContext = wifi_init_sta(ssid, password, MAIN_WIFI_EVENT_HANDLER);

  // Wait for connection result
  while(wifiContext->connected == WIFI_STATUS_WAITING){
     printf(".");
     vTaskDelay(500 / portTICK_PERIOD_MS);
  }

  if(wifiContext->connected == WIFI_STATUS_CONNECTED) {
    char* mqtt_uri;
    load_mqtt_uri_from_nvs(&mqtt_uri);
    mqtt_app_start(mqtt_uri, MAIN_MQTT_EVENT_HANDLER);
    free(mqtt_uri);
  }

  // init_mdns();
  // char ip[16];
  // char port[5];
  // bool found = look_for_mqtt_broker(ip, port);
  // if (found) {
  //   char uri[30];
  //   sprintf(uri, "mqtt://%s:%s/", ip, port);
  //   ESP_LOGI(MQTT_CMD_TAG, "Saving broker uri %s", uri);
  //   save_mqtt_uri_to_nvs(uri);
  // }

  // char* mqtt_uri;
  // load_mqtt_uri_from_nvs(&mqtt_uri);
  // mqtt_app_start(mqtt_uri, MAIN_MQTT_EVENT_HANDLER);
  //
  //
  // ESP_LOGI(MAIN_TAG, "Color topic : %s", color_topic);
  // ESP_LOGI(MAIN_TAG, "Init strip");
  // last_color.red = 10;
  // last_color.green = 10;
  // last_color.blue = 10;
  // for (int i = 0; i < NUM_LED; i++) {
  //   strip.setPixel(i, last_color.red, last_color.green, last_color.blue);
  // }
  // strip.show();
}

void quit_default_mode() {
  clean_mqtt();
  clean_wifi();
}
