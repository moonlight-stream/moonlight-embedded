#include <stdbool.h>
#include <vita2d.h>

#include <psp2/touch.h>

#define WIDTH 960
#define HEIGHT 544

typedef struct menu_entry {
  int id;
  unsigned int color;
  char *name, *suffix;
  char subname[1024];
  bool disabled, separator;
} menu_entry;

typedef struct menu_geom {
  int x, y, width, height, el, total_y;
} menu_geom;

vita2d_font *font;

struct menu_geom make_geom_centered(int w, int h);

#define SCE_CTRL_HOLD 0x80000000

typedef struct input_data {
  int buttons;
  SceTouchData touch;
} input_data;

typedef int (*gui_loop_callback) (int, void *, const input_data *);
typedef int (*gui_back_callback) (void *);
typedef void (*gui_draw_callback) (void);

bool was_button_pressed(short id);
bool is_button_down(short id);
bool is_rectangle_touched(const SceTouchData *touch, int lx, int ly, int rx, int ry);

int display_menu(
        menu_entry menu[],
        int total_elements,
        menu_geom *geom_ptr,
        gui_loop_callback loop_callback,
        gui_back_callback back_callback,
        gui_draw_callback draw_callback,
        void *context
        );

void display_alert(
        char *message,
        char *button_captions[],
        int buttons_count,
        gui_loop_callback cb,
        void *context
        );

void display_error(char *format, ...);

void flash_message(char *format, ...);

void guilib_init(gui_loop_callback global_loop_cb, gui_draw_callback global_draw_cb);
