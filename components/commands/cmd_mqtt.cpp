#include "cmd_mqtt.h"
#include "esp_console.h"
#include "esp_err.h"
#include "argtable3/argtable3.h"
#include "esp_log.h"
#include "mqtt_config.h"

#define WIFI_CMD_TAG "PIXLED_MQTT"
/** Arguments used by 'wifi' function */
static struct {
    struct arg_lit *check;
    struct arg_str *adress;
    struct arg_lit *test;
    struct arg_end *end;
} mqtt_args;

static int handle_mqtt(int argc, char** argv) {
  int nerrors = arg_parse(argc, argv, (void**) &mqtt_args);
  if (nerrors != 0) {
      arg_print_errors(stderr, mqtt_args.end, argv[0]);
      return 1;
  }
  return 0;
}

void register_wifi()
{
    mqtt_args.adress = arg_str0("a", "adress", "<adress>", "set the IP or name of your PixLed MQTT broker");
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
