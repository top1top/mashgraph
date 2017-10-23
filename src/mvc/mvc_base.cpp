#include "mvc/mvc_base.h"

ViewBase::ViewBase(const ModelBase* model_, ControllerBase* controller_) : model(model_), controller(controller_) {
}

void ViewBase::applyNotification(const NotificationBase& notification) {
    applyModelNotification(notification);
}

void ModelBase::notifyViews(const NotificationBase& notification) {
    notifyObservers(notification);
}

void ModelBase::addView(ViewBase* view) {
    addObserver(view);
}

ControllerBase::ControllerBase(ModelBase* model_) : model(model_) {}
