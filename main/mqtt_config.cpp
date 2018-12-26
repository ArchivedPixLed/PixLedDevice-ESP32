#include "mqtt_config.h"
#include "main.h"

static esp_err_t mqtt_event_handler(esp_mqtt_event_handle_t event)
{
  esp_mqtt_client_handle_t client = event->client;
  int msg_id;
  // your_context_t *context = event->context;
  switch (event->event_id) {
      case MQTT_EVENT_CONNECTED:
          ESP_LOGI(MQTT_TAG, "MQTT_EVENT_CONNECTED");

          ESP_LOGI(MQTT_TAG, "Color topic : %s", color_topic);
          msg_id = esp_mqtt_client_subscribe(client, color_topic, 0);
          ESP_LOGI(MQTT_TAG, "Color topic subscribe successful, msg_id=%d", msg_id);

          msg_id = esp_mqtt_client_subscribe(client, switch_topic, 0);
          ESP_LOGI(MQTT_TAG, "Switch topic subscribe successful, msg_id=%d", msg_id);

          // Tell the broker that this light is connected.
          // The Android App should subscribe directly to this topic.
          esp_mqtt_client_publish(client, connection_topic, "connected", 0, 1, 1);

          // Stop blink
          vTaskDelete((TaskHandle_t)(event->user_context));
          break;
      case MQTT_EVENT_DISCONNECTED:
          ESP_LOGI(MQTT_TAG, "MQTT_EVENT_DISCONNECTED");
          break;

      case MQTT_EVENT_SUBSCRIBED:
          ESP_LOGI(MQTT_TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
          break;

      case MQTT_EVENT_UNSUBSCRIBED:
          ESP_LOGI(MQTT_TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
          break;

      case MQTT_EVENT_PUBLISHED:
          ESP_LOGI(MQTT_TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
          break;

      case MQTT_EVENT_ERROR:
          ESP_LOGI(MQTT_TAG, "MQTT_EVENT_ERROR");
          break;

      case MQTT_EVENT_DATA:
          ESP_LOGI(MQTT_TAG, "MQTT_EVENT_DATA");
          printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
          printf("COLOR_TOPIC=%s\r\n", color_topic);
          printf("DATA=%.*s\r\n", event->data_len, event->data);
          size_t topic_len = event->topic_len;
          char topic_str[topic_len + 1];
          for (int i = 0; i < topic_len + 1; i++) {
            topic_str[i] = event->topic[i];
          }
          topic_str[topic_len] = '\0';

          if (strcmp(topic_str, color_topic) == 0) {
            handle_color_changed(event->data_len, event->data);
          }

          else if (strcmp(topic_str, switch_topic) == 0) {
            handle_switch(event->data_len, event->data);
          }
          break;

  }
  return ESP_OK;
}

void mqtt_app_start()
{
  ESP_LOGI(MAIN_TAG, "Init topics");
  sprintf(client_id, "light_%i", LIGHT_ID);
  sprintf(color_topic, "/buildings/-1/rooms/-10/lights/%i/color", LIGHT_ID);
  sprintf(switch_topic, "/buildings/-1/rooms/-10/lights/%i/switch", LIGHT_ID);
  sprintf(connection_topic, "/buildings/-1/rooms/-10/lights/%i/connection", LIGHT_ID);

  uint32_t delay_ms = 300;

  TaskHandle_t blinkLedTaskHandler;
  xTaskCreate(&blink_task, "blink_connect_mqtt", configMINIMAL_STACK_SIZE, (void*)&delay_ms, 5, &blinkLedTaskHandler);
  vTaskDelay(2000 / portTICK_PERIOD_MS);

  esp_mqtt_client_config_t mqtt_cfg = { };
  mqtt_cfg.uri = "mqtt://192.168.1.124";
  mqtt_cfg.event_handle = mqtt_event_handler;
  mqtt_cfg.user_context = (void*)blinkLedTaskHandler;
  mqtt_cfg.client_id = client_id;
  // If this light disconnect, devices subscribed to this topic (e.g. Spring
  // server and Android apps) will be warned that this module has disconnected.
  mqtt_cfg.lwt_topic = connection_topic;
  mqtt_cfg.lwt_msg = "disconnected";

  esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
  esp_mqtt_client_start(client);
}
