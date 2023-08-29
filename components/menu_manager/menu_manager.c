// FIX: Execution function.
// TODO: condition back in root
// TODO: verify if vTaskDelete stop task
// NOTE: Thing if quick press restart function because maybe the function use
// this key

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

static TaskHandle_t current_function;
SemaphoreHandle_t mutex;

menu_path_t steck_path[CONFIG_MAX_DEPTH_PATH];
menu_path_t path;
uint8_t depth = 0;
Navigate_t input_command;

void NavigationUp() {
  ESP_LOGI(TAG, "Command UP");
  path.current_index++;
  path.current_index %= path.current_menu->num_options;
}

void NavigationDown() {
  ESP_LOGI(TAG, "Command DOWN");
  path.current_index =
      (path.current_menu->num_options + path.current_index - 1) %
      path.current_menu->num_options;
}

void ExecFunction() {
  ESP_LOGI(TAG, "Execute Function");
  xTaskCreatePinnedToCore(
      path.current_menu->submenus[path.current_index].function,
      "Function_by_menu", 10240, NULL, 5, &current_function, 1);
}

void SelectionOption() {
  ESP_LOGI(TAG, "Command SELECT");
  if (path.current_menu->submenus[path.current_index].function) {
    ExecFunction();
  } else {
    ESP_LOGI(TAG, "Open Submenu");
    path.current_menu = &path.current_menu->submenus[path.current_index];
    path.current_index = 0;
    depth++;
    steck_path[depth] = path;
  }
}

void NavigationBack() {
  ESP_LOGI(TAG, "Command BACK");
  depth--;
  path = steck_path[depth];
}

void BackFunction() {
  if (current_function != NULL) {
    ESP_LOGI(TAG, "Delete Function");
    vTaskDelete(current_function);
  }
}

void ResetFunction() {
  BackFunction();
  ExecFunction();
}

void menu_init(void *args) {
  ESP_LOGI(TAG, "Start menu");
  menu_config_t *params = (menu_config_t *)args;

  mutex = xSemaphoreCreateMutex();
  xSemaphoreGive(mutex);

  path.current_index = 0;
  path.current_menu = &params->root;
  steck_path[depth] = path;

  ESP_LOGI(TAG, "Root Title: %s", path.current_menu->label);

  while (true) {
    if (xSemaphoreTake(mutex, 0) == pdTRUE) {
      params->display(&path);
      ESP_LOGI(TAG, "Updated display");
      xSemaphoreGive(mutex);
    }
    input_command = params->input();

    if (xSemaphoreTake(mutex, 0) == pdTRUE) {
      xSemaphoreGive(mutex);

      switch (input_command) {

      case NAVIGATE_UP:
        NavigationUp();
        break;

      case NAVIGATE_DOWN:
        NavigationDown();
        break;

      case NAVIGATE_SELECT:
        SelectionOption();
        break;

      case NAVIGATE_BACK:
        NavigationBack();
        break;

      default:
        ESP_LOGW(TAG, "Undenfined Input");
        break;
      }
    } else {
      switch (input_command) {

      case NAVIGATE_SELECT:
        ResetFunction();
        break;

      case NAVIGATE_BACK:
        BackFunction();
        xSemaphoreGive(mutex);
        break;

      default:
        ESP_LOGW(TAG, "Undenfined Input");
        break;
      }
    }
  }
}
