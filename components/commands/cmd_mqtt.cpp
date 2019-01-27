#include "cmd_mqtt.h"
#include "esp_console.h"
#include "esp_err.h"
#include "argtable3/argtable3.h"
#include "esp_log.h"
#include "mqtt_config.h"
#include "wifi_config.h"

#define MQTT_CMD_TAG "PIXLED_MQTT"
/** Arguments used by 'wifi' function */
static struct {
    struct arg_lit *check;
    struct arg_str *uri;
    struct arg_lit *test;
    struct arg_end *end;
} mqtt_args;

static int handle_mqtt(int argc, char** argv) {
  int nerrors = arg_parse(argc, argv, (void**) &mqtt_args);
  if (nerrors != 0) {
      arg_print_errors(stderr, mqtt_args.end, argv[0]);
      return 1;
  }

  if(mqtt_args.uri->count > 0) {
    ESP_LOGI(MQTT_CMD_TAG, "Saving MQTT URI to nvs : %s", mqtt_args.uri->sval[0]);
    save_mqtt_uri_to_nvs(mqtt_args.uri->sval[0]);
  }

  if(mqtt_args.check->count > 0) {
    char* mqtt_uri;
    load_mqtt_uri_from_nvs(&mqtt_uri);
    ESP_LOGI(MQTT_CMD_TAG, "Currently stored mqtt uri : %s", mqtt_uri);
    free(mqtt_uri);
  }

  if(mqtt_args.test->count > 0) {
    ESP_LOGI(MQTT_CMD_TAG, "Connecting to test WiFi.");
    clean_wifi();
    char* ssid;
    char* password;
    load_wifi_config_from_nvs(&ssid, &password);
    WifiContext connection_result = wifi_init_sta(ssid, password, TEST_WIFI_EVENT_HANDLER);

    if (connection_result.connected == 1) {
      clean_mqtt();
      char* uri;
      load_mqtt_uri_from_nvs(&uri);
      mqtt_app_start(uri, TEST_MQTT_EVENT_HANDLER);

      clean_mqtt();
    }
    else {
      ESP_LOGI(MQTT_CMD_TAG, "WiFi connection failed.");
    }
    clean_wifi();
  }
  return 0;
}

void register_wifi()
{
    mqtt_args.uri = arg_str0("u", "uri", "<uri>", "set the URI (IP or name) of your PixLed MQTT broker");
    mqtt_args.check = arg_lit0("c", "check", "check current stored configuration");
    mqtt_args.test = arg_lit0("t", "test", "performs an MQTT connection test from the stored configuration");
    mqtt_args.end = arg_end(3);

    esp_console_cmd_t mqtt_cmd = { };
    mqtt_cmd.command = "mqtt";
    mqtt_cmd.help = "Set up MQTT connection info.";
    mqtt_cmd.hint = NULL;
    mqtt_cmd.func = &handle_mqtt;
    mqtt_cmd.argtable = &mqtt_args;

    ESP_ERROR_CHECK( esp_console_cmd_register(&mqtt_cmd) );
}
