#include "cmd_server.h"

#include "esp_console.h"
#include "esp_err.h"
#include "argtable3/argtable3.h"
#include "esp_log.h"
#include "wifi_config.h"
#include "mdns_config.h"
#include "server_config.h"

#define SERVER_CMD_TAG "SERVER"

static struct {
    struct arg_lit *check;
    struct arg_lit *info;
    struct arg_str *url;
    struct arg_lit *mdns;
    struct arg_lit *reset;
    struct arg_end *end;
} server_args;

static int handle_server(int argc, char** argv) {
  int nerrors = arg_parse(argc, argv, (void**) &server_args);
  if (nerrors != 0) {
      arg_print_errors(stderr, server_args.end, argv[0]);
      return 1;
  }
  if (server_args.reset->count > 0){
    delete_id_from_nvs();
  }
  if (server_args.check->count > 0){
    int8_t device_id;
    if (load_id_from_nvs(&device_id)) {
      ESP_LOGI(SERVER_CMD_TAG, "Currently stored device id : %i", device_id);
    }
    else {
      ESP_LOGI(SERVER_CMD_TAG, "No device id stored yet.");
    }

    char* server_url;
    if (load_server_url_from_nvs(&server_url)) {
      ESP_LOGI(SERVER_CMD_TAG, "Currently stored server_url : %s", server_url);
      free(server_url);
    }
    else {
      ESP_LOGI(SERVER_CMD_TAG, "No server url stored yet.");
    }
  }
  if (server_args.info->count > 0) {
    ESP_LOGI(SERVER_CMD_TAG, "Connecting to test WiFi.");
    clean_wifi();
    char* ssid;
    char* password;
    if (load_wifi_config_from_nvs(&ssid, &password)) {
      WifiContext* connection_status = wifi_init_sta(ssid, password, TEST_WIFI_EVENT_HANDLER);
      while(connection_status->connected == WIFI_STATUS_WAITING) {
      }
      if (connection_status->connected == WIFI_STATUS_CONNECTED) {
        perform_device_request();
      }
      else {
        ESP_LOGI(SERVER_CMD_TAG, "WiFi connection failed.");
      }
      clean_wifi();
    } else {
        ESP_LOGI(SERVER_CMD_TAG, "Missing wifi parameters.");
    }
  }
  if(server_args.url->count > 0) {
    ESP_LOGI(SERVER_CMD_TAG, "Saving server url to nvs : %s", server_args.url->sval[0]);
    save_server_url_to_nvs(server_args.url->sval[0]);
  }
  else {
    if (server_args.mdns->count > 0) {
      ESP_LOGI(SERVER_CMD_TAG, "Connecting to test WiFi.");
      clean_wifi();
      char* ssid;
      char* password;
      if (load_wifi_config_from_nvs(&ssid, &password)) {
        WifiContext* connection_status = wifi_init_sta(ssid, password, TEST_WIFI_EVENT_HANDLER);
        while(connection_status->connected == WIFI_STATUS_WAITING) {
        }
        if (connection_status->connected == WIFI_STATUS_CONNECTED) {
          clean_mdns();
          init_mdns();
          char ip[16];
          char port[5];
          bool found = look_for_server(ip, port);
          if (found) {
            char url[50];
            sprintf(url, "http://%s:%s", ip, port);
            ESP_LOGI(SERVER_CMD_TAG, "Saving server url %s", url);
            save_server_url_to_nvs(url);
          }
          clean_mdns();
        }
        else {
          ESP_LOGI(SERVER_CMD_TAG, "WiFi connection failed.");
        }
        clean_wifi();
      }
      else {
        ESP_LOGI(SERVER_CMD_TAG, "Missing wifi parameters.");
      }
    }
  }

  return 0;
}

void register_server()
{
    server_args.check = arg_lit0("c", "check", "check current stored configuration");
    server_args.info = arg_lit0("i", "info", "fetch device info from server");
    server_args.url = arg_str0("u", "url", "<url>", "set the URL (using IP or name) of your PixLed MQTT server");
    server_args.mdns = arg_lit0("m", "mdns", "Look for a PixLed server using mDNS (ignored if -u is specified)");
    server_args.reset = arg_lit0("r", "reset", "Delete currently stored id. A create device request will be performed on next request. Does not delete device from database.");
    server_args.end = arg_end(3);

    esp_console_cmd_t server_cmd = { };
    server_cmd.command = "server";
    server_cmd.help = "Set up server info.";
    server_cmd.hint = NULL;
    server_cmd.func = &handle_server;
    server_cmd.argtable = &server_args;

    ESP_ERROR_CHECK( esp_console_cmd_register(&server_cmd) );
}
