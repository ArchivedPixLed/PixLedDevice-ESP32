#include "cmd_wifi.h"
#include "esp_console.h"
#include "esp_err.h"
#include "argtable3/argtable3.h"
#include "esp_log.h"
#include "wifi_config.h"

#define WIFI_CMD_TAG "PIXLED_WIFI"
/** Arguments used by 'wifi' function */
static struct {
    struct arg_lit *check;
    struct arg_str *ssid;
    struct arg_str *password;
    struct arg_lit *test;
    struct arg_end *end;
} wifi_args;

static int handle_wifi(int argc, char** argv) {
  int nerrors = arg_parse(argc, argv, (void**) &wifi_args);
  if (nerrors != 0) {
      arg_print_errors(stderr, wifi_args.end, argv[0]);
      return 1;
  }
  if (wifi_args.ssid->count > 0){
    ESP_LOGI(WIFI_CMD_TAG, "Saving wifi config.");
    printf("ssid : %s\n", wifi_args.ssid->sval[0]);
    printf("password : %s\n", wifi_args.password->sval[0]);
    save_wifi_info_to_nvs(wifi_args.ssid->sval[0], wifi_args.password->sval[0]);
  }
  if (wifi_args.check->count > 0){
    char* ssid;
    char* password;
    load_wifi_config_from_nvs(&ssid, &password);
    ESP_LOGI(WIFI_CMD_TAG, "Currently stored wifi configuration :");
    printf("ssid : %s\n", ssid);
    printf("password : %s\n", password);
    free(ssid);
    free(password);
  }
  if (wifi_args.test->count > 0){
    ESP_LOGI(WIFI_CMD_TAG, "Testing connection.");
    clean_wifi();
    char* ssid;
    char* password;
    load_wifi_config_from_nvs(&ssid, &password);
    wifi_init_sta(ssid, password, TEST_WIFI_EVENT_HANDLER);
    clean_wifi();
  }
  return 0;
}

void register_mqtt()
{
    wifi_args.ssid = arg_str0("s", "ssid", "<ssid>", "SSID of your access point");
    wifi_args.password = arg_str0("p", "password", "<password>", "WiFi password");
    wifi_args.check = arg_lit0("c", "check", "check current stored configuration");
    wifi_args.test = arg_lit0("t", "test", "performs a WiFi connection test from the stored configuration");
    wifi_args.end = arg_end(3);

    esp_console_cmd_t wifi_cmd = { };
    wifi_cmd.command = "wifi";
    wifi_cmd.help = "Set up connection info.";
    wifi_cmd.hint = NULL;
    wifi_cmd.func = &handle_wifi;
    wifi_cmd.argtable = &wifi_args;

    ESP_ERROR_CHECK( esp_console_cmd_register(&wifi_cmd) );
}
