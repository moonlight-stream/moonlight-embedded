#pragma once

#include "sui-element.h"

class SUIGridContainer : public SUIElement {
public:
    SUIGridContainer();
    ~SUIGridContainer();

    void addChildCell(SUIElement *element, int row, int column);
    void layout();

private:
    std::map<SUIElement *, std::pair<int, int>> grid_;
};
