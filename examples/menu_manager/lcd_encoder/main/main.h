#include <esp_err.h>
#include <hd44780.h>
#include <menu_manager.h>

esp_err_t start_lcd(void);

void display(menu_path_t *current_path);

void display_loop(menu_path_t *current_path);

Navigate_t map(void);

esp_err_t start_encoder(void);

// functions

void switch_blacklight(void *args);

typedef struct {
  void (*type_menu)(menu_path_t *current_path);
  bool loop_menu;
  char *label;
} switch_menu_t;

void switch_menu(void *args);

void dumb(void *args);
