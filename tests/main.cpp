#include <gtest/gtest.h>
#include <align_help.h>
#include <cstdlib>
#include <stdexcept>
#include <filters.h>

template <typename T>
bool doubleEqual(const T& val1, const T& val2, const T& eps) {
    return abs(val1 - val2) < eps;
}

template <typename T, typename Pred>
bool matrixIsEqual(const Matrix<T>& matrix1, const Matrix<T>& matrix2, Pred pred) {
    if (matrix1.n_rows != matrix2.n_rows || matrix1.n_cols != matrix2.n_cols)
        return false;
    for (size_t row = 0; row < matrix1.n_rows; ++row) {
        for (size_t col = 0; col < matrix1.n_cols; ++col) {
            if (!pred(matrix1(row, col), matrix2(row, col)))
                return false;
        }
    }
    return true;
}

template <typename T>
bool matrixIsEqual(const Matrix<T>& matrix1, const Matrix<T>& matrix2) {
    return matrixIsEqual(matrix1, matrix2, std::equal_to<T>());
}

static bool imagesIsEqual(const Image& image1, const Image& image2) {
    return matrixIsEqual(image1, image2);
}

static Image mergeImagesVertical(const std::vector<Image>& images) {
    if (images.empty())
        throw std::logic_error("can't merger 0 images");
    size_t height = 0;
    for (size_t i = 1; i < images.size(); ++i) {
        if (images[i].n_cols != images[i - 1].n_cols)
            throw std::logic_error("can't vertical merge images with different widths");
    }
    for (const auto& image : images)
        height += image.n_rows;
    Image ans(height, images[0].n_cols);
    size_t curRow = 0;
    for (const auto& image : images) {
        for (size_t row = 0; row < image.n_rows; ++row, ++curRow) {
            for (size_t col = 0; col < image.n_cols; ++col) {
                ans(curRow, col) = image(row, col);
            }
        }
    }
    return ans;
}

static void checkSplitImage(size_t height, size_t width) {
    Image im(height, width);
    for (size_t row = 0; row < im.n_rows; ++row) {
        for (size_t col = 0; col < im.n_cols; ++col) {
            im(row, col) = {rand() % 255, rand() % 255, rand() % 255};
        }
    }
    auto images = divideImageOnChannels(im);
    ASSERT_EQ(images.size(), 3);
    ASSERT_EQ(images[0].n_rows + images[1].n_rows + images[2].n_rows, height);
    for (size_t i = 0; i < 3; ++i)
        ASSERT_EQ(images[i].n_cols, width);
    size_t minRows = std::min({images[0].n_rows, images[1].n_rows, images[2].n_rows});
    size_t maxRows = std::max({images[0].n_rows, images[1].n_rows, images[2].n_rows});
    ASSERT_LE(maxRows - minRows, 1);
    ASSERT_TRUE(imagesIsEqual(mergeImagesVertical(images), im));
}

TEST(Images, divideImageOnChannels) {
    checkSplitImage(100, 100);
    checkSplitImage(99, 100);
    checkSplitImage(3, 100);
    checkSplitImage(98, 100);
    checkSplitImage(50, 1);
    checkSplitImage(92, 5);
}

TEST(Images, crossImagesImpl) {
    CrossImageResult res;

    // two images

    res = crossImagesImpl({10, 10}, {{10, 10}}, {{0, 0}});
    ASSERT_TRUE(res.up == 0 && res.left == 0 && res.height == 10 && res.width == 10);

    res = crossImagesImpl({10, 10}, {{10, 10}}, {{1, 1}});
    ASSERT_TRUE(res.up == 1 && res.left == 1 && res.height == 9 && res.width == 9);

    res = crossImagesImpl({10, 10}, {{10, 10}}, {{-1, -1}});
    ASSERT_TRUE(res.up == 0 && res.left == 0 && res.height == 9 && res.width == 9);

    res = crossImagesImpl({8, 5}, {{6, 10}}, {{-4, 1}});
    ASSERT_TRUE(res.up == 0 && res.left == 1 && res.height == 2 && res.width == 4);

    res = crossImagesImpl({10, 10}, {{10, 10}}, {{-2, 2}});
    ASSERT_TRUE(res.up == 0 && res.left == 2 && res.height == 8 && res.width == 8);

    // three images

    res = crossImagesImpl({10, 10}, {{10, 10}, {10, 10}}, {{-2, 2}, {-3, 3}});
    ASSERT_TRUE(res.up == 0 && res.left == 3 && res.height == 7 && res.width == 7);

    res = crossImagesImpl({10, 10}, {{10, 10}, {10, 10}}, {{-2, 2}, {0, 0}});
    ASSERT_TRUE(res.up == 0 && res.left == 2 && res.height == 8 && res.width == 8);

    res = crossImagesImpl({10, 10}, {{10, 10}, {10, 10}}, {{2, 2}, {3, 5}});
    ASSERT_TRUE(res.up == 3 && res.left == 5 && res.height == 7 && res.width == 5);
}

TEST(Images, crossImages2) {
    CrossImageResult res;

    res = crossImages(Image(5, 6), Image(5, 6), 0, 0);
    ASSERT_TRUE(res.up == 0 && res.left == 0 && res.height == 5 && res.width == 6);

    res = crossImages(Image(5, 6), Image(5, 6), 2, 0);
    ASSERT_TRUE(res.up == 2 && res.left == 0 && res.height == 3 && res.width == 6);

    res = crossImages(Image(5, 6), Image(10, 10), 2, 2);
    ASSERT_TRUE(res.up == 2 && res.left == 2 && res.height == 3 && res.width == 4);

    res = crossImages(Image(5, 10), Image(10, 6), -2, 3);
    ASSERT_TRUE(res.up == 0 && res.left == 3 && res.height == 5 && res.width == 6);
}

TEST(Images, crossImages3) {
    CrossImageResult res;

    res = crossImages(Image(5, 5), Image(5, 5), Image(5, 5), 0, 0, 0, 0);
    ASSERT_TRUE(res.up == 0 && res.left == 0 && res.height == 5 && res.width == 5);

    res = crossImages(Image(5, 5), Image(5, 5), Image(5, 5), 1, -1, 2, -2);
    ASSERT_TRUE(res.up == 2 && res.left == 0 && res.height == 3 && res.width == 3);
}

TEST(Images, calculateSum) {
    {
        Image image1(2, 2), image2(2, 2);

        image1(0, 0) = {2, 2, 2};
        image1(0, 1) = {4, 4, 4};
        image1(1, 0) = {10, 10, 10};
        image1(1, 1) = {0, 0, 0};

        image2(0, 0) = {2, 2, 2};
        image2(0, 1) = {4, 4, 4};
        image2(1, 0) = {10, 10, 10};
        image2(1, 1) = {0, 0, 0};

        auto sum = calculateSum(image1, image2, 0, 0, [] (int val1, int val2) { return abs(val1 - val2); });
        ASSERT_EQ(sum, 0);
    }

    {
        Image image1(2, 2), image2(2, 2);

        image1(0, 0) = {2, 2, 2};
        image1(0, 1) = {4, 4, 4};
        image1(1, 0) = {10, 10, 10};
        image1(1, 1) = {0, 0, 0};

        image2(0, 0) = {5, 5, 5};
        image2(0, 1) = {2, 2, 2};
        image2(1, 0) = {8, 8, 8};
        image2(1, 1) = {1, 1, 1};

        auto sum = calculateSum(image1, image2, 0, 0, [] (int val1, int val2) { return abs(val1 - val2); });
        ASSERT_EQ(sum, 3 + 2 + 2 + 1);

        sum = calculateSum(image1, image2, 1, 0, [] (int val1, int val2) { return abs(val1 - val2); });
        ASSERT_EQ(sum, 7);

        sum = calculateSum(image1, image2, -1, 0, [] (int val1, int val2) { return abs(val1 - val2); });
        ASSERT_EQ(sum, 9);

        sum = calculateSum(image1, image2, -1, -1, [] (int val1, int val2) { return abs(val1 - val2); });
        ASSERT_EQ(sum, 1);

        sum = calculateSum(image1, image2, 1, 1, [] (int val1, int val2) { return abs(val1 - val2); });
        ASSERT_EQ(sum, 5);
    }

    {
        Image image1(2, 1), image2(1, 2);

        image1(0, 0) = {1, 1, 1};
        image1(1, 0) = {2, 2, 2};

        image2(0, 0) = {4, 4, 4};
        image2(0, 1) = {3, 3, 3};

        auto sum = calculateSum(image1, image2, 0, 0, [] (int val1, int val2) { return (val1 - val2) * (val1 - val2); });
        ASSERT_EQ(sum, 9);

        sum = calculateSum(image1, image2, 1, 0, [] (int val1, int val2) { return (val1 - val2) * (val1 - val2); });
        ASSERT_EQ(sum, 4);

        sum = calculateSum(image1, image2, 0, -1, [] (int val1, int val2) { return (val1 - val2) * (val1 - val2); });
        ASSERT_EQ(sum, 4);
    }
}

TEST(Images, calculateMSE) {
    const long double eps = 0.0000001L;
    {
        Image image1(2, 2), image2(2, 2);

        image1(0, 0) = {2, 2, 2};
        image1(0, 1) = {4, 4, 4};
        image1(1, 0) = {10, 10, 10};
        image1(1, 1) = {0, 0, 0};

        image2(0, 0) = {2, 2, 2};
        image2(0, 1) = {4, 4, 4};
        image2(1, 0) = {10, 10, 10};
        image2(1, 1) = {0, 0, 0};

        auto sum = calculateMSE(image1, image2, 0, 0);
        ASSERT_EQ(sum, 0);
    }

    {
        Image image1(2, 2), image2(2, 2);

        image1(0, 0) = {2, 2, 2};
        image1(0, 1) = {4, 4, 4};
        image1(1, 0) = {10, 10, 10};
        image1(1, 1) = {0, 0, 0};

        image2(0, 0) = {5, 5, 5};
        image2(0, 1) = {2, 2, 2};
        image2(1, 0) = {8, 8, 8};
        image2(1, 1) = {1, 1, 1};

        auto sum = calculateMSE(image1, image2, 0, 0);
        ASSERT_TRUE(doubleEqual(sum, static_cast<long double>(3 * 3 + 2 * 2 + 2 * 2 + 1 * 1) / 4, eps));

        sum = calculateMSE(image1, image2, 1, 0);
        ASSERT_TRUE(doubleEqual(sum, static_cast<long double>(5 * 5 + 2 * 2) / 2, eps));

        sum = calculateMSE(image1, image2, -1, 0);
        ASSERT_TRUE(doubleEqual(sum, static_cast<long double>(6 * 6 + 3 * 3) / 2, eps));

        sum = calculateMSE(image1, image2, -1, -1);
        ASSERT_TRUE(doubleEqual(sum, static_cast<long double>(1 * 1) / 1, eps));

        sum = calculateMSE(image1, image2, 1, 1);
        ASSERT_TRUE(doubleEqual(sum, static_cast<long double>(5 * 5) / 1, eps));
    }
}

TEST(Images, calculateCrossCorrelation) {
     {
        Image image1(2, 2), image2(2, 2);

        image1(0, 0) = {2, 2, 2};
        image1(0, 1) = {4, 4, 4};
        image1(1, 0) = {10, 10, 10};
        image1(1, 1) = {0, 0, 0};

        image2(0, 0) = {2, 2, 2};
        image2(0, 1) = {4, 4, 4};
        image2(1, 0) = {10, 10, 10};
        image2(1, 1) = {0, 0, 0};

        auto sum = calculateCrossCorrelation(image1, image2, 0, 0);
        ASSERT_EQ(sum, 2 * 2 + 4 * 4 + 10 * 10 + 0 * 0);
    }

    {
        Image image1(2, 2), image2(2, 2);

        image1(0, 0) = {2, 2, 2};
        image1(0, 1) = {4, 4, 4};
        image1(1, 0) = {10, 10, 10};
        image1(1, 1) = {0, 0, 0};

        image2(0, 0) = {5, 5, 5};
        image2(0, 1) = {2, 2, 2};
        image2(1, 0) = {8, 8, 8};
        image2(1, 1) = {1, 1, 1};

        auto sum = calculateCrossCorrelation(image1, image2, 0, 0);
        ASSERT_EQ(sum, 2 * 5 + 4 * 2 + 10 * 8 + 0 * 1);

        sum = calculateCrossCorrelation(image1, image2, 1, 0);
        ASSERT_EQ(sum, 10 * 5 + 0 * 2);

        sum = calculateCrossCorrelation(image1, image2, -1, 0);
        ASSERT_EQ(sum, 2 * 8 + 1 * 4);

        sum = calculateCrossCorrelation(image1, image2, -1, -1);
        ASSERT_EQ(sum, 2);

        sum = calculateCrossCorrelation(image1, image2, 1, 1);
        ASSERT_EQ(sum, 0);
    }
}

TEST(Images, getBestShiftImpl) {
    auto bestShift = getBestShiftImpl(-15, 15, -15, 15, [] (int dr, int dc) { return (dr + dc - 5 + 15) % 15; }, ActionType::MAXIMIZE);
    ASSERT_EQ((bestShift.first + bestShift.second - 5 + 150) % 15, 14);
    bestShift = getBestShiftImpl(-15, 15, -15, 15, [] (int dr, int dc) { return (dr + dc - 5 + 15) % 15; }, ActionType::MINIMIZE);
    ASSERT_EQ((bestShift.first + bestShift.second - 5 + 150) % 15, 0);
}

TEST(Images, getBestShiftByMSE) {
    {
        Image image1 = { {{1, 1, 1}, {2, 2, 2}},
                         {{3, 3, 3}, {4, 4, 4}} };
        Image image2 = { {{1, 1, 1}, {2, 2, 2}},
                         {{3, 3, 3}, {4, 4, 4}} };

        auto res = getBestShiftByMSE(image1, image2, -1, 1, -1, 1);
        ASSERT_EQ(res.first, 0);
        ASSERT_EQ(res.second, 0);
    }

    {
        Image image1 = { {{255, 255, 255}, {2, 2, 2}},
                         {{3, 3, 3}, {4, 4, 4}} };
        Image image2 = { {{1, 1, 1}, {2, 2, 2}},
                         {{3, 3, 3}, {255, 255, 255}} };

        auto res = getBestShiftByMSE(image1, image2, -1, 1, -1, 1);
        ASSERT_EQ(res.first, -1);
        ASSERT_EQ(res.second, -1);
    }

    {
        Image image1 = { {{1, 1, 1}, {255, 255, 255}},
                         {{3, 3, 3}, {4, 4, 4}} };
        Image image2 = { {{255, 255, 255}, {255, 255, 255}},
                         {{3, 3, 3}, {255, 255, 255}} };

        auto res = getBestShiftByMSE(image1, image2, -1, 1, -1, 1);
        ASSERT_EQ(res.first, 0);
        ASSERT_EQ(res.second, 1);
    }
}

TEST(Images, getBestShiftByCrossCorrelation) {
    {
        Image image1 = { {{1, 1, 1}, {2, 2, 2}},
                         {{3, 3, 3}, {4, 4, 4}} };
        Image image2 = { {{1, 1, 1}, {2, 2, 2}},
                         {{3, 3, 3}, {4, 4, 4}} };

        auto res = getBestShiftByCrossCorrelation(image1, image2, -1, 1, -1, 1);
        ASSERT_EQ(res.first, 0);
        ASSERT_EQ(res.second, 0);
    }

    {
        Image image1 = { {{255, 255, 255}, {2, 2, 2}},
                         {{3, 3, 3}, {4, 4, 4}} };
        Image image2 = { {{1, 1, 1}, {2, 2, 2}},
                         {{3, 3, 3}, {255, 255, 255}} };

        auto res = getBestShiftByCrossCorrelation(image1, image2, -1, 1, -1, 1);
        ASSERT_EQ(res.first, -1);
        ASSERT_EQ(res.second, -1);
    }

    {
        Image image1 = { {{1, 1, 1}, {255, 255, 255}},
                         {{3, 3, 3}, {4, 4, 4}} };
        Image image2 = { {{255, 255, 255}, {0, 0, 0}},
                         {{3, 3, 3}, {0, 0, 0}} };

        auto res = getBestShiftByCrossCorrelation(image1, image2, -1, 1, -1, 1);
        ASSERT_EQ(res.first, 0);
        ASSERT_EQ(res.second, 1);
    }
}

TEST(Images, mergeImages) {
    {
        Image image1 = { {{1, 1, 1}, {2, 2, 2}},
                         {{3, 3, 3}, {4, 4, 4}} };
        Image image2 = { {{1, 1, 1}, {2, 2, 2}},
                         {{3, 3, 3}, {4, 4, 4}} };
        Image image3 = { {{1, 1, 1}, {2, 2, 2}},
                         {{3, 3, 3}, {4, 4, 4}} };

        auto resImage = mergeImages(image1, image2, image3, {0, 0}, {0, 0});
        ASSERT_TRUE(imagesIsEqual(resImage, Image({ {{1, 1, 1}, {2, 2, 2}},
                                                    {{3, 3, 3}, {4, 4, 4}} })));

        resImage = mergeImages(image1, image2, image3, {1, 1}, {0, 1});
        ASSERT_TRUE(imagesIsEqual(resImage, Image({ {{3, 4, 1}} })));
    }
}

TEST(Filters, Gauss) {
    Matrix<double> expectedMatrix = { {0.003, 0.013, 0.022, 0.013, 0.003},
                                      {0.013, 0.059, 0.097, 0.059, 0.013},
                                      {0.022, 0.097, 0.159, 0.097, 0.022},
                                      {0.013, 0.059, 0.097, 0.059, 0.013},
                                      {0.003, 0.013, 0.022, 0.013, 0.003} };

    ASSERT_TRUE(matrixIsEqual(getGaussKernel(2, 1), expectedMatrix, [] (double val1, double val2) {
        return doubleEqual(val1, val2, 0.005);
    }));

    Image image(5, 5);

    for (size_t row = 0; row < 5; ++row) {
        for (size_t col = 0; col < 5; ++col) {
            image(row, col) = {1, 1, 1};
        }
    }

    KernelFilterImpl<double> gauss(getGaussKernel(2, 1));

    ASSERT_EQ(std::get<0>(gauss(image)), 1);
}

TEST(Filters, SepGauss) {
    srand(223);
    GaussSepFilter sepFilter(2, 1);
    GaussFilter filter(2, 1);

    Image image(20, 20);

    for (size_t i = 0; i < 100; ++i) {
        for (size_t row = 0; row < image.n_rows; ++row) {
            for (size_t col = 0; col < image.n_cols; ++col) {
                size_t r = rand() % 255;
                image(row, col) = {r, r, r};
            }
        }
        auto image1 = filter.applyToImage(image);
        auto image2 = sepFilter.applyToImage(image);
        ASSERT_TRUE(matrixIsEqual(image1.submatrix(2, 2, 16, 16), image2.submatrix(2, 2, 16, 16),
                    [] (const std::tuple<uint, uint, uint>& a, const std::tuple<uint, uint, uint>& b) {
                        return abs(int(std::get<0>(a)) - int(std::get<0>(b))) <= 1;
                    }));
    }
}

TEST(Filters, MediansCmp) {
    srand(223);

    for (size_t i = 0; i < 100; ++i) {
        size_t n_rows = rand() % 100 + 10;
        size_t n_cols = rand() % 100 + 10;

        Image im(n_rows, n_cols);

        for (size_t row = 0; row < im.n_rows; ++row) {
            for (size_t col = 0; col < im.n_cols; ++col)
                im(row, col) = {rand() % 256, rand() % 256, rand() % 256};
        }

        size_t radius = rand() % (std::min(im.n_rows, im.n_cols) / 2 - 1) + 1;

        Image res1 = MedianSimpleFilter(radius).applyToImage(im);
        Image res2 = MedianLinearFilter(radius).applyToImage(im);
        Image res3 = MedianConstFilter(radius).applyToImage(im);

        ASSERT_TRUE(imagesIsEqual(res1, res2));
        ASSERT_TRUE(imagesIsEqual(res2, res3));
    }
}

TEST(Mirror, SimpleTest) {
    {
        Image im = { {{1, 1, 1}, {2, 2, 2}, {3, 3, 3}},
                     {{4, 4, 4}, {5, 5, 5}, {6, 6, 6}},
                     {{7, 7, 7}, {8, 8, 8}, {9, 9, 9}} };
        auto im1 = mirror(im, 0);
        ASSERT_TRUE(imagesIsEqual(im, im1));
        auto im2 = mirror(im, 1);
        ASSERT_TRUE(imagesIsEqual(im2, Image({ {{1, 1, 1}, {1, 1, 1}, {2, 2, 2}, {3, 3, 3}, {3, 3, 3}},
                                               {{1, 1, 1}, {1, 1, 1}, {2, 2, 2}, {3, 3, 3}, {3, 3, 3}},
                                               {{4, 4, 4}, {4, 4, 4}, {5, 5, 5}, {6, 6, 6}, {6, 6, 6}},
                                               {{7, 7, 7}, {7, 7, 7}, {8, 8, 8}, {9, 9, 9}, {9, 9, 9}},
                                               {{7, 7, 7}, {7, 7, 7}, {8, 8, 8}, {9, 9, 9}, {9, 9, 9}}})));
    }
}
