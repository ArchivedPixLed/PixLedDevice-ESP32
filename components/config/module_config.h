#include "main.h"
#include "WS2812.h"

#define MODULE_TAG "MODULE"

static pixel_t last_color = { };
static bool on;
extern WS2812* strip;
extern uint16_t num_led;

void init_strip();
void save_led_number_to_nvs(uint16_t led_number);
bool load_led_number_from_nvs(uint16_t* led_number);
void handle_color_changed(long color);
void handle_switch(const char* switch_str);
