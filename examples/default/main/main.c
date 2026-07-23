#include "menu_manager.h"
#include <esp_log.h>
#include <esp_system.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/semphr.h>
#include <freertos/task.h>
#include <sdkconfig.h>
#include <stdbool.h>
#include <stdio.h>

static const char *TAG = "main";

void dumb(void *args) {
  ESP_LOGI(TAG, "I am dumb");
  while (true) {
    vTaskDelay(10000 / portTICK_PERIOD_MS);
  }
}

menu_node_t submenu[3] = {
    {.label = "funcA", .function = &dumb},
    {.label = "funcB", .function = &dumb},
    {.label = "funcC", .function = &dumb},
};

menu_node_t root = {
    .label = "root",
    .num_options = 3,
    .submenus = (menu_node_t[3]){
        {.label = "submenu1", .submenus = submenu, .num_options = 3},
        {.label = "submenu2", .submenus = submenu, .num_options = 3},
        {.label = "submenu3", .submenus = submenu, .num_options = 3},
    }};

static const char *command_name(Navigate_t command) {
  switch (command) {
  case NAVIGATE_UP:
    return "UP";
  case NAVIGATE_DOWN:
    return "DOWN";
  case NAVIGATE_SELECT:
    return "SELECT";
  case NAVIGATE_BACK:
    return "BACK";
  default:
    return "UNKNOWN";
  }
}

static void send_command(Navigate_t command) {
  ESP_LOGI(TAG, "Command received: %s", command_name(command));
  xQueueSend(qCommands, &command, portMAX_DELAY);
}

void simula_input(void *args) {
  vTaskDelay(6000 / portTICK_PERIOD_MS);
  send_command(NAVIGATE_UP);
  vTaskDelay(5000 / portTICK_PERIOD_MS);
  send_command(NAVIGATE_SELECT);
  vTaskDelay(5000 / portTICK_PERIOD_MS);
  send_command(NAVIGATE_DOWN);
  vTaskDelay(5000 / portTICK_PERIOD_MS);
  send_command(NAVIGATE_SELECT);
  vTaskDelay(5000 / portTICK_PERIOD_MS);
  send_command(NAVIGATE_BACK);
  vTaskDelay(5000 / portTICK_PERIOD_MS);
  send_command(NAVIGATE_BACK);
  vTaskDelay(10000 / portTICK_PERIOD_MS);

  ESP_LOGI(TAG, "Finalizada");
  vTaskDelete(NULL);
}

void display(menu_path_t *current_path) {
  menu_node_t *menu = current_path->current_menu;

  printf("\n========== MENU ==========\n");
  printf("Menu: %s\n", menu->label);
  printf("--------------------------\n");
  for (size_t position = menu->num_options; position > 0; position--) {
    size_t index = position - 1;
    printf("%c %s\n", index == current_path->current_index ? '>' : ' ',
           menu->submenus[index].label);
  }
  printf("==========================\n\n");
  fflush(stdout);
}

void app_main(void) {

  static menu_config_t config;
  config = (menu_config_t){
      .root = root,
      .loop = true,
      .display = &display,
  };

  xTaskCreate(menu_init, "menu_init", 2048, &config, 3, NULL);
  vTaskDelay(5000 / portTICK_PERIOD_MS);
  xTaskCreate(simula_input, "simula", 2048, NULL, 1, NULL);
  vTaskDelete(NULL);
}
