#pragma once

#include "sui-common.h"

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

enum SUIToolbarAction {
  SUIToolbarActionA, SUIToolbarActionB
};

typedef std::tuple<std::string, SUIToolbarAction> SUIToolbarActionItem;

class SUI;
class SUIElement;

class SUIGraphics {
public:
    SUIToolbarActionItem makeToolbarActionItem(const char *text, SUIToolbarAction action);
    SUIToolbarActionItem makeToolbarActionItem(std::string text, SUIToolbarAction action);

    void drawBottomToolbar(std::vector<SUIToolbarActionItem> items);
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

private:
    friend class SUIElement;
    
    SUI *ui();

    SUIGraphics(SUIElement *element);
    SUIElement *element_;
};
