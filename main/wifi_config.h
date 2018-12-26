#define WIFI_SSID      CONFIG_WIFI_SSID
#define WIFI_PASS      CONFIG_WIFI_PASS
#define MAXIMUM_RETRY  10

#include <string.h>
#include "esp_wifi.h"
#include "freertos/event_groups.h"
#include "esp_log.h"
#include "esp_event_loop.h"

static const char *WIFI_TAG = "PixLedModule_WiFi";
/* FreeRTOS event group to signal when we are connected*/
static EventGroupHandle_t s_wifi_event_group;

/* The event group allows multiple bits for each event, but we only care about one event
 * - are we connected to the AP with an IP? */
const int WIFI_CONNECTED_BIT = BIT0;

void wifi_init_sta();
