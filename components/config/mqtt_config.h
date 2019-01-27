#define MQTT_BROKER_URI CONFIG_MQTT_BROKER_URI

#include "mqtt_client.h"
#include "esp_log.h"
// #include "freertos/FreeRTOS.h"
// #include "freertos/task.h"

static const char *MQTT_TAG = "PixLedModule_MQTT";

static char light_id[5];
static char client_id[10];
static char color_topic[50];
static char switch_topic[50];
static char const *connection_topic = "/connected";
static char const *disconnection_topic = "/disconnected";
static char const *check_topic = "/check";


struct mqtt_context {
  // TaskHandle_t blink_led_task_handler;
  bool connected;
  bool subscribed_to_switch_topic;
  bool subscribed_to_color_topic;
};

void save_mqtt_uri_to_nvs(const char* uri);
void load_mqtt_uri_from_nvs(char** uri);
void mqtt_app_start();
