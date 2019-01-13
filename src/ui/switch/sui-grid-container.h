#pragma once

#include "sui-container.h"

class SUIGridContainer : public SUIContainer {
public:
    SUIGridContainer(std::string name);
    ~SUIGridContainer();

    int minimumRowSpacing();
    void setMinimumRowSpacing(int val);

    int minimumColumnSpacing();
    void setMinimumColumnSpacing(int val);

    void addChildCell(SUIElement *element, int row, int column);
    void layout();

    SUIFocusResult updateFocus(SUIInput *input, SUIElement *previous = nullptr) override;

private:
    void resizeGrid(int rows, int columns);

    std::vector<std::vector<SUIElement *>> grid_;
    std::pair<int, int> grid_size_;

    std::pair<int, int> grid_first_position_;
    std::pair<int, int> grid_last_position_;

    int minimum_row_spacing_;
    int minimum_column_spacing_;
};
