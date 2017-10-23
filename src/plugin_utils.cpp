#include "plugin_utils.h"

#include <dirent.h>
#include <vector>
#include <string>
#include <dlfcn.h>

#include <iostream>

static std::vector<std::string> findPluginCandidates() {
    std::string base = "plugins";

    DIR* pluginDir = opendir(base.c_str());
    if (pluginDir == nullptr) {
        return {};
    }

    dirent* entry;
    std::vector<std::string> pluginFiles;

    while ((entry = readdir(pluginDir)) != nullptr) {
        if (entry->d_type == DT_REG)
            pluginFiles.push_back(base + "/" + entry->d_name);
    }

    closedir(pluginDir);
    return pluginFiles;
}

void registerPlugins(FilterPluginManager& manager) {
    auto pluginCandidates = findPluginCandidates();
    manager.registerPlugins(pluginCandidates);
}
