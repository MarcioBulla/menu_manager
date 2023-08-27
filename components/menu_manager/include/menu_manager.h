/**
 * @file menu_manager.h
 * @brief This file defines structures and functions related to the Menu System.
 */

#ifndef __MENU_MANAGER_H__
#define __MENU_MANAGER_H__

#include "sdkconfig.h"
#include <esp_err.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <stdint.h>

/**
 * Type with  possibles of action in menu system.
 *
 */
typedef enum {
  NAVIGATE_UP,
  /**< Action up (index++). */
  NAVIGATE_DOWN,
  /**< Action DOWN (index--). */
  NAVIGATE_SELECT,
  /**< Action SELECT (depth++). */
  NAVIGATE_BACK,
  /**< Action BACH (depth--). */
} Navigate_t;

/**
 * Menu System is based in nodes all menus are nodes. Submenus are nodes with
 * array of more submenus and function (leafs) are nodes with function.
 *
 */
typedef struct menu_node {
  const char label[CONFIG_HORIZONTAL_SIZE];
  /**< Title of node with CONFIG_HORIZONTAL_SIZE max number characters. */
  struct menu_node *submenus;
  /**< Point of array submenus or one submenu. */
  size_t num_options;
  /**< number of option on submenu. */
  void (*function)(void *args);
  /**< function that defined this node as leaf. */
} menu_node_t;

/**
 * Struct for save location and index.
 *
 */
typedef struct {
  menu_node_t *current_menu;
  /**< Point if current menu. */
  uint8_t current_index;
  /**< current Index of selected option. */
} menu_path_t;

/**
 * This struct is all essential args that menu_init will need.
 */
typedef struct {
  menu_node_t root;
  /**< Menu_node_t: origin of Menu System. */
  Navigate_t (*input)(void);
  /**< Function that have to return Navigate_t to navigate. */
  esp_err_t (*display)(menu_path_t *current_path);
  /**< Function that receive menu_path_t and index for display current
   * selection. */
} menu_config_t;

/**
 * Start menu system that receive generic input function and generic
 * display function.
 *
 * @param params The struct that there are args.
 */
void menu_init(void *params);

#endif //__MENU_MANAGER_H__
