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

enum class SUIToolbarAction {
  A, B
};

typedef std::tuple<std::string, SUIToolbarAction> SUIToolbarActionItem;

class SUI;
class SUIElement;

class SUIGraphics {
public:
    SUIToolbarActionItem makeToolbarActionItem(std::string text, SUIToolbarAction action);

    void drawBottomToolbar(std::vector<SUIToolbarActionItem> items);
    void drawTopHeader(std::string text);

    void drawTexture(SDL_Texture *texture, int x, int y, int w, int h);
    void drawTexture(SDL_Texture *texture, const SUIRect &bounds);
    void drawTextureClipped(SDL_Texture *texture, int x, int y, int w, int h, const SUIRect &clip);
    void drawTextureClipped(SDL_Texture *texture, const SUIRect &bounds, const SUIRect &clip);

    void drawBox(int x, int y, int width, int height, uint32_t color);
    void drawBox(const SUIRect &bounds, uint32_t color);
    void drawBoxClipped(int x, int y, int width, int height, const SUIRect &clip, uint32_t color);
    void drawBoxClipped(const SUIRect &bounds, const SUIRect &clip, uint32_t color);
    
    void drawRectangle(int x, int y, int width, int height, uint32_t color);
    void drawRectangle(const SUIRect &bounds, uint32_t color);
    void drawRectangleClipped(int x, int y, int width, int height, const SUIRect &clip, uint32_t color);
    void drawRectangleClipped(const SUIRect &bounds, const SUIRect &clip, uint32_t color);

    int measureTextAscent(TTF_Font *font);
    void measureText(TTF_Font *font, std::string text, int *width, int *height);
    void drawText(TTF_Font *font, std::string text, int x, int y, uint32_t color, bool align_center, int truncate_width);
    void drawText(TTF_Font *font, std::string text, const SUIRect &bounds, uint32_t color, bool align_center);
    void drawTextClipped(TTF_Font *font, std::string text, int x, int y, const SUIRect &clip, uint32_t color, bool align_center, int truncate_width);
    void drawTextClipped(TTF_Font *font, std::string text, const SUIRect &bounds, const SUIRect &clip, uint32_t color, bool align_center);

    void drawLine(int x1, int y1, int x2, int y2, uint32_t color);
    void drawLineClipped(int x1, int y1, int x2, int y2, const SUIRect &clip, uint32_t color);

    SUIRect clipBounds(int x, int y, int width, int height, const SUIRect &clip);
    SUIRect clipBounds(const SUIRect &bounds, const SUIRect &clip);
    SUIRect offsetGlobal(const SUIRect &bounds);
    uint32_t interpolate(uint32_t a, uint32_t b, double t);

    SUIElement *element();

private:
    friend class SUIElement;
    
    SUI *ui();

    SUIGraphics(SUIElement *element);
    SUIElement *element_;
};
