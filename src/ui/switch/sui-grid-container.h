#pragma once

#include "sui-element.h"

class SUIGridContainer : public SUIElement {
public:
    SUIGridContainer();
    ~SUIGridContainer();

    int minimumRowSpacing();
    void setMinimumRowSpacing(int val);

    int minimumColumnSpacing();
    void setMinimumColumnSpacing(int val);

    void addChildCell(SUIElement *element, int row, int column);
    void layout();

private:
    std::map<SUIElement *, std::pair<int, int>> grid_;
    int minimum_row_spacing_;
    int minimum_column_spacing_;
};
