#include <stdbool.h>
#include <vita2d.h>

#define WIDTH 960
#define HEIGHT 544

struct menu_entry {
  char *name;
  char *description;
  bool disabled, separator;
  short id;
};

struct menu_geom {
  int x, y, width, height, el, total_y;
};

vita2d_pgf *gui_font;

struct menu_geom make_geom_centered(int w, int h);

bool was_button_pressed(short id);
bool is_button_down(short id);

void gui_ctrl_begin();
void gui_ctrl_end();

void gui_ctrl_cursor(int *cursor_ptr, int total_elements);
void gui_ctrl_offset(int *offset_ptr, struct menu_geom geom, int cursor);

void display_menu(
        struct menu_entry menu[],
        int total_elements,
        int (*cb)(int, void *),
        void *context);

void display_alert(char *message, char *button_captions[], int buttons_count, int (*cb)(int));
void display_error(char *format, ...);
void flash_message(char *format, ...);

void guilib_init();
