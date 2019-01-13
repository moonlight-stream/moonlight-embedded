#include "sui-grid-container.h"
#include "sui-graphics.h"
#include "sui.h"

#include <limits>

SUIGridContainer::SUIGridContainer(std::string name)
    : SUIContainer(name),
      grid_(),
      grid_size_(std::make_pair(0, 0)),
      grid_first_position_(std::make_pair(std::numeric_limits<int>::max(), std::numeric_limits<int>::max())),
      grid_last_position_(std::make_pair(-1, -1)),
      minimum_row_spacing_(0),
      minimum_column_spacing_(0)
{

}

SUIGridContainer::~SUIGridContainer() {

}

void SUIGridContainer::addChildCell(SUIElement *element, int row, int col) {
    addChild(element);

    if (row + 1 > grid_size_.first || col + 1 > grid_size_.second) {
        resizeGrid(std::max(row + 1, grid_size_.first), std::max(col + 1, grid_size_.second));
    }

    grid_[row][col] = element;

    // If this is the furthest right element in the last row, save it
    if (row > grid_last_position_.first) {
        grid_last_position_.first = row;
        grid_last_position_.second = col;
    }
    else if (row + 1 == grid_size_.first && col >= grid_last_position_.second) {
        grid_last_position_.first = row;
        grid_last_position_.second = col;
    }
    
    // If this is the furthest left element in the first row, save it
    if (row == 0 && col <= grid_first_position_.second) {
        grid_first_position_.first = row;
        grid_first_position_.second = col;
    }
}

void SUIGridContainer::layout() {
    std::vector<int> row_heights(grid_size_.first);
    std::vector<int> col_widths(grid_size_.second);

    // Compute the heights and widths of each cell
    for (int row = 0; row < grid_size_.first; row++) {
        for (int col = 0; col < grid_size_.second; col++) {
            SUIElement *cell = grid_[row][col];

            if (cell) {
                row_heights[row] = std::max(row_heights[row], cell->bounds().h);
                col_widths[col] = std::max(col_widths[col], cell->bounds().w);
            }
        }
    }
    
    // Compute spacing needed between elements
    int row_total_height = std::accumulate(row_heights.begin(), row_heights.end(), 0);
    int col_total_width = std::accumulate(col_widths.begin(), col_widths.end(), 0);

    float row_spacing = minimum_row_spacing_;
    float col_spacing = minimum_column_spacing_;

    if (row_total_height < bounds().h)
        row_spacing = (float)(bounds().h - row_total_height) / (row_heights.size() - 1);

    if (col_total_width < bounds().w)
        col_spacing = (float)(bounds().w - col_total_width) / (col_widths.size() - 1);

    // Layout!
    int offset_y = 0;
    for (int row = 0; row < grid_size_.first; row++) {
        int offset_x = 0;
        for (int col = 0; col < grid_size_.second; col++) {
            SUIElement *child = grid_[row][col];
            child->bounds().x = static_cast<int>(round(offset_x));
            child->bounds().y = static_cast<int>(round(offset_y));

            offset_x += col_widths[col];
            offset_x += col_spacing;
        }

        offset_y += row_heights[row];
        offset_y += row_spacing;
    }
}

SUIFocusResult SUIGridContainer::updateFocus(SUIInput *input, SUIElement *previous) {
    last_focus_ = previous;

    SUIElement *focus_current = nullptr;
    int focus_row, focus_col;

    // Find the focused element
    for (int row = 0; row < grid_size_.first; row++) {
        for (int col = 0; col < grid_size_.second; col++) {
            SUIElement *cell = grid_[row][col];

            if (cell == previous) {
                focus_current = cell;
                focus_row = row;
                focus_col = col;
                break;
            }
        }
    }

    if (focus_current == nullptr) {
        // printf("\tCould not find element '%s' in grid '%s'\n", previous ? previous->name().c_str() : "<null>", name().c_str());
    }
    else {
        // printf("\tFound element '%s' in grid '%s'\n", previous->name().c_str(), name().c_str());
    }

    if ((focus_row == 0 && input->buttons.down & KEY_UP) ||
        (focus_row == grid_size_.first - 1 && input->buttons.down & KEY_DOWN) ||
        (focus_col == 0 && input->buttons.down & KEY_LEFT) ||
        (focus_col == grid_size_.second - 1 && input->buttons.down & KEY_RIGHT))
    {
        return SUIFocusRelease;
    }
    
    if (input->buttons.down & KEY_UP) {
        SUIElement *next_focus;

        for (int i = focus_row - 1; i >= 0; i--) {
            SUIElement *next_cell = grid_[i][focus_col];

            if (next_cell) {
                next_focus = next_cell->acceptFocus();

                if (next_focus) {
                    break;
                }
            }
        }

        if (next_focus) {
            stage()->setFocusedElement(next_focus);
        }
        else {
            return SUIFocusRelease;
        }
    }
    
    if (input->buttons.down & KEY_LEFT) {
        SUIElement *next_focus;

        for (int i = focus_col - 1; i >= 0; i--) {
            SUIElement *next_cell = grid_[focus_row][i];

            if (next_cell) {
                next_focus = next_cell->acceptFocus();

                if (next_focus) {
                    break;
                }
            }
        }

        if (next_focus) {
            stage()->setFocusedElement(next_focus);
        }
        else {
            return SUIFocusRelease;
        }
    }
    
    if (input->buttons.down & KEY_DOWN) {
        SUIElement *next_focus;

        for (int i = focus_row + 1; i < grid_size_.first; i++) {
            SUIElement *next_cell = grid_[i][focus_col];

            if (next_cell) {
                next_focus = next_cell->acceptFocus();

                if (next_focus) {
                    break;
                }
            }
        }

        if (next_focus) {
            stage()->setFocusedElement(next_focus);
        }
        else {
            return SUIFocusRelease;
        }
    }
    
    if (input->buttons.down & KEY_RIGHT) {
        SUIElement *next_focus;

        for (int i = focus_col + 1; i <= grid_size_.second; i++) {
            SUIElement *next_cell = grid_[focus_row][i];

            if (next_cell) {
                next_focus = next_cell->acceptFocus();

                if (next_focus) {
                    break;
                }
            }
        }

        if (next_focus) {
            stage()->setFocusedElement(next_focus);
        }
        else {
            return SUIFocusRelease;
        }
    }

    return SUIFocusRetain;
}

int SUIGridContainer::minimumRowSpacing() { return minimum_row_spacing_; }
void SUIGridContainer::setMinimumRowSpacing(int val) { minimum_row_spacing_ = val; }

int SUIGridContainer::minimumColumnSpacing() { return minimum_column_spacing_; }
void SUIGridContainer::setMinimumColumnSpacing(int val) { minimum_column_spacing_ = val; }

void SUIGridContainer::resizeGrid(int rows, int columns) {
    grid_.resize(rows);
    for (int r = 0; r < rows; r++) {
        grid_[r].resize(columns, nullptr);
    }

    grid_size_.first = rows;
    grid_size_.second = columns;
}