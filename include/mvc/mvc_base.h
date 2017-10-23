#pragma once

#include "observer.h"

#include <vector>

class ModelBase;
class ViewBase;
class ControllerBase;

class ViewBase : public ObserverBase {
public:
    ViewBase(const ModelBase* model_, ControllerBase* controller_);

    virtual ~ViewBase() = default;

    ViewBase(const ViewBase&) = delete;
    ViewBase& operator=(const ViewBase&) = delete;

protected:
    virtual void applyModelNotification(const NotificationBase& notification) = 0;

    void applyNotification(const NotificationBase& notification) override final;

protected:
    const ModelBase* model;
    ControllerBase* controller;
};

class ModelBase : public ObservableBase {
public:
    ModelBase(const ModelBase&) = delete;
    ModelBase& operator=(const ModelBase&) = delete;

    virtual ~ModelBase() = default;

    void addView(ViewBase* view);

protected:
    void notifyViews(const NotificationBase& notification);
};

class ControllerBase : public ObserverBase {
public:
    ControllerBase(ModelBase* model_);

    ControllerBase(ControllerBase&) = delete;
    ControllerBase& operator = (ControllerBase&) = delete;

    virtual ~ControllerBase() = default;

protected:
    ModelBase* model;
};
