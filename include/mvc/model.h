#pragma once

#include "observer.h"
#include "io.h"
#include "align.h"
#include "filters.h"


class LoadImageNotification : public NotificationBase {
    DECLARE_NOTIFICATION
};

class ImageWasDividedOnChannels : public NotificationBase {
    DECLARE_NOTIFICATION
};

class ImagesWasCropped : public NotificationBase {
    DECLARE_NOTIFICATION
};

class ImagesWasAligned : public NotificationBase {
    DECLARE_NOTIFICATION
};

class ImageResultWasProcessed : public NotificationBase {
    DECLARE_NOTIFICATION
};

class Model : public ObservableBase {
public:
    void align(const char *srcImageName, bool isInterp, bool isSubpixel, double subScale);

    void align(const Image& srcImage, bool isInterp, bool isSubpixel, double subScale);

    Image getResultImage() const {
        if (resImage.n_rows > 0 && resImage.n_cols > 0)
            return resImage;
        throw std::string("resImage no exist");
    }

    template <typename Filter>
    void processResult(const Filter& filter) {
        resImage = filter.applyToImage(resImage).deep_copy();
        notifyObservers(ImageResultWasProcessed());
    }

private:
    Image resImage{};
};
