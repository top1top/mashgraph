#pragma once

#include "observer.h"
#include "model.h"
#include "io.h"

#include <vector>
#include <string>
#include <fstream>

namespace ConsoleViews {
    class TextView : public ObserverBase {
    public:
        TextView(const Model* model_, const char* logFileName) : model(model_), logFile(std::make_unique<std::ofstream>(logFileName)) {}

        TextView(const TextView&) = delete;
        TextView& operator = (const TextView&) = delete;

        void applyNotification(const NotificationBase& notification) override {
            if (notification.getType() == getNotificationType<LoadImageNotification>()) {
                (*logFile) << "image was loading" << std::endl;
            } else if (notification.getType() == getNotificationType<ImageWasDividedOnChannels>()) {
                (*logFile) << "image was diveded on channels" << std::endl;
            } else if (notification.getType() == getNotificationType<ImagesWasCropped>()) {
                (*logFile) << "image was cropped" << std::endl;
            } else if (notification.getType() == getNotificationType<ImagesWasAligned>()) {
                (*logFile) << "image was aligned" << std::endl;
            } else if (notification.getType() == getNotificationType<ImageResultWasProcessed>()) {
                (*logFile) << "image was postprocessing" << std::endl;
            }
        }

    private:
        const Model* model;
        std::unique_ptr<std::ofstream> logFile;
    };

    class ImageView : public ObserverBase {
    public:
        ImageView(const Model* model_, const char* resultPath_) : model(model_), resultPath(resultPath_) {}

        ImageView(const ImageView&) = delete;

        void applyNotification(const NotificationBase& notification) override {
            if (notification.getType() == getNotificationType<ImagesWasAligned>()) {
                save_image(model->getResultImage(), resultPath);
            } else if (notification.getType() == getNotificationType<ImageResultWasProcessed>()) {
                save_image(model->getResultImage(), resultPath);
            }
        }

        ImageView& operator = (const ImageView&) = delete;

    private:
        const Model* model;
        const char* resultPath;
    };
}
