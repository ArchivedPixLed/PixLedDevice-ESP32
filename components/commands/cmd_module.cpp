#include "cmd_server.h"

#include "esp_console.h"
#include "esp_err.h"
#include "argtable3/argtable3.h"
#include "esp_log.h"
#include "module_config.h"

#define MODULE_CMD_TAG "MODULE"

static struct {
    struct arg_lit *check;
    struct arg_int *led_number;
    struct arg_end *end;
} module_args;

static int handle_module(int argc, char** argv) {
  int nerrors = arg_parse(argc, argv, (void**) &module_args);
  if (nerrors != 0) {
      arg_print_errors(stderr, module_args.end, argv[0]);
      return 1;
  }
  if (module_args.check->count > 0){
    uint16_t led_number;
    if (load_led_number_from_nvs(&led_number)) {
      ESP_LOGI(MODULE_CMD_TAG, "Currently stored led number : %i", led_number);
    }
    else {
      ESP_LOGI(MODULE_CMD_TAG, "No led number stored.");
    }
  }
  if (module_args.led_number->count > 0) {
    save_led_number_to_nvs(*module_args.led_number->ival);
  }
  return 0;
}

void register_module()
{
    module_args.check = arg_lit0("c", "check", "check current stored configuration");
    module_args.led_number = arg_int0("n", "numled", "<n>", "set led number");
    module_args.end = arg_end(3);

    esp_console_cmd_t module_cmd = { };
    module_cmd.command = "module";
    module_cmd.help = "Set up module config.";
    module_cmd.hint = NULL;
    module_cmd.func = &handle_module;
    module_cmd.argtable = &module_args;

    ESP_ERROR_CHECK( esp_console_cmd_register(&module_cmd) );
}
