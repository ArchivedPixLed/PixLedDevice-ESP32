#include <string.h>
#include "mdns_config.h"
#include "mdns.h"
#include "esp_log.h"

#define MQTT_SERVICE_NAME "PixLedBroker"
#define SERVER_SERVICE_NAME "PixLedServer"
#define MDNS_TAG "MDNS"
#define MAX_RETRY 3
#define MDNS_TIMEOUT 1500

static int retry = 0;
static bool found = false;

static const char * if_str[] = {"STA", "AP", "ETH", "MAX"};
static const char * ip_protocol_str[] = {"V4", "V6", "MAX"};

static bool check_results(mdns_result_t * results, char* ip, char* port){
    mdns_result_t * r = results;
    mdns_ip_addr_t * a = NULL;

    int i = 1;
    while(r){
      ESP_LOGD(MDNS_TAG, "%d: Interface: %s, Type: %s\n", i++, if_str[r->tcpip_if], ip_protocol_str[r->ip_protocol]);
      if (r->instance_name) {
        if (strcmp(r->instance_name, MQTT_SERVICE_NAME) == 0 || strcmp(r->instance_name, SERVER_SERVICE_NAME) == 0) {
          if(r->hostname){
              ESP_LOGI(MDNS_TAG, "Server : %s.local:%u", r->hostname, r->port);
          }
          a = r->addr;
          if(a){
              if(a->addr.type == MDNS_IP_PROTOCOL_V6){
                  // printf("  AAAA: " IPV6STR "\n", IPV62STR(a->addr.u_addr.ip6));
                  // ESP_LOGI(MDNS_TAG, "Service found at mqtt://" IPV6STR ":%u/", IPV62STR(a->addr.u_addr.ip6), r->port);
                  ESP_LOGW(MDNS_TAG, "IPV6 protocol is currently not fully handled.");
              } else {
                  // printf("  A   : " IPSTR "\n", IP2STR(&(a->addr.u_addr.ip4)));
                  ESP_LOGI(MDNS_TAG, "Service found at " IPSTR ":%u", IP2STR(&(a->addr.u_addr.ip4)), r->port);
                  sprintf(ip, IPSTR, IP2STR(&(a->addr.u_addr.ip4)));
                  sprintf(port, "%u", r->port);
                  return true;
              }
            }
          }
        }
        r = r->next;
      }
      return false;
    }

void init_mdns() {
  ESP_ERROR_CHECK( mdns_init() );
}

void clean_mdns() {
  ESP_LOGI(MDNS_TAG, "Cleaning MDNS...");
  mdns_free();
}

bool look_for_mqtt_broker(char* ip, char* port) {
  mdns_result_t * results = NULL;
  ESP_ERROR_CHECK( mdns_query_ptr("_mqtt", "_tcp", MDNS_TIMEOUT, 5,  &results) );
  if(!results){
    retry++;
    if (retry < MAX_RETRY) {
      ESP_LOGI(MDNS_TAG, "No mqtt broker found, retry.");
      return look_for_mqtt_broker(ip, port);
    }
    else {
      retry = 0;
      ESP_LOGE(MDNS_TAG, "No mqtt broker found!");
      mdns_query_results_free(results);
      return false;
    }
  }
  else {
    retry = 0;
    bool broker_found = check_results(results, ip, port);
    mdns_query_results_free(results);
    return broker_found;
  }
}

bool look_for_server(char* ip, char* port) {
  mdns_result_t * results = NULL;
  ESP_ERROR_CHECK( mdns_query_ptr("_http", "_tcp", MDNS_TIMEOUT, 5,  &results) );
  if(!results){
    retry++;
    if (retry < MAX_RETRY) {
      ESP_LOGI(MDNS_TAG, "No server found, retry.");
      return look_for_mqtt_broker(ip, port);
    }
    else {
      retry = 0;
      ESP_LOGE(MDNS_TAG, "No server found!");
      mdns_query_results_free(results);
      return false;
    }
  }
  else {
    retry = 0;
    bool server_found = check_results(results, ip, port);
    mdns_query_results_free(results);
    return server_found;
  }
}
