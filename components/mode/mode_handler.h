#include "main.h"

#if CONFIG_MODE_HANDLER

#define PIN_MODE_1 CONFIG_MODE_PIN_1
#define PIN_MODE_2 CONFIG_MODE_PIN_2

#include "driver/gpio.h"
#include "esp_log.h"
#include "console_config.h"


#define DEBOUNCE_TIMEOUT 1000
#define MODE_TASKSIZE 4096

#define MODE_TAG "MODE"
// #define PIN_MODE_1 13
// #define PIN_MODE_2 14
#define GPIO_MODE_PIN_SEL  ((1ULL<<PIN_MODE_1) | (1ULL<<PIN_MODE_2))

#define DEFAULT_MODE 0
#define CMD_MODE 1
#define SLEEP_MODE 2

void initialize_mode_handler();
void switchMode();

void enterCommandLineMode();
void quitCommandLineMode();

#endif
