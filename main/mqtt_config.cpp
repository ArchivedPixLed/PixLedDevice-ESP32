#include "mqtt_config.h"
#include "main.h"

static esp_err_t mqtt_event_handler(esp_mqtt_event_handle_t event)
{
  esp_mqtt_client_handle_t client = event->client;
  int msg_id;
  struct mqtt_context* context = (mqtt_context*)(event->user_context);
  // your_context_t *context = event->context;
  switch (event->event_id) {
      case MQTT_EVENT_CONNECTED:
          ESP_LOGI(MQTT_TAG, "MQTT_EVENT_CONNECTED");
          context->connected=true;
          // Publish light_id to /connected topic
          esp_mqtt_client_publish(client, connection_topic, light_id, 0, 1, 0);

          // (Re)initialize subscribtions
          context->subscribed_to_switch_topic=false;
          context->subscribed_to_color_topic=false;

          // Firstly, connect to switch topic, and wait for the first switch initialization message.
          // -> go to MQTT_EVENT_DATA
          ESP_LOGI(MQTT_TAG, "Subscribing to switch topic...");
          esp_mqtt_client_subscribe(client, switch_topic, 1);
          break;
      case MQTT_EVENT_DISCONNECTED:
          ESP_LOGI(MQTT_TAG, "MQTT_EVENT_DISCONNECTED");
          context->connected=false;
          break;

      case MQTT_EVENT_SUBSCRIBED:
          ESP_LOGI(MQTT_TAG, "MQTT_EVENT_SUBSCRIBED, topic=%.*s, msg_id=%d",event->topic_len, event->topic, event->msg_id);
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
          printf("DATA=%.*s\r\n", event->data_len, event->data);
          size_t topic_len = event->topic_len;
          char topic_str[topic_len + 1];
          for (int i = 0; i < topic_len + 1; i++) {
            topic_str[i] = event->topic[i];
          }
          topic_str[topic_len] = '\0';

          if (strcmp(topic_str, check_topic) == 0) {
            esp_mqtt_client_publish(client, connection_topic, light_id, 0, 1, 0);
          }


          else if (strcmp(topic_str, switch_topic) == 0) {
            handle_switch(event->data_len, event->data);
            ESP_LOGI(MQTT_TAG, "subscribed_to_switch_topic : %s", context->subscribed_to_switch_topic ? "true" : "false");
            if(!context->subscribed_to_switch_topic) {
              // subscribed_to_switch_topic was false, so this is the initialization message.
              context->subscribed_to_switch_topic=true;

              // Then, we connect to color_topic
              ESP_LOGI(MQTT_TAG, "Subscribing to color topic...");
              esp_mqtt_client_subscribe(client, color_topic, 1);
            }
          }

          else if (strcmp(topic_str, color_topic) == 0) {
            handle_color_changed(event->data_len, event->data);
            if(!context->subscribed_to_color_topic){
              // subscribed_to_color_topic was false, so this is the initialization message.
              context->subscribed_to_color_topic=true;
              ESP_LOGI(MQTT_TAG, "Initialization complete!");
              esp_mqtt_client_subscribe(client, check_topic, 1);
            }
          }

          break;

  }
  return ESP_OK;
}

void mqtt_app_start()
{
  // Topics initialization
  ESP_LOGI(MAIN_TAG, "Init topics");
  sprintf(light_id, "%i", LIGHT_ID);
  sprintf(client_id, "light_%i", LIGHT_ID);
  sprintf(color_topic, "/buildings/-1/rooms/-10/lights/%i/color", LIGHT_ID);
  sprintf(switch_topic, "/buildings/-1/rooms/-10/lights/%i/switch", LIGHT_ID);

  // Blink on board led
  uint32_t delay_ms = 300;
  TaskHandle_t blinkLedTaskHandler;
  xTaskCreate(&blink_task, "blink_connect_mqtt", configMINIMAL_STACK_SIZE, (void*)&delay_ms, 5, &blinkLedTaskHandler);
  vTaskDelay(2000 / portTICK_PERIOD_MS);

  ESP_LOGI(MQTT_TAG, "Connecting to broker...");

  // Context initialization
  struct mqtt_context context {false, false, false};

  // Mqtt client initialization
  esp_mqtt_client_config_t mqtt_cfg = { };
  mqtt_cfg.uri = MQTT_BROKER_URI;
  mqtt_cfg.event_handle = mqtt_event_handler;
  mqtt_cfg.user_context = (void*)&context;
  mqtt_cfg.client_id = client_id;
  mqtt_cfg.keepalive = 5;
  // If this light disconnect, devices subscribed to this topic (e.g. Spring
  // server and Android apps) will be warned that this module has disconnected.
  mqtt_cfg.lwt_topic = disconnection_topic;
  mqtt_cfg.lwt_msg = light_id;
  mqtt_cfg.lwt_qos = 1;
  mqtt_cfg.lwt_retain = 0;

  esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
  esp_mqtt_client_start(client);


  vTaskDelete(blinkLedTaskHandler);
  gpio_set_level((gpio_num_t) BLINK_GPIO, 0);
}
