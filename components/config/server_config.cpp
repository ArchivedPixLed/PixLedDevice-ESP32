#include "server_config.h"
#include "esp_log.h"
#include "cJSON.h"
#include "module_config.h"

#define SERVER_TAG "SERVER"

static bool device_registered = false;

void save_id_to_nvs(int8_t device_id) {
  // Init NVS connection
  nvs_handle nvs_config_handle;
  ESP_ERROR_CHECK(nvs_open("conf", NVS_READWRITE, &nvs_config_handle));

  // Save id
  ESP_LOGI(SERVER_TAG, "Save id to nvs : %i", device_id);
  ESP_ERROR_CHECK(nvs_set_i8(nvs_config_handle, "device_id", device_id));
  ESP_ERROR_CHECK(nvs_commit(nvs_config_handle));

  // Close NVS handler
  nvs_close(nvs_config_handle);
}

bool load_id_from_nvs(int8_t* device_id) {
  // Init nvs connection
  nvs_handle nvs_config_handle;
  ESP_ERROR_CHECK(nvs_open("conf", NVS_READONLY, &nvs_config_handle));

  // Load id
  esp_err_t err = nvs_get_i8(nvs_config_handle, "device_id", device_id);
  bool found = true;
  if (err == ESP_ERR_NVS_NOT_FOUND) {
    found = false;
  }

  nvs_close(nvs_config_handle);
  return found;
}

void delete_id_from_nvs() {
  // Init NVS connection
  nvs_handle nvs_config_handle;
  ESP_ERROR_CHECK(nvs_open("conf", NVS_READWRITE, &nvs_config_handle));

  ESP_ERROR_CHECK(nvs_erase_key(nvs_config_handle, "device_id"));
  ESP_ERROR_CHECK(nvs_commit(nvs_config_handle));

  nvs_close(nvs_config_handle);
}

void save_server_url_to_nvs(const char* url) {
  // Init NVS connection
  nvs_handle nvs_config_handle;
  ESP_ERROR_CHECK(nvs_open("conf", NVS_READWRITE, &nvs_config_handle));

  // Save url
  ESP_ERROR_CHECK(nvs_set_str(nvs_config_handle, "server_url", url));
  ESP_ERROR_CHECK(nvs_commit(nvs_config_handle));

  // Close NVS handler
  nvs_close(nvs_config_handle);
}

bool load_server_url_from_nvs(char** url) {
  // Init nvs connection
  nvs_handle nvs_config_handle;
  ESP_ERROR_CHECK(nvs_open("conf", NVS_READONLY, &nvs_config_handle));

  // Load url
  size_t url_length;
  esp_err_t err = nvs_get_str(nvs_config_handle, "server_url", NULL, &url_length);
  bool found = true;
  if (err == ESP_ERR_NVS_NOT_FOUND) {
    found = false;
  }
  else {
    *url = (char*) malloc(url_length);
    ESP_ERROR_CHECK(nvs_get_str(nvs_config_handle, "server_url", *url, &url_length));
  }

  // Close NVS handler
  nvs_close(nvs_config_handle);
  return found;
}

esp_err_t fetch_device_handler(esp_http_client_event_t *evt)
{
  cJSON *device;
  cJSON *state;
  cJSON *color_object;
  char* status;
  long color;
  switch(evt->event_id) {
      case HTTP_EVENT_ERROR:
          ESP_LOGI(SERVER_TAG, "HTTP ERROR");
          break;
      case HTTP_EVENT_ON_CONNECTED:
          ESP_LOGI(SERVER_TAG, "HTTP CONNECTED");
          break;
      case HTTP_EVENT_HEADER_SENT:
          break;
      case HTTP_EVENT_ON_HEADER:
          break;
      case HTTP_EVENT_ON_DATA:
          ESP_LOGI(SERVER_TAG, "Device info received : %.*s", evt->data_len, (char*)evt->data);
          device_registered = true;
          device = cJSON_Parse((char*)evt->data);
          state = cJSON_GetObjectItem(device, "state");
          status = cJSON_GetObjectItem(state, "toggle")->valuestring;
          color_object = cJSON_GetObjectItem(state, "color");
          color = cJSON_GetObjectItem(color_object, "argb")->valueint;

          handle_switch(status);
          handle_color_changed(color);
          cJSON_Delete(device);
          break;
      case HTTP_EVENT_ON_FINISH:
          ESP_LOGI(SERVER_TAG, "HTTP FINISH");
          break;
      case HTTP_EVENT_DISCONNECTED:
          ESP_LOGI(SERVER_TAG, "HTTP DISCONNECTED");
          break;
  }
  return ESP_OK;
}

esp_err_t create_device_handler(esp_http_client_event_t *evt)
{
  cJSON *device;
  cJSON *state;
  cJSON *color_object;
  char* status;
  long color;
  int8_t device_id;
  switch(evt->event_id) {
      case HTTP_EVENT_ERROR:
          ESP_LOGI(SERVER_TAG, "HTTP ERROR");
          break;
      case HTTP_EVENT_ON_CONNECTED:
          ESP_LOGI(SERVER_TAG, "HTTP CONNECTED");
          break;
      case HTTP_EVENT_HEADER_SENT:
          break;
      case HTTP_EVENT_ON_HEADER:
          break;
      case HTTP_EVENT_ON_DATA:
          ESP_LOGI(SERVER_TAG, "Device registered : %.*s", evt->data_len, (char*)evt->data);
          device = cJSON_Parse((char*)evt->data);
          device_id = cJSON_GetObjectItem(device,"id")->valueint;
          state = cJSON_GetObjectItem(device, "state");
          status = cJSON_GetObjectItem(state, "toggle")->valuestring;
          color_object = cJSON_GetObjectItem(state, "color");
          color = cJSON_GetObjectItem(color_object, "argb")->valueint;

          ESP_LOGI(SERVER_TAG, "device id : %i", device_id);
          save_id_to_nvs(device_id);

          handle_switch(status);
          handle_color_changed(color);
          cJSON_Delete(device);
          device_registered = true;
          break;
      case HTTP_EVENT_ON_FINISH:
          ESP_LOGI(SERVER_TAG, "HTTP FINISH");
          break;
      case HTTP_EVENT_DISCONNECTED:
          ESP_LOGI(SERVER_TAG, "HTTP DISCONNECTED");
          break;
  }
  return ESP_OK;
}

void perform_device_request() {
  char* root_url;
  if (load_server_url_from_nvs(&root_url)) {
    int8_t device_id;
    esp_http_client_config_t http_client_config = { };

    bool post_body = false;
    char url[50];
    if (load_id_from_nvs(&device_id)) {
      sprintf(url, "%s/api/devices/%i", root_url, device_id);
      http_client_config.event_handler = FETCH_DEVICE_HANDLER;
    }
    else {
      post_body = true;
      sprintf(url, "%s/api/devices/", root_url);
      http_client_config.method = HTTP_METHOD_POST;
      http_client_config.event_handler = CREATE_DEVICE_HANDLER;
    }
    http_client_config.url = url;
    ESP_LOGI(SERVER_TAG, "Request path : %s", url);

    esp_http_client_handle_t client = esp_http_client_init(&http_client_config);
    esp_http_client_set_header(client, "Content-Type", "application/json");
    esp_http_client_set_header(client, "Accept", "application/json");
    if(post_body) {
      char request_body[50];
      sprintf(
        request_body,
        "{\"type\":\"strip\",\"length\":%i}",
        num_led
      );
      esp_http_client_set_post_field(client, request_body, 50);
    }
    esp_err_t err = esp_http_client_perform(client);
    if (err == ESP_OK) {
      ESP_LOGI(SERVER_TAG, "Status = %d, content_length = %d",
           esp_http_client_get_status_code(client),
           esp_http_client_get_content_length(client));
      if (!device_registered) {
        ESP_LOGI(SERVER_TAG, "It seems that device has been deleted. A new create request should be performed.");
        delete_id_from_nvs();
        perform_device_request();
      }
    }

    else if (err == ESP_ERR_HTTP_CONNECT) {
      ESP_LOGE(SERVER_TAG, "Connection to the server failed. Check if your PixLed server is running, and if the server url looks correct.");
    }
    esp_http_client_cleanup(client);
    free(root_url);
  }
  else {
    ESP_LOGI(SERVER_TAG, "Missing server url.");
  }
}
