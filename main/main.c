#include "freertos/portmacro.h"
#include "menu_manager.h"
#include <esp_log.h>
#include <esp_system.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/semphr.h>
#include <freertos/task.h>
#include <hd44780.h>
#include <pcf8574.h>
#include <sdkconfig.h>
#include <stdbool.h>
#include <stdio.h>

static const char *TAG = "main";

QueueHandle_t qinput;

void dumb(void *args) {
  ESP_LOGI(TAG, "I am dumb");
  vTaskDelete(NULL);
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

static i2c_dev_t pcf8574;

static esp_err_t write_lcd_data(const hd44780_t *lcd, uint8_t data) {
  return pcf8574_port_write(&pcf8574, data);
}

hd44780_t lcd = {.write_cb = write_lcd_data,
                 .font = HD44780_FONT_5X8,
                 .lines = 4,
                 .pins = {
                     .rs = 0,
                     .e = 2,
                     .d4 = 4,
                     .d5 = 5,
                     .d6 = 6,
                     .d7 = 7,
                     .bl = 3,
                 }};

void start_lcd(void) {
  ESP_ERROR_CHECK(i2cdev_init());
  ESP_ERROR_CHECK(pcf8574_init_desc(&pcf8574, 0x27, 0, 21, 22));
  ESP_ERROR_CHECK(hd44780_init(&lcd));

  ESP_LOGI(TAG, "LCD ON");
}

void simula_input(void *args) {
  Navigate_t teste = NAVIGATE_UP;
  vTaskDelay(3000 / portTICK_PERIOD_MS);
  ESP_LOGI(TAG, "NEXT");
  xQueueSend(qinput, &teste, portMAX_DELAY);
  vTaskDelay(1000 / portTICK_PERIOD_MS);
  teste = NAVIGATE_SELECT;
  ESP_LOGI(TAG, "SELECT");
  xQueueSend(qinput, &teste, portMAX_DELAY);
  vTaskDelay(1000 / portTICK_PERIOD_MS);
  teste = NAVIGATE_DOWN;
  ESP_LOGI(TAG, "DOWN");
  xQueueSend(qinput, &teste, portMAX_DELAY);
  vTaskDelay(1000 / portTICK_PERIOD_MS);
  teste = NAVIGATE_SELECT;
  ESP_LOGI(TAG, "SELECT");
  xQueueSend(qinput, &teste, portMAX_DELAY);
  vTaskDelay(1000 / portTICK_PERIOD_MS);
  teste = NAVIGATE_BACK;
  ESP_LOGI(TAG, "BACK");
  xQueueSend(qinput, &teste, portMAX_DELAY);
  vTaskDelay(3000 / portTICK_PERIOD_MS);
  ESP_LOGI(TAG, "Finalizada");
  vTaskDelete(NULL);
}

Navigate_t map(void) {
  Navigate_t teste;
  xQueueReceive(qinput, &teste, portMAX_DELAY);
  return teste;
}

esp_err_t display(menu_path_t *current_path) {

  ESP_LOGI(TAG, "title: %s, index_select: %d",
           current_path->current_menu->label, current_path->current_index);
  // for (int i = 0; i < 3; i++) {
  ESP_LOGI(
      TAG, "Option Selected %s",
      current_path->current_menu->submenus[current_path->current_index].label);
  // }
  // hd44780_gotoxy(&lcd, 0, 0);
  // hd44780_puts(&lcd, path.current_menu->label);
  // hd44780_gotoxy(&lcd, 1, 0);
  // hd44780_puts(&lcd, path.current_menu[0].label);
  // hd44780_gotoxy(&lcd, 2, 0);
  // hd44780_puts(&lcd, path.current_menu[path.current_index].label);
  // hd44780_gotoxy(&lcd, 3, 0);
  // hd44780_puts(&lcd, path.current_menu[2].label);

  return ESP_OK;
}

void app_main(void) {
  qinput = xQueueCreate(5, sizeof(Navigate_t));
  menu_config_t config = {
      .root = root,
      .input = &map,
      .display = &display,
  };

  start_lcd();

  xTaskCreatePinnedToCore(&menu_init, "menu_init", 2048, &config, 3, NULL, 0);
  vTaskDelay(5000 / portMAX_DELAY);
  xTaskCreatePinnedToCore(&simula_input, "simula", 2048, NULL, 1, NULL, 0);
  // vTaskStartScheduler();
  vTaskDelete(NULL);
}
