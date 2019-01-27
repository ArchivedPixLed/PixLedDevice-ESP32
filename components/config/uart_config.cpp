#include "uart_config.h"
#include "esp_console.h"
#include "esp_vfs_dev.h"
#include "linenoise/linenoise.h"

#include "cmd_wifi.h"
#include "cmd_mqtt.h"

// static QueueHandle_t uart0_queue;

void initialize_uart() {

  /* Disable buffering on stdin and stdout */
  setvbuf(stdin, NULL, _IONBF, 0);
  setvbuf(stdout, NULL, _IONBF, 0);

  /* Minicom, screen, idf_monitor send CR when ENTER key is pressed */
  esp_vfs_dev_uart_set_rx_line_endings(ESP_LINE_ENDINGS_CR);
  /* Move the caret to the beginning of the next line on '\n' */
  esp_vfs_dev_uart_set_tx_line_endings(ESP_LINE_ENDINGS_CRLF);

  uart_config_t uart_config = { };
  uart_config.baud_rate = CONFIG_CONSOLE_UART_BAUDRATE;
  uart_config.data_bits = UART_DATA_8_BITS;
  uart_config.parity = UART_PARITY_DISABLE;
  uart_config.stop_bits = UART_STOP_BITS_1;
  uart_config.use_ref_tick = true;

  ESP_ERROR_CHECK( uart_param_config((uart_port_t) CONFIG_CONSOLE_UART_NUM, &uart_config) );

  /* Install UART driver for interrupt-driven reads and writes */
  ESP_ERROR_CHECK( uart_driver_install((uart_port_t) CONFIG_CONSOLE_UART_NUM,
        256, 256, 0, NULL, 0) );

  esp_console_config_t console_config = { };
  console_config.max_cmdline_args = 8;
  console_config.max_cmdline_length = 256;

  /* Tell VFS to use UART driver */
  esp_vfs_dev_uart_use_driver(CONFIG_CONSOLE_UART_NUM);

  ESP_ERROR_CHECK( esp_console_init(&console_config) );

  linenoiseSetMultiLine(1);

  /* Tell linenoise where to get command completions and hints */
  linenoiseSetCompletionCallback(&esp_console_get_completion);

  /* Register commands */
  esp_console_register_help_command();
  register_wifi();
  register_mqtt();

  /* Prompt to be printed before each line.
       * This can be customized, made dynamic, etc.
       */
  const char* prompt = LOG_COLOR_I "pixled> " LOG_RESET_COLOR;

  printf("\n"
         "Welcome to the PixLed Strip console configuration!\n"
         "Type 'help' to get the list of commands.\n");

  /* Figure out if the terminal supports escape sequences */
  int probe_status = linenoiseProbe();
  if (probe_status) { /* zero indicates success */
      printf("\n"
             "Your terminal application does not support escape sequences.\n"
             "Line editing and history features are disabled.\n"
             "On Windows, try using Putty instead.\n");
      linenoiseSetDumbMode(1);
      }

      /* Main loop */
      while(true) {
          /* Get a line using linenoise.
           * The line is returned when ENTER is pressed.
           */
          char* line = linenoise(prompt);
          if (line == NULL) { /* Ignore empty lines */
              printf("\n");
              continue;
          }
          printf("\n");
          /* Add the command to the history */
          linenoiseHistoryAdd(line);

          /* Try to run the command */
          int ret;
          esp_err_t err = esp_console_run(line, &ret);
          if (err == ESP_ERR_NOT_FOUND) {
              printf("Unrecognized command.\n"
                     "Type 'help' to get available commands.\n"
                    );
          } else if (err == ESP_ERR_INVALID_ARG) {
              // command was empty
          } else if (err == ESP_OK && ret != ESP_OK) {
              printf("Command returned non-zero error code: 0x%x (%s)\n", ret, esp_err_to_name(err));
          } else if (err != ESP_OK) {
              printf("Internal error: %s\n", esp_err_to_name(err));
          }
          /* linenoise allocates line buffer on the heap, so need to free it */
          linenoiseFree(line);
      }
}
