#include "align_help.h"

#include <stdexcept>
#include <algorithm>
#include <cassert>

Image cropImage(const Image& src_image, int threshold1, int threshold2, size_t countRows, size_t countColumns, size_t cntNullable) {
    auto cannyImage = canny(src_image, threshold1, threshold2);

    struct BorderInfo {
        size_t line;    // start row or column
        int dl;         // next line
        enum LineType {
            ROW, COLUMN
        };
        LineType type;
    };

    BorderInfo borders[4] = {{0, 1, BorderInfo::ROW},
                             {src_image.n_rows - 1, -1, BorderInfo::ROW},
                             {0, 1, BorderInfo::COLUMN},
                             {src_image.n_cols - 1, -1, BorderInfo::COLUMN}};

    size_t up = 0, down = src_image.n_rows - 1, left = 0, right = src_image.n_cols - 1;

    for (const auto& cur : borders) {
        size_t wd = cur.type == BorderInfo::ROW ? countRows : countColumns;
        size_t cntLines = 0;

        std::vector<size_t> values;

        for (size_t line = cur.line; cntLines < wd; line += cur.dl, ++cntLines) {
            size_t row, col;
            size_t drow = 0, dcol = 0;
            if (cur.type == BorderInfo::ROW) {
                row = line;
                col = 0;
                dcol = 1;
            } else {
                col = line;
                row = 0;
                drow = 1;
            }
            size_t cntOnBorder = 0;
            for (; row < src_image.n_rows && col < src_image.n_cols; row += drow, col += dcol) {
                if (std::get<0>(cannyImage(row, col)) != 0)
                    ++cntOnBorder;
            }
            values.push_back(cntOnBorder);
        }

        size_t lineMax1 = std::max_element(values.begin(), values.end()) - values.begin();
        for (int i = static_cast<int>(lineMax1) - cntNullable; i <= static_cast<int>(lineMax1 + cntNullable); ++i) {
            if (i >= 0 && i < static_cast<int>(values.size()))
                values[i] = 0;
        }
        size_t lineMax2 = std::max_element(values.begin(), values.end()) - values.begin();
        size_t lineMax = std::max(lineMax1, lineMax2);
        size_t lineId = cur.line + cur.dl * lineMax;
        if (cur.type == BorderInfo::ROW) {
            if (cur.dl > 0)
                up = lineId;
            else
                down = lineId;
        } else {
            if (cur.dl > 0)
                left = lineId;
            else
                right = lineId;
        }
    }

    return src_image.submatrix(up, left, down - up + 1, right - left + 1).deep_copy();
}

std::vector<Image> divideImageOnChannels(const Image &image) {
    std::vector<Image> images;
    size_t current_row = 0;
    for (int i = 0; i < 3; ++i) {
        size_t current_height = (image.n_rows - current_row) / (3 - i);
        images.push_back(image.submatrix(current_row, 0, current_height, image.n_cols));
        current_row += current_height;
    }
    return images;
}


std::vector<Image> getImagesPyramid(const Image& srcImage, double k, size_t minLen, bool isInterp) {
    std::vector<Image> pyramid;
    pyramid.push_back(srcImage.deep_copy());
    Image curImage = isInterp ? bicubicResize(srcImage, k) : resize(srcImage, k);
    while (std::min(curImage.n_rows, curImage.n_cols) >= minLen) {
        pyramid.push_back(curImage);
        curImage = isInterp ? bicubicResize(curImage, k) : resize(curImage, k);
    }
    return pyramid;
}

Image simpleCropImage(const Image& im, double rowsDiscared, double colsDiscared) {
    size_t drows = round(im.n_rows * rowsDiscared);
    size_t dcols = round(im.n_cols * colsDiscared);
    return im.submatrix(drows, dcols, im.n_rows - 2 * drows, im.n_cols - 2 * dcols).deep_copy();
}

CrossImageResult crossImagesImpl(const std::pair<size_t, size_t>& baseImage,
                                 std::initializer_list<std::pair<size_t, size_t>> imagesSize,
                                 std::initializer_list<std::pair<int, int>> shifts) {
    assert(imagesSize.size() == shifts.size());
    CrossImageResult res;
    res.up = 0;
    res.left = 0;
    size_t down = baseImage.first - 1;
    size_t right = baseImage.second - 1;
    auto itImages = imagesSize.begin();
    auto itShifts = shifts.begin();
    for (; itImages != imagesSize.end() && itShifts != shifts.end(); ++itImages, ++itShifts) {
        if (itImages->first == 0 || itImages->second == 0)
            throw std::logic_error("empty image in input");
        if (itShifts->first > 0)
            res.up = std::max(res.up, size_t(itShifts->first));
        if (itShifts->second > 0)
            res.left = std::max(res.left, size_t(itShifts->second));
        down = std::min(down, itImages->first  - 1 + itShifts->first);
        right = std::min(right, itImages->second - 1 + itShifts->second);
    }

    if (down < res.up || right < res.left)
        throw std::logic_error("images hasn't non empty cross");

    res.height = down - res.up + 1;
    res.width = right - res.left + 1;

    return res;
}
CrossImageResult crossImages(const Image& image1, const Image& image2, int rowShift, int colShift) {
    return crossImagesImpl({image1.n_rows, image1.n_cols}, {{image2.n_rows, image2.n_cols}}, {{rowShift, colShift}});
}

CrossImageResult crossImages(const Image& image1, const Image& image2, const Image& image3,
                             int rowShift2, int colShift2, int rowShift3, int colShift3) {
    return crossImagesImpl({size_t(image1.n_rows), size_t(image1.n_cols)},
                           {{size_t(image2.n_rows), size_t(image2.n_cols)}, {size_t(image3.n_rows), size_t(image3.n_cols)}},
                           {{rowShift2, colShift2}, {rowShift3, colShift3}});
}

long double calculateMSE(const Image& image1, const Image& image2, int rowShift, int colShift) {
    auto cross = crossImages(image1, image2, rowShift, colShift);   // TODO calculate cross one time
    return static_cast<long double>(calculateSum(image1, image2, rowShift, colShift, [](size_t val1, size_t val2) {
        int d = static_cast<int>(val1) - static_cast<int>(val2);
        return d * d;
    })) / (cross.height * cross.width);
}

unsigned long long calculateCrossCorrelation(const Image& image1, const Image& image2, int rowShift, int colShift) {
    return calculateSum(image1, image2, rowShift, colShift, [](size_t val1, size_t val2) {
        return val1 * val2;
    });
}

std::pair<int, int> getBestShiftByMSE(const Image& image1, const Image& image2,
        int minRowShift, int maxRowShift, int minColShift, int maxColShift)
{
    return getBestShiftImpl(minRowShift, maxRowShift, minColShift, maxColShift, [&image1, &image2](int dRow, int dCol) {
        return calculateMSE(image1, image2, dRow, dCol);
    }, ActionType::MINIMIZE);
}

std::pair<int, int> getBestShiftByCrossCorrelation(const Image& image1, const Image& image2,
        int minRowShift, int maxRowShift, int minColShift, int maxColShift)
{
    return getBestShiftImpl(minRowShift, maxRowShift, minColShift, maxColShift, [&image1, &image2](int dRow, int dCol) {
        return calculateCrossCorrelation(image1, image2, dRow, dCol);
    }, ActionType::MAXIMIZE);
}

// GBR
Image mergeImages(const Image& imageBase, const Image& image1, const Image& image2,
                  const std::pair<int, int>& shif1, const std::pair<int, int>& shift2)
{
    auto cross = crossImages(imageBase, image1, image2, shif1.first, shif1.second, shift2.first, shift2.second);
    Image ans(cross.height, cross.width);
    for (size_t r = cross.up, r1 = r - shif1.first, r2 = r - shift2.first;
         r < imageBase.n_rows && r1 < image1.n_rows && r2 < image2.n_rows;
         ++r, ++r1, ++r2)
    {
        for (size_t c = cross.left, c1 = c - shif1.second, c2 = c - shift2.second;
             c < imageBase.n_cols && c1 < image1.n_cols && c2 < image2.n_cols;
             ++c, ++c1, ++c2)
        {
            std::get<0>(ans(r - cross.up, c - cross.left)) = std::get<0>(image2(r2, c2));
            std::get<1>(ans(r - cross.up, c - cross.left)) = std::get<0>(imageBase(r, c));
            std::get<2>(ans(r - cross.up, c - cross.left)) = std::get<0>(image1(r1, c1));
        }
    }
    return ans;
}

