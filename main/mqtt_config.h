#include "mqtt_client.h"
#include "esp_log.h"

static const char *MQTT_TAG = "PixLedModule_MQTT";

static char client_id[10];
static char color_topic[50];
static char switch_topic[50];
static char connection_topic[50];

void mqtt_app_start();
