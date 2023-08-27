#include "menu_manager.h"
#include "esp_err.h"
#include "freertos/portmacro.h"
#include "sdkconfig.h"
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

const static char *TAG = "menu_menager";

TaskHandle_t current_function;

void menu_init(void *args) {
  ESP_LOGI(TAG, "Start menu");
  menu_config_t *params = (menu_config_t *)args;
  menu_path_t steck_path[CONFIG_MAX_DEPTH_PATH];
  menu_path_t path;
  uint8_t depth = 0;
  Navigate_t input_command;

  path.current_index = 0;
  path.current_menu = &params->root;
  steck_path[depth] = path;

  ESP_LOGI(TAG, "Root Title: %s", path.current_menu->label);

  while (true) {
    params->display(&path);
    ESP_LOGI(TAG, "Updated display");
    input_command = params->input();

    switch (input_command) {

    case NAVIGATE_UP:
      ESP_LOGI(TAG, "Command UP");
      path.current_index++;
      path.current_index %= path.current_menu->num_options;
      break;

    case NAVIGATE_DOWN:
      ESP_LOGI(TAG, "Command DOWN");
      path.current_index =
          (path.current_menu->num_options + path.current_index - 1) %
          path.current_menu->num_options;
      break;

    case NAVIGATE_SELECT:
      ESP_LOGI(TAG, "Command SELECT");
      if (path.current_menu->submenus[path.current_index].function) {

        xTaskCreatePinnedToCore(
            path.current_menu->submenus[path.current_index].function,
            "Function_by_menu", 10240, NULL, 5, &current_function, 1);
        vTaskDelay(100 / portTICK_PERIOD_MS);
      } else {
        path.current_menu = &path.current_menu->submenus[path.current_index];
        path.current_index = 0;
        depth++;
        steck_path[depth] = path;
        vTaskDelay(100 / portTICK_PERIOD_MS);
      }
      break;

    case NAVIGATE_BACK:
      ESP_LOGI(TAG, "Command BACK");
      depth--;
      path = steck_path[depth];
      break;

    default:
      ESP_LOGW(TAG, "Undenfined Input");
      break;
    }
  }
}
