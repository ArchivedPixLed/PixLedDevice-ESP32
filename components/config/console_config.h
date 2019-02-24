#include "driver/uart.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define CONSOLE_STACK_SIZE 6144

void initialize_console();
void clean_console();
