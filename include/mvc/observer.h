#pragma once

#include <vector>
#include <type_traits>
#include <iostream>

class NotificationBase {
    template <typename NotificationType>
    friend int getNotificationType();

public:
    virtual ~NotificationBase() = default;

    virtual int getType() const = 0;

private:
    static int registerType() {
        static int currentType = 0;
        return currentType++;
    }
};

template <typename NotificationType>
int getNotificationType() {
    static const int type = NotificationBase::registerType();
    return type;
}

#define DECLARE_NOTIFICATION public: int getType() const override { \
    return getNotificationType<std::remove_cv_t<std::remove_reference_t<decltype(*this)>>>(); \
}

class ObserverBase {
public:
    virtual ~ObserverBase() = default;
    virtual void applyNotification(const NotificationBase& notification) = 0;
};

class ObservableBase {
public:
    virtual ~ObservableBase() = default;

    void addObserver(ObserverBase* observer) {
        observers.push_back(observer);
    }

    void notifyObservers(const NotificationBase& notification) {
        for (auto curObserver : observers)
            curObserver->applyNotification(notification);
    }

private:
    std::vector<ObserverBase*> observers{};
};
