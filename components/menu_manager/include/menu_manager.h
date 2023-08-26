#ifndef __MENU_MANAGER_H__
#define __MENU_MANAGER_H__

#include "sdkconfig.h"
#include <esp_err.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <stdint.h>

typedef enum {
  NAVIGATE_UP,
  NAVIGATE_DOWN,
  NAVIGATE_SELECT,
  NAVIGATE_BACK,
} Navigate_t;

typedef struct menu_node {
  const char label[CONFIG_HORIZONTAL_SIZE];
  struct menu_node *submenus;
  size_t num_options;
  void (*function)(void *args);
} menu_node_t;

typedef struct {
  menu_node_t *current_menu;
  uint8_t current_index;
} menu_path_t;

typedef struct {
  menu_node_t root;
  Navigate_t (*input)(void);
  esp_err_t (*display)(menu_path_t *current_path);
} menu_config_t;

void menu_init(void *args);

#endif //__MENU_MANAGER_H__
