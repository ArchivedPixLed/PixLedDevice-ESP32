#include "nvs_flash.h"
#include "esp_http_client.h"

#define NUM_LED CONFIG_NUM_LED

#define SERVER_URL CONFIG_SERVER_URL
#define FETCH_DEVICE_HANDLER fetch_device_handler
#define CREATE_DEVICE_HANDLER create_device_handler

void save_id_to_nvs();
bool load_id_from_nvs(int8_t* device_id);
void delete_id_from_nvs();
void save_server_url_to_nvs(const char* url);
bool load_server_url_from_nvs(char** url);
void perform_device_request();

esp_err_t fetch_device_handler(esp_http_client_event_t *evt);
esp_err_t create_device_handler(esp_http_client_event_t *evt);
