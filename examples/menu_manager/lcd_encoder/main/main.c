// TODO: Organizar codigo definir tud no inicio depois main_app e depois todas
// as funçõe

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
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

static const char *TAG = "main";

QueueHandle_t qinput;
static rotary_encoder_t re;

bool blacklight = true;
static i2c_dev_t pcf8574;

uint8_t first = 0, end = 0;
const char *old_title;
void *displays[2];

menu_config_t config;

static const uint8_t char_data[] = { // Define new chars
                                     // Right Arrow
    0x00, 0x04, 0x06, 0x1F, 0x1F, 0x06, 0x04, 0x00};

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

void input_encoder(void *args) {

  ESP_ERROR_CHECK(rotary_encoder_init(qinput));

  re.pin_a = CONFIG_ENCODER_CLK;
  re.pin_b = CONFIG_ENCODER_DT;
  re.pin_btn = CONFIG_ENCODER_SW;

  ESP_ERROR_CHECK(rotary_encoder_add(&re));
  vTaskDelete(NULL);
}

void dumb(void *args) {

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

  hd44780_clear(&lcd);
  hd44780_puts(&lcd, "FINISH:");
  hd44780_gotoxy(&lcd, 0, 1);
  hd44780_puts(&lcd, "dumb function");
  ESP_LOGI(TAG, "FINISH I am dumb");

  END_MENU_FUNCTION;
}

void switch_blacklight(void *args) {
  SET_QUICK_FUNCTION;

  blacklight = !blacklight;
  hd44780_switch_backlight(&lcd, blacklight);

  END_MENU_FUNCTION;
}

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

void display_loop(menu_path_t *current_path) {
  const char *title = current_path->current_menu->label;

  uint8_t select = current_path->current_index;
  uint8_t prev = (current_path->current_menu->num_options + select - 1) %
                 current_path->current_menu->num_options;
  uint8_t next = (select + 1) % current_path->current_menu->num_options;

  const char *prev_label = current_path->current_menu->submenus[prev].label;
  const char *select_label = current_path->current_menu->submenus[select].label;
  const char *next_label = current_path->current_menu->submenus[next].label;

  uint8_t central_title = (CONFIG_HORIZONTAL_SIZE - strlen(title)) / 2;
  hd44780_clear(&lcd);
  hd44780_gotoxy(&lcd, central_title, 0);
  hd44780_puts(&lcd, title);
  hd44780_gotoxy(&lcd, 0, 1);
  hd44780_puts(&lcd, prev_label);
  hd44780_gotoxy(&lcd, 0, 2);
  hd44780_putc(&lcd, 0);
  hd44780_putc(&lcd, ' ');
  hd44780_puts(&lcd, select_label);
  hd44780_gotoxy(&lcd, 0, 3);
  hd44780_puts(&lcd, next_label);
}

void display(menu_path_t *current_path) {
  uint8_t select = current_path->current_index;
  uint8_t count = 1;
  const char *title = current_path->current_menu->label;

  hd44780_clear(&lcd);
  hd44780_gotoxy(&lcd, (CONFIG_HORIZONTAL_SIZE - strlen(title)) / 2, 0);
  hd44780_puts(&lcd, title);
  if (select < first || select == 0) {
    first = select;
    end = first + CONFIG_VERTICAL_SIZE - 1;
    old_title = title;
  } else if (select + 1 > end || old_title != title) {
    end = select + 1;
    first = end - CONFIG_VERTICAL_SIZE + 1;
    old_title = title;
  }

  for (uint8_t _ = first; _ < end; _++) {
    hd44780_gotoxy(&lcd, 0, count);
    count++;
    if (_ == select) {
      hd44780_putc(&lcd, 0);
      hd44780_putc(&lcd, ' ');
    }
    hd44780_puts(&lcd, current_path->current_menu->submenus[_].label);
  }
}

uint8_t option_type_menu = 0;
void switch_menu(void *args) {
  SET_QUICK_FUNCTION;
  option_type_menu ^= 1;
  config.display = displays[option_type_menu];
  END_MENU_FUNCTION;
}

menu_node_t submenu[3] = {
    {.label = "funcA", .function = &dumb},
    {.label = "funcB", .function = &dumb},
    {.label = "funcC", .function = &dumb},
};

menu_node_t root = {
    .label = "root",
    .num_options = 5,
    .submenus = (menu_node_t[5]){
        {.label = "submenu1", .submenus = submenu, .num_options = 3},
        {.label = "submenu2", .submenus = submenu, .num_options = 3},
        {.label = "submenu3", .submenus = submenu, .num_options = 3},
        {.label = "blacklight", .function = &switch_blacklight},
        {.label = "Type Menu", .function = &switch_menu}}};

void app_main(void) {
  qinput = xQueueCreate(5, sizeof(rotary_encoder_event_t));

  ESP_ERROR_CHECK(i2cdev_init());
  ESP_ERROR_CHECK(pcf8574_init_desc(&pcf8574, CONFIG_DISPLAY_ADDR, 0,
                                    CONFIG_I2C_SDA, CONFIG_I2C_SCL));

  hd44780_switch_backlight(&lcd, blacklight);
  ESP_ERROR_CHECK(hd44780_init(&lcd));
  hd44780_upload_character(&lcd, 0, char_data);
  ESP_LOGI(TAG, "LCD ON!");

  config.root = root;
  config.input = &map;
  config.display = &display;

  displays[0] = &display;
  displays[1] = &display_loop;

  xTaskCreatePinnedToCore(&input_encoder, "encoder", 2048, NULL, 5, NULL, 0);
  xTaskCreatePinnedToCore(&menu_init, "menu_init", 2048, &config, 1, NULL, 0);
  vTaskDelete(NULL);
}
