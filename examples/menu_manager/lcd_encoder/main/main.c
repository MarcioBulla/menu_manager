#include "esp_err.h"
#include "freertos/portmacro.h"
#include "i2cdev.h"
#include <encoder.h>
#include <esp_idf_lib_helpers.h>
#include <esp_log.h>
#include <esp_system.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/semphr.h>
#include <freertos/task.h>
#include <hd44780.h>
#include <menu_manager.h>
#include <pcf8574.h>
#include <sdkconfig.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

static const char *TAG = "main";

QueueHandle_t qinput;
static rotary_encoder_t re;

static i2c_dev_t pcf8574;

void input_encoder(void *args) {

  ESP_ERROR_CHECK(rotary_encoder_init(qinput));

  re.pin_a = 16;
  re.pin_b = 17;
  re.pin_btn = 5;

  ESP_ERROR_CHECK(rotary_encoder_add(&re));
  vTaskDelete(NULL);
}

/**
 * Callback to comunication to LCD from pcf8574
 *
 * @param Plcd display that connected pcf8574
 * @param command that dispay xQueueReceive
 * @return Action that esp32 will happend
 */
static esp_err_t write_lcd_data(const hd44780_t *lcd, uint8_t data) {
  return pcf8574_port_write(&pcf8574, data);
}

hd44780_t lcd = {.write_cb = write_lcd_data,
                 .font = HD44780_FONT_5X8,
                 .lines = CONFIG_VERTICAL_SIZE,
                 .pins = {
                     .rs = 0,
                     .e = 2,
                     .d4 = 4,
                     .d5 = 5,
                     .d6 = 6,
                     .d7 = 7,
                     .bl = 3,
                 }};

void dumb(void *args) {
  ESP_LOGI(TAG, "START I am dumb");

  hd44780_clear(&lcd);

  hd44780_gotoxy(&lcd, 0, 0);
  hd44780_puts(&lcd, "I");
  hd44780_gotoxy(&lcd, 0, 1);
  hd44780_puts(&lcd, "am");
  hd44780_gotoxy(&lcd, 0, 2);
  hd44780_puts(&lcd, "dumb   or");
  hd44780_gotoxy(&lcd, 0, 3);
  hd44780_puts(&lcd, "dummy");

  vTaskDelay(5000 / portTICK_PERIOD_MS);
  ESP_LOGI(TAG, "FINISH I am dumb");

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

Navigate_t map(void) {
  rotary_encoder_event_t e;
  xQueueReceive(qinput, &e, portMAX_DELAY);

  switch (e.type) {

  case RE_ET_BTN_CLICKED:
    ESP_LOGI(TAG, "SELECT");
    return NAVIGATE_SELECT;

  case RE_ET_BTN_LONG_PRESSED:
    ESP_LOGI(TAG, "BACK");
    return NAVIGATE_BACK;

  case RE_ET_CHANGED:
    if (e.diff > 0) {
      ESP_LOGI(TAG, "UP");
      return NAVIGATE_UP;
    } else {
      ESP_LOGI(TAG, "DOWN");
      return NAVIGATE_DOWN;
    }
  default:
    ESP_LOGI(TAG, "NOTHIN");
    return map();
  }
}

void display(menu_path_t *current_path) {
  char *title = (char *)current_path->current_menu->label;

  uint8_t select = current_path->current_index;
  uint8_t prev = (current_path->current_menu->num_options + select - 1) %
                 current_path->current_menu->num_options;
  uint8_t next = (select + 1) % current_path->current_menu->num_options;

  printf("prev: %d\nselect: %d\nnext: %d\n", prev, select, next);

  hd44780_clear(&lcd);

  hd44780_gotoxy(&lcd, 0, 0);
  hd44780_puts(&lcd, title);
  hd44780_gotoxy(&lcd, 0, 1);
  hd44780_puts(&lcd, current_path->current_menu->submenus[prev].label);
  hd44780_gotoxy(&lcd, 0, 2);
  hd44780_puts(&lcd, current_path->current_menu->submenus[select].label);
  hd44780_gotoxy(&lcd, 0, 3);
  hd44780_puts(&lcd, current_path->current_menu->submenus[next].label);
}

void app_main(void) {
  qinput = xQueueCreate(5, sizeof(rotary_encoder_event_t));
  ESP_ERROR_CHECK(i2cdev_init());
  ESP_ERROR_CHECK(pcf8574_init_desc(&pcf8574, 0x27, 0, 21, 22));
  ESP_ERROR_CHECK(hd44780_init(&lcd));
  hd44780_switch_backlight(&lcd, true);
  ESP_LOGI(TAG, "LCD ON!");

  menu_config_t config = {
      .root = root,
      .input = &map,
      .display = &display,
  };

  xTaskCreatePinnedToCore(&input_encoder, "encoder", 2048, NULL, 5, NULL, 0);
  xTaskCreatePinnedToCore(&menu_init, "menu_init", 2048, &config, 1, NULL, 0);
  vTaskDelete(NULL);
}
