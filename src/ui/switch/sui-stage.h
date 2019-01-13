#pragma once

#include "sui-container.h"

class SUIStage : public SUIContainer {
public:
    SUIStage(SUI *ui, std::string name);
    ~SUIStage();

    void update(SUIInput *input) override;
    void render() override;
};