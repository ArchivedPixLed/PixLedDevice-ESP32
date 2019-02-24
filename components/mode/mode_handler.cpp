#include "mode_handler.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

int currentMode = -1;

static void IRAM_ATTR mode_handler(void* arg)
{
  vTaskNotifyGiveFromISR((TaskHandle_t) arg, NULL);
}

static void mode_task(void* arg)
{
    uint32_t notify_value;

    //Test xTaskNotifyWait from task
    while(1) {
        ESP_LOGI(MODE_TAG, "Waiting for mode switch.");
        // Wake up the task at first "bounce"
        xTaskNotifyWait(0, 0xFFFFFFFF, &notify_value, portMAX_DELAY);
        // Wait to be sure that switch state is well defined
        vTaskDelay(DEBOUNCE_TIMEOUT / portTICK_PERIOD_MS);
        switchMode();
    }
}

void switchMode() {
  // Compute mode
  int mode = 0 | (gpio_get_level((gpio_num_t) PIN_MODE_2) << 1) | gpio_get_level((gpio_num_t) PIN_MODE_1);
  if (currentMode != DEFAULT_MODE && mode == DEFAULT_MODE) {
    ESP_LOGI(MODE_TAG, "Switch to default mode");
    if (currentMode == CMD_MODE) {
      clean_console();
      ESP_LOGI(MODE_TAG, "Quit command line mode OK");
    }
    launch_default_mode();
  }
  else if (currentMode != CMD_MODE && mode == CMD_MODE) {
    ESP_LOGI(MODE_TAG, "Switch to command line mode");
    if (currentMode == DEFAULT_MODE) {
      quit_default_mode();
    }
    initialize_console();
  }
  else if (mode == SLEEP_MODE) {
    ESP_LOGI(MODE_TAG, "Sleep mode");
  }
  currentMode = mode;
}

void initialize_mode_handler() {
    gpio_config_t io_conf;

    io_conf.intr_type = GPIO_INTR_ANYEDGE;
    //bit mask of the mode pins
    io_conf.pin_bit_mask = GPIO_MODE_PIN_SEL;
    //set as input mode
    io_conf.mode = GPIO_MODE_INPUT;
    //enable pull-up mode
    io_conf.pull_up_en = (gpio_pullup_t) 1;

    gpio_config(&io_conf);

    TaskHandle_t mode_task_handler;
    xTaskCreate(mode_task, "mode task", MODE_TASKSIZE, NULL, 10, &mode_task_handler);

    //install gpio isr service
    gpio_install_isr_service(ESP_INTR_FLAG_EDGE);
    // hook isr handler for mode pins
    gpio_isr_handler_add((gpio_num_t) PIN_MODE_1, mode_handler, (void*) mode_task_handler);
    gpio_isr_handler_add((gpio_num_t) PIN_MODE_2, mode_handler, (void*) mode_task_handler);

}
