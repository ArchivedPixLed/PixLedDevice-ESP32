#include "wifi_config.h"
#include "mqtt_config.h"
#include "main.h"

static int s_retry_num = 0;

static esp_err_t wifi_event_handler(void *blinkLedTaskHandler, system_event_t *event)
{
    switch(event->event_id) {
    case SYSTEM_EVENT_STA_START:
        esp_wifi_connect();
        break;
    case SYSTEM_EVENT_STA_GOT_IP:
        ESP_LOGI(WIFI_TAG, "got ip:%s",
                 ip4addr_ntoa(&event->event_info.got_ip.ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
        vTaskDelete((TaskHandle_t) blinkLedTaskHandler);
        ESP_LOGI(MAIN_TAG, "Connect to MQTT");
        mqtt_app_start();
        break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
        {
            if (s_retry_num < MAXIMUM_RETRY) {
                esp_wifi_connect();
                xEventGroupClearBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
                s_retry_num++;
                ESP_LOGI(WIFI_TAG,"retry to connect to the AP");
            }
            ESP_LOGI(WIFI_TAG,"connect to the AP fail\n");
            break;
        }
    default:
        break;
    }
    return ESP_OK;
}

void wifi_init_sta()
{
    uint32_t delay_ms = 100;
    TaskHandle_t blinkLedTaskHandler;
    xTaskCreate(&blink_task, "blink_connect_wifi", configMINIMAL_STACK_SIZE, (void*)&delay_ms, 5, &blinkLedTaskHandler);
    s_wifi_event_group = xEventGroupCreate();

    tcpip_adapter_init();
    ESP_ERROR_CHECK(esp_event_loop_init(wifi_event_handler, (void*)blinkLedTaskHandler));
    //ESP_ERROR_CHECK(esp_event_loop_init(wifi_event_handler, NULL));

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    wifi_config_t wifi_config = { };
    strcpy((char*) wifi_config.sta.ssid, WIFI_SSID);
    strcpy((char*) wifi_config.sta.password, WIFI_PASS);

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config) );
    ESP_ERROR_CHECK(esp_wifi_start() );

    ESP_LOGI(WIFI_TAG, "wifi_init_sta finished.");
    ESP_LOGI(WIFI_TAG, "connect to ap SSID:%s password:%s",
             WIFI_SSID, WIFI_PASS);
}
