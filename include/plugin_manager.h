#pragma once

#include <vector>
#include <memory>
#include <dlfcn.h>

#include "io.h"

class IFilterPlugin {
public:
    virtual ~IFilterPlugin() = default;

    virtual std::string getName() const = 0;

    virtual std::string getUserInvitation() const {
        return "";
    }

    virtual void sendUserOutput(const std::string& /*initStr*/) {

    }

    virtual Image applyToImage(const Image& image) const = 0;
};

template <typename Plugin>
class IPluginFactory {
public:
    virtual Plugin* createObject() = 0;
};

using IFilterPluginFactory = IPluginFactory<IFilterPlugin>;

template <typename Plugin>
class PluginManager {
using PluginPointer = std::unique_ptr<Plugin>;

public:
    virtual ~PluginManager() {
        plugins.clear();
        for (auto h : handles)
            dlclose(h);
        handles.clear();
    }

    void registerPlugin(IPluginFactory<Plugin>& factory) {
        auto plugin = factory.createObject();
        if (plugin)
            plugins.push_back(PluginPointer(plugin));
    }

    const std::vector<PluginPointer>& getPlugins() const {
        return plugins;
    }

    virtual void registerPlugins(const std::vector<std::string>& pluginFiles) = 0;

protected:
    std::vector<void*> handles{};

private:
    std::vector<PluginPointer> plugins{};
};

class FilterPluginManager : public PluginManager<IFilterPlugin> {
public:
    void registerPlugins(const std::vector<std::string>& pluginFiles) override {
        using RegisterFunctionType = void (*)(FilterPluginManager&);
        for (const auto& pluginName : pluginFiles) {
            auto plugin = dlopen(pluginName.c_str(), RTLD_LAZY);
            if (plugin == nullptr)
                continue;
            auto registerFunc = RegisterFunctionType(dlsym(plugin, "registerPlugin"));
            if (registerFunc == nullptr)
                continue;
            registerFunc(*this);
            handles.push_back(plugin);
        }
    }
};
