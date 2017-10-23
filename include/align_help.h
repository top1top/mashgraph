#pragma once

#include "align.h"
#include <vector>
#include <numeric>
#include <initializer_list>

Image cropImage(const Image& src_image, int threshold1, int threshold2, size_t countRows, size_t countColumns, size_t cntNullable);

Image simpleCropImage(const Image& im, double rowsDiscared, double colsDiscared);

std::vector<Image> getImagesPyramid(const Image& srcImage, double k, size_t minLen, bool isInterp);

template <typename Func>
static std::pair<int, int> getBestShiftForPyramids(const std::vector<Image>& pyramid1, const std::vector<Image>& pyramid2,
        Func getBestShiftFor2, int maxShiftBegin, int maxShiftCorr, double k)
{
    if (pyramid1.empty() || pyramid2.empty())
        throw std::logic_error("one of pyramids is empty");

    std::pair<int, int> bestShift = {0, 0};
    for (int i = std::min(pyramid1.size(), pyramid2.size()) - 1; i >= 0; --i) {
        bestShift.first = round(bestShift.first / k);
        bestShift.second = round(bestShift.second / k);
        size_t maxShift = i == static_cast<int>(std::min(pyramid1.size(), pyramid2.size())) - 1 ? maxShiftBegin : maxShiftCorr;
        bestShift = getBestShiftFor2(pyramid1[i], pyramid2[i],
                bestShift.first - maxShift, bestShift.first + maxShift,
                bestShift.second - maxShift, bestShift.second + maxShift);
    }

    return bestShift;
}

std::vector<Image> divideImageOnChannels(const Image &image);

struct CrossImageResult {
    size_t up, left, height, width;
};

CrossImageResult crossImagesImpl(const std::pair<size_t, size_t>& baseImage,
                                 std::initializer_list<std::pair<size_t, size_t>> imagesSize,
                                 std::initializer_list<std::pair<int, int>> shifts);

CrossImageResult crossImages(const Image& image1, const Image& image2, int rowShift, int colShift);

CrossImageResult crossImages(const Image& image1, const Image& image2, const Image& image3,
                             int rowShift2, int colShift2, int rowShift3, int colShift3);

template <typename Func>
unsigned long long calculateSum(const Image& image1, const Image& image2, int rowShift, int colShift, Func func) {
    auto cross = crossImages(image1, image2, rowShift, colShift);
    unsigned long long res = 0;

    for (size_t r1 = cross.up, r2 = r1 - rowShift; r1 < image1.n_rows && r2 < image2.n_rows; ++r1, ++r2) {
        for (size_t c1 = cross.left, c2 = c1 - colShift; c1 < image1.n_cols && c2 < image2.n_cols; ++c1, ++c2) {
            res += func(std::get<0>(image1(r1, c1)), std::get<0>(image2(r2, c2)));
        }
    }

    return res;
}

long double calculateMSE(const Image& image1, const Image& image2, int rowShift, int colShift);

unsigned long long calculateCrossCorrelation(const Image& image1, const Image& image2, int rowShift, int colShift);

enum class ActionType {
    MINIMIZE, MAXIMIZE
};

template <typename KeyFunc>
std::pair<int, int> getBestShiftImpl(int minRowShift, int maxRowShift, int minColShift, int maxColShift, KeyFunc key, ActionType action) {
    int bestDRow = minRowShift;
    int bestDCol = minColShift;
    unsigned long long bestVal = action == ActionType::MINIMIZE ? std::numeric_limits<unsigned long long>::max() : 0;

    for (int dRow = minRowShift; dRow <= maxRowShift; ++dRow) {
        for (int dCol = minColShift; dCol <= maxColShift; ++dCol) {
            auto curVal = key(dRow, dCol);
            if ((action == ActionType::MINIMIZE && curVal < bestVal) || (action == ActionType::MAXIMIZE && curVal > bestVal)) {
                bestVal = curVal;
                bestDRow = dRow;
                bestDCol = dCol;
            }
        }
    }
    return {bestDRow, bestDCol};
}

std::pair<int, int> getBestShiftByMSE(const Image& image1, const Image& image2,
        int minRowShift, int maxRowShift, int minColShift, int maxColShift);

std::pair<int, int> getBestShiftByCrossCorrelation(const Image& image1, const Image& image2,
        int minRowShift, int maxRowShift, int minColShift, int maxColShift);

// GBR
Image mergeImages(const Image& imageBase, const Image& image1, const Image& image2,
                  const std::pair<int, int>& shif1, const std::pair<int, int>& shift2);
