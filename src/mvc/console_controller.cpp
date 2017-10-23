#include "mvc/console_controller.h"
#include "mvc/console_views.h"
#include "io.h"
#include "plugin_manager.h"
#include "plugin_utils.h"

#include <numeric>
#include <string>
#include <iostream>

static void checkArgcCount(int argc, int from, int to = std::numeric_limits<int>::max()) {
    if (argc < from)
        throw std::string("too few arguments for operation");
    if (argc > to)
        throw std::string("too many arguments for operation");
}

static void printHelp(const char* argv0) {
    std::cout << "Usage: " << argv0 << " <input_image_path> <output_image_path> <logfile_path> [--filter]" << std::endl;
}

void ConsoleController::run(int argc, char* argv[]) {
    checkArgcCount(argc, 2);
    if (std::string(argv[1]) == "--help") {
        printHelp(argv[0]);
        return;
    }

    checkArgcCount(argc, 4, 5);

    bool isFilter = false;

    if (argc == 5) {
        if (std::string(argv[4]) == "--filter")
            isFilter = true;
        else
            throw std::string("unknown option ") + std::string(argv[4]);
    }

    const char* srcImageName = argv[1];
    const char* dstImageName = argv[2];
    const char* logFileName = argv[3];

    FilterPluginManager manager;
    IFilterPlugin* plugin = nullptr;

    if (isFilter) {
        registerPlugins(manager);
        const auto& plugins = manager.getPlugins();
        if (plugins.empty()) {
            std::cout << "plugins no found" << std::endl;
        } else {
            std::cout << "plugins found:" << std::endl;
            size_t num = 0;
            for (const auto& plug : plugins) {
                std::cout << "[" << num << "]" << ' ' << plug->getName() << std::endl;
                ++num;
            }
            std::cout << "chose plugin:" << std::endl;
            std::cin >> num;
            plugin = plugins[num].get();
            std::string invite = plugin->getUserInvitation();
            while (!invite.empty()) {
                std::cout << invite << std::endl;
                std::string initStr;
                std::cin >> initStr;
                plugin->sendUserOutput(initStr);
                invite = plugin->getUserInvitation();
            }
        }
    }

    textView = std::make_unique<ConsoleViews::TextView>(model, logFileName);
    imageView = std::make_unique<ConsoleViews::ImageView>(model, dstImageName);
    model->addObserver(textView.get());
    model->addObserver(imageView.get());

    model->align(srcImageName, false, false, 0);
    if (plugin)
        model->processResult(*plugin);
}
