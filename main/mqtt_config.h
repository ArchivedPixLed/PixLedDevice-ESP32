#include "mqtt_client.h"
#include "esp_log.h"
// #include "freertos/FreeRTOS.h"
// #include "freertos/task.h"

static const char *MQTT_TAG = "PixLedModule_MQTT";

static char client_id[10];
static char color_topic[50];
static char switch_topic[50];
static char connection_topic[50];

struct mqtt_context {
  // TaskHandle_t blink_led_task_handler;
  bool connected;
  bool subscribed_to_switch_topic;
  bool subscribed_to_color_topic;
};

void mqtt_app_start();
