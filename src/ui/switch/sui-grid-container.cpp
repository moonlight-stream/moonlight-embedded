#include "sui-grid-container.h"
#include "sui-graphics.h"
#include "sui.h"

SUIGridContainer::SUIGridContainer()
    : SUIElement(),
      grid_(),
      minimum_row_spacing_(0),
      minimum_column_spacing_(0)
{

}

SUIGridContainer::~SUIGridContainer() {

}

void SUIGridContainer::addChildCell(SUIElement *element, int row, int col) {
    addChild(element);
    grid_.emplace(element, std::make_pair(row, col));
}

void SUIGridContainer::layout() {
    std::vector<int> row_heights, col_widths;

    // Compute the heights and widths of each cell
    for (auto grid_item : grid_) {
        SUIElement *child = grid_item.first;
        int child_height = child->bounds().h;
        int child_width = child->bounds().w;

        int row = grid_item.second.first;
        int col = grid_item.second.second;

        if (row_heights.size() < row + 1)
            row_heights.resize(row + 1, 0);
        
        if (col_widths.size() < col + 1) 
            col_widths.resize(col + 1, 0);

        if (child_height > row_heights[row])
            row_heights[row] = child_height;

        if (child_width > col_widths[col])
            col_widths[col] = child_width;
    }
    
    // Compute spacing needed between elements
    float row_spacing = minimum_row_spacing_, col_spacing = minimum_column_spacing_;
    int row_total_height = std::accumulate(row_heights.begin(), row_heights.end(), 0);
    int col_total_width = std::accumulate(col_widths.begin(), col_widths.end(), 0);

    if (row_total_height < bounds().h)
        row_spacing = (float)(bounds().h - row_total_height) / (row_heights.size() - 1);

    if (col_total_width < bounds().w)
        col_spacing = (float)(bounds().w - col_total_width) / (col_widths.size() - 1);

    // Layout!
    for (auto grid_item : grid_) {
        SUIElement *child = grid_item.first;
        int row = grid_item.second.first;
        int col = grid_item.second.second;

        float x = 0, y = 0;

        for (int r = 0; r < row; r++) {
            y += row_heights[r];
            y += row_spacing;
        }

        for (int c = 0; c < col; c++) {
            x += col_widths[c];
            x += col_spacing;
        }

        child->bounds().x = static_cast<int>(round(x));
        child->bounds().y = static_cast<int>(round(y));
    }
}

int SUIGridContainer::minimumRowSpacing() { return minimum_row_spacing_; }
void SUIGridContainer::setMinimumRowSpacing(int val) { minimum_row_spacing_ = val; }

int SUIGridContainer::minimumColumnSpacing() { return minimum_column_spacing_; }
void SUIGridContainer::setMinimumColumnSpacing(int val) { minimum_column_spacing_ = val; }