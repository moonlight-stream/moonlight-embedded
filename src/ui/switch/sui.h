#pragma once

#include <switch.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <SDL2/SDL2_rotozoom.h>

#include <initializer_list>
#include <tuple>
#include <string>

#define SUI_MARGIN_SIDE 30
#define SUI_MARGIN_TOP 88
#define SUI_MARGIN_BOTTOM 73

#define SUI_MARGIN_TOOLBAR_SIDE 30
#define SUI_MARGIN_BETWEEN_TOOLBAR_ICON_TEXT  10
#define SUI_MARGIN_BETWEEN_TOOLBAR_BUTTONS    44

#define SUI_COLOR_BACKGROUND  0xffebebeb
#define SUI_COLOR_DARK        0xff2d2d2d
#define SUI_COLOR_LIGHT       0xff6d6d6d

#define SUI_TRUNCATE_FADE_WIDTH 15

typedef SDL_Rect SUIRect;

typedef struct
{
    struct {
        uint64_t down;
        uint64_t held;
        uint64_t up;
    } buttons;

    struct {
        JoystickPosition left;
        JoystickPosition right;
    } joysticks;

    struct {
        bool touched;
        touchPosition position;
    } touch;
} SUIInput;

enum SUIToolbarAction {
  SUIToolbarActionA, SUIToolbarActionB
};

class SUI {
public:
    SUI();
    ~SUI();

    SDL_Texture *loadPNG(const void *data, size_t size);
    SDL_Texture *loadPNGScaled(const void *data, size_t size, int width, int height);

    SUIInput *inputPoll(HidControllerID id);
    bool inputTest(SUIInput *input);

    void drawBottomToolbar(std::initializer_list<std::tuple<std::string, SUIToolbarAction>> items);
    void drawTopHeader(char *text);

    void drawTexture(SDL_Texture *texture, int x, int y, int w, int h);
    void drawTexture(SDL_Texture *texture, SUIRect *bounds);
    void drawTextureClipped(SDL_Texture *texture, int x, int y, int w, int h, SUIRect *clip);
    void drawTextureClipped(SDL_Texture *texture, SUIRect *bounds, SUIRect *clip);

    void drawBox(int x, int y, int width, int height, uint32_t color);
    void drawBox(SUIRect *bounds, uint32_t color);
    void drawBoxClipped(int x, int y, int width, int height, SUIRect *clip, uint32_t color);
    void drawBoxClipped(SUIRect *bounds, SUIRect *clip, uint32_t color);
    
    void drawRectangle(int x, int y, int width, int height, uint32_t color);
    void drawRectangle(SUIRect *bounds, uint32_t color);
    void drawRectangleClipped(int x, int y, int width, int height, SUIRect *clip, uint32_t color);
    void drawRectangleClipped(SUIRect *bounds, SUIRect *clip, uint32_t color);

    int measureTextAscent(TTF_Font *font);
    void measureText(TTF_Font *font, const char *text, int *width, int *height);
    void measureText(TTF_Font *font, std::string text, int *width, int *height);
    void drawText(TTF_Font *font, const char *text, int x, int y, uint32_t color, bool align_center, int truncate_width);
    void drawText(TTF_Font *font, std::string text, int x, int y, uint32_t color, bool align_center, int truncate_width);
    void drawTextClipped(TTF_Font *font, const char *text, int x, int y, SUIRect *clip, uint32_t color, bool align_center, int truncate_width);
    void drawTextClipped(TTF_Font *font, std::string text, int x, int y, SUIRect *clip, uint32_t color, bool align_center, int truncate_width);

    SUIRect clipBounds(int x, int y, int width, int height, SUIRect *clip);
    SUIRect clipBounds(SUIRect *bounds, SUIRect *clip);
    uint32_t interpolate(uint32_t a, uint32_t b, double t);

    SDL_Window *window;
    SDL_Renderer *renderer;
    int width;
    int height;
    SUIRect clip;
    SUIInput input;

    PlFontData font_data;
    TTF_Font *font_small;
    TTF_Font *font_normal;
    TTF_Font *font_heading;
    TTF_Font *font_massive;

    SDL_Texture *button_a_texture;
    SDL_Texture *button_b_texture;
    int button_a_width;
    int button_a_height;
    int button_b_width;
    int button_b_height;
};
