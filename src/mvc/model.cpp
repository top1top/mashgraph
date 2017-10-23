#include "mvc/model.h"
#include "align_help.h"

Image loadImage(const char* name) {
    Image srcImage = load_image(name);
    return srcImage;
}

void Model::align(const Image& srcImage, bool isInterp, bool isSubpixel, double subScale)
{

    static const double pyramidScale = 0.5;

    auto images = divideImageOnChannels(srcImage);

    notifyObservers(ImageWasDividedOnChannels());

    if (isSubpixel) {
        std::for_each(images.begin(), images.end(),
                [subScale, isInterp] (Image& im) { im = isInterp ? bicubicResize(im, subScale) : resize(im, subScale); });
    }

    bool willCroped = images[0].n_rows * images[0].n_cols <= 500000;

    std::vector<Image> tmpImages;

    if (!willCroped) {
        for (auto &image : images)
            tmpImages.push_back(image.deep_copy());
    }

    std::for_each(images.begin(), images.end(),
            willCroped ? [] (Image& im) { im = cropImage(im, 10, 30, im.n_rows * 0.07, im.n_cols * 0.07, 2); }
            : [] (Image& im) { im = simpleCropImage(im, 0.04, 0.05); });

    notifyObservers(ImagesWasCropped());

    std::vector<std::vector<Image>> pyramids(images.size());

    for (size_t i = 0; i < images.size(); ++i)
        pyramids[i] = getImagesPyramid(images[i], pyramidScale, 300, isInterp);

    static const int maxShift = 30;

    auto shift0 = getBestShiftForPyramids(pyramids[1], pyramids[0], getBestShiftByMSE, maxShift, 2, pyramidScale);
    auto shift2 = getBestShiftForPyramids(pyramids[1], pyramids[2], getBestShiftByMSE, maxShift, 2, pyramidScale);

    auto ans = mergeImages(willCroped ? images[1] : tmpImages[1],
                           willCroped ? images[0] : tmpImages[0],
                           willCroped ? images[2] : tmpImages[2],
                           shift0, shift2);



    if (isSubpixel) {
        ans = isInterp ? bicubicResize(ans, 1 / subScale) : resize(ans, 1 / subScale);
    }

    resImage = ans;
    notifyObservers(ImagesWasAligned());
}

void Model::align(const char *srcImageName, bool isInterp, bool isSubpixel, double subScale)
{
    Image srcImage = loadImage(srcImageName);
    notifyObservers(LoadImageNotification());
    resImage = {};
    align(srcImage, isInterp, isSubpixel, subScale);
}
