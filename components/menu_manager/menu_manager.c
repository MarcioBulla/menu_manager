// TODO: Add comments
#include "menu_manager.h"
#include "esp_err.h"
#include "freertos/portmacro.h"
#include "freertos/projdefs.h"
#include "sdkconfig.h"
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

const static char *TAG = "menu_menager";

TaskHandle_t MMfunction = NULL;
SemaphoreHandle_t MMmutex;

menu_path_t steck_path[CONFIG_MAX_DEPTH_PATH];
menu_path_t path;
uint8_t depth = 0;
Navigate_t input_command;

void NavigationUp(bool loop) {
  ESP_LOGI(TAG, "Command UP");

  if (path.current_index < path.current_menu->num_options - 1 || loop) {
    path.current_index++;
    path.current_index %= path.current_menu->num_options;
  }
}

void NavigationDown(bool loop) {
  ESP_LOGI(TAG, "Command DOWN");
  if (path.current_index > 0 || loop) {
    path.current_index =
        (path.current_menu->num_options + path.current_index - 1) %
        path.current_menu->num_options;
  }
}

void ExecFunction() {
  ESP_LOGI(TAG, "Execute Function");
  xSemaphoreTake(MMmutex, portMAX_DELAY);
  xTaskCreatePinnedToCore(
      path.current_menu->submenus[path.current_index].function,
      "Function_by_menu", 10240, NULL, 5, &MMfunction, 1);
}

void SelectionOption() {
  ESP_LOGI(TAG, "Open Submenu");
  path.current_menu = &path.current_menu->submenus[path.current_index];
  path.current_index = 0;
  depth++;
  steck_path[depth] = path;
}

void NavigationBack() {
  ESP_LOGI(TAG, "Command BACK");
  depth--;
  path = steck_path[depth];
#if !CONFIG_SALVE_INDEX
  path.current_index = 0;
#endif
}

void BackFunction() {
  if (MMfunction != NULL) {
    ESP_LOGI(TAG, "Delete Function");
    vTaskDelete(MMfunction);
  }
}

void menu_init(void *args) {
  ESP_LOGI(TAG, "Start menu");
  menu_config_t *params = (menu_config_t *)args;

  MMmutex = xSemaphoreCreateBinary();
  xSemaphoreGive(MMmutex);

  path.current_index = 0;
  path.current_menu = &params->root;
  steck_path[depth] = path;
  params->display(&path);

  ESP_LOGI(TAG, "Root Title: %s", path.current_menu->label);

  while (true) {
    input_command = params->input();

    if (xSemaphoreTake(MMmutex, 0) == pdTRUE) {
      xSemaphoreGive(MMmutex);

      switch (input_command) {

      case NAVIGATE_UP:
        NavigationUp(params->loop);
        params->display(&path);
        break;

      case NAVIGATE_DOWN:
        NavigationDown(params->loop);
        params->display(&path);
        break;

      case NAVIGATE_SELECT:
        if (path.current_menu->submenus[path.current_index].function) {
          ExecFunction();
        } else {
          SelectionOption();
          params->display(&path);
        }
        break;

      case NAVIGATE_BACK:
        if (depth) {
          NavigationBack();
          params->display(&path);
        }
        break;

      default:
        ESP_LOGW(TAG, "Undenfined Input");
        break;
      }
    } else if (input_command == NAVIGATE_BACK) {
      BackFunction();
      params->display(&path);
      xSemaphoreGive(MMmutex);
    }
  }
}
