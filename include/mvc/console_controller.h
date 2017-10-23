#pragma once

#include "model.h"
#include "console_views.h"

class ConsoleController {
public:
    ConsoleController(Model* model_) : model(model_) {}

    ConsoleController(const ConsoleController&) = delete;
    ConsoleController& operator = (const ConsoleController&) = delete;

    void run(int argc, char* argv[]);

private:
    Model* model;
    std::unique_ptr<ConsoleViews::TextView> textView = nullptr;
    std::unique_ptr<ConsoleViews::ImageView> imageView = nullptr;
};
