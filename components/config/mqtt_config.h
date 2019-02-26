#define MQTT_BROKER_URI CONFIG_MQTT_BROKER_URI
#define MAIN_MQTT_EVENT_HANDLER main_mqtt_event_handler
#define TEST_MQTT_EVENT_HANDLER test_mqtt_event_handler

#include "mqtt_client.h"
#include "server_config.h"
#include "esp_log.h"
#include "esp_event_loop.h"

#define MQTT_TAG "MQTT"

static char device_id[5];
static char client_id[10];
static char color_topic[50];
static char switch_topic[50];
static char const *connection_topic = "/connected";
static char const *disconnection_topic = "/disconnected";
static char const *check_topic = "/check";


struct mqtt_context {
  // TaskHandle_t blink_led_task_handler;
  int connected;
  bool subscribed_to_switch_topic;
  bool subscribed_to_color_topic;
};

esp_err_t main_mqtt_event_handler(esp_mqtt_event_handle_t event);
esp_err_t test_mqtt_event_handler(esp_mqtt_event_handle_t event);

void save_mqtt_uri_to_nvs(const char* uri);
bool load_mqtt_uri_from_nvs(char** uri);
void mqtt_app_start(const char* uri, mqtt_event_callback_t mqtt_event_handler);
void clean_mqtt();
