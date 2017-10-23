#pragma once

#include "matrix.h"
#include "io.h"

#include <cstddef>
#include <cmath>
#include <tuple>
#include <iostream>
#include <memory>
#include <vector>

template <typename T>
inline size_t normalizeRes(const T& val) {
    int res = round(val);
    if (res < 0)
        res = -res;
    if (res > 255)
        res = 255;
    return res;
}

template <typename T>
std::tuple<uint, uint, uint> applyKernel(const Image &image, const Matrix<T>& kernel) {
    if (image.n_rows != kernel.n_rows || image.n_cols != kernel.n_cols)
        throw std::logic_error("can't apply gauss kernel, don't correct size subimage");
    T resR = 0, resG = 0, resB = 0;
    for (size_t row = 0; row < kernel.n_rows; ++row) {
        for (size_t col = 0; col < kernel.n_cols; ++col) {
            resR += std::get<0>(image(row, col)) * kernel(row, col);
            resG += std::get<1>(image(row, col)) * kernel(row, col);
            resB += std::get<2>(image(row, col)) * kernel(row, col);
        }
    }
    return std::make_tuple(normalizeRes(resR), normalizeRes(resG), normalizeRes(resB));
}

class BaseFilterImpl {
public:
    virtual std::tuple<uint, uint, uint> operator () (const Image& image) const = 0;

    virtual ~BaseFilterImpl() = default;
};

template <typename T>
class KernelFilterImpl : public BaseFilterImpl {
public:
    KernelFilterImpl(Matrix<T>&& kern) : kernel(kern), n_rows(kernel.n_rows), n_cols(kernel.n_cols) {}

    KernelFilterImpl(const Matrix<T>& kern) : KernelFilterImpl(Matrix<T>(kern)) {}

    std::tuple<uint, uint, uint> operator () (const Image& image) const override {
        return applyKernel(image, kernel);
    }

private:
    Matrix<T> kernel;

public:
    size_t n_rows, n_cols;
};

class BaseFilterWrapper {
public:
    virtual Image applyToImage(const Image& image) const = 0;

    virtual ~BaseFilterWrapper() = default;
};

static Matrix<double> getGaussKernel(size_t radius, double sigma) {
    Matrix<double> kernel(2 * radius + 1, 2 * radius + 1);
    double sum = 0;
    for (size_t row = 0; row < 2 * radius + 1; ++row) {
        for (size_t col = 0; col < 2 * radius + 1; ++col) {
            int y = static_cast<int>(row) - radius;
            int x = static_cast<int>(col) - radius;
            kernel(row, col) = 1 / (2 * M_PI * sigma * sigma) * exp(-(y * y + x * x) / (2 * sigma * sigma));
            sum += kernel(row, col);
        }
    }
    for (size_t row = 0; row < 2 * radius + 1; ++row) {
        for (size_t col = 0; col < 2 * radius + 1; ++col) {
            kernel(row, col) /= sum;
        }
    }
    return kernel;
}

class GaussFilter : public BaseFilterWrapper {
public:
    GaussFilter(size_t radius, double sigma) : impl(getGaussKernel(radius, sigma)) {}

    Image applyToImage(const Image& image) const override {
        return image.unary_map(impl);
    }

private:
    KernelFilterImpl<double> impl;
};

template <typename T>
static bool floatIsEqual(const T& a, const T& b, const T& eps) {
    return abs(a - b) < eps;
}

static std::pair<Matrix<double>, Matrix<double>> getSepGaussKernel(size_t radius, double sigma) {
    Matrix<double> gorizontal(1, 2 * radius + 1), vertical(2 * radius + 1, 1);
    double sum = 0;

    for (int i = -radius; i <= static_cast<int>(radius); ++i) {
        gorizontal(0, i + radius) = 1.0 / sqrt(2 * M_PI * sigma) * exp(-static_cast<int>(i * i) / (2 * sigma * sigma));
        vertical(i + radius, 0) = gorizontal(0, i + radius);
        sum += gorizontal(0, i + radius);
    }
    for (size_t i = 0; i < 2 * radius + 1; ++i) {
        vertical(i, 0) /= sum;
        gorizontal(0, i) /= sum;
    }

    return {gorizontal, vertical};
}

class GaussSepFilter : public BaseFilterWrapper {
public:
    GaussSepFilter(size_t radius, double sigma) : filters(getSepGaussKernel(radius, sigma)) {}

    Image applyToImage(const Image& image) const override {
        return image.unary_map(filters.first).unary_map(filters.second);
    }

private:
    std::pair<KernelFilterImpl<double>, KernelFilterImpl<double>> filters;
};

class SobelKernelX : public BaseFilterWrapper {
public:
    SobelKernelX() : impl({{-1, 0, 1}, {-2, 0, 2}, {-1, 0, 1}}) {}

    Image applyToImage(const Image& image) const override {
        return image.unary_map(impl);
    }

private:
    KernelFilterImpl<int> impl;
};

class SobelKernelY : public BaseFilterWrapper {
public:
    SobelKernelY() : impl({{1, 2, 1}, {0, 0, 0}, {-1, -2, -1}}) {}

    Image applyToImage(const Image& image) const override {
        return image.unary_map(impl);
    }

private:
    KernelFilterImpl<int> impl;
};

class Histogram {
public:
    Histogram() {
        std::fill(std::begin(hist), std::end(hist), 0);
    }

    void add(size_t val) {
        if (val >= size_)
            throw std::logic_error("value greter than histogram size");
        ++hist[val];
        ++elementsCount;
        if (median > val)
            ++skippedElements;
    }

    void remove(size_t val) {
        if (val >= size_)
            throw std::logic_error("value greter than histogram size");
        if (hist[val] == 0)
            throw std::logic_error("don't exist removable value");
        --hist[val];
        --elementsCount;
        if (median > val)
            --skippedElements;
    }

    size_t findMedian() const {
        if (elementsCount == 0)
            throw std::logic_error("histogram is empty");
        while (tryGoRight()) {}
        while (tryGoLeft()) {}
        return median;
    }

    void clear() {
        std::fill(std::begin(hist), std::end(hist), 0);
        median = 0;
        elementsCount = 0;
        skippedElements = 0;
    }

    void addHist(const Histogram& h) {
        for (size_t i = 0; i < size_; ++i) {
            if (h.hist[i] != 0) {
                hist[i] += h.hist[i];
                elementsCount += h.hist[i];
                if (i < median)
                    skippedElements += h.hist[i];
            }
        }
    }

    void subHist(const Histogram& h) {
        for (size_t i = 0; i < size_; ++i) {
            if (h.hist[i] != 0) {
                if (h.hist[i] > hist[i]) {
                    throw std::logic_error("no corect sub histogram");
                }
                hist[i] -= h.hist[i];
                elementsCount -= h.hist[i];
                if (i < median)
                    skippedElements -= h.hist[i];
            }
        }
    }

private:
    static const size_t size_ = 256;

private:
    size_t hist[size_];
    mutable size_t median = 0;
    size_t elementsCount = 0;
    mutable size_t skippedElements = 0;

private:
    bool tryGoRight() const {
        if (elementsCount == 0)
            return false;
        size_t targetSkipped = elementsCount / 2;
        if (skippedElements + hist[median] <= targetSkipped) {
            skippedElements += hist[median];
            ++median;
            return true;
        }
        return false;
    }

    bool tryGoLeft() const {
        if (elementsCount == 0)
            return false;
        size_t targetSkipped = elementsCount / 2;
        if (skippedElements > targetSkipped) {
            --median;
            skippedElements -= hist[median];
            return true;
        }
        return false;
    }
};

inline size_t getBrightness(const std::tuple<uint, uint, uint>& pixel) {
    static const double shareR = 0.2125;
    static const double shareG = 0.7154;
    static const double shareB = 0.0721;
    return shareR * std::get<0>(pixel) + shareG * std::get<1>(pixel) + shareB * std::get<2>(pixel);
}

class AutoContrastFilter : public BaseFilterWrapper {
public:
    AutoContrastFilter(double shareOfDiscared_) : shareOfDiscared(shareOfDiscared_) {}

    Image applyToImage(const Image& image) const override {
        size_t hist[256];
        std::fill(std::begin(hist), std::end(hist), 0);

        for (size_t row = 0; row < image.n_rows; ++row) {
            for (size_t col = 0; col < image.n_cols; ++col)
                ++hist[normalizeRes(getBrightness(image(row, col)))];
        }

        size_t cntDiscared = shareOfDiscared * image.n_rows * image.n_cols;

        size_t discaredRest = cntDiscared;
        size_t i1 = 0;

        while (discaredRest > 0) {
            if (hist[i1] <= discaredRest) {
                discaredRest -= hist[i1];
                hist[i1] = 0;
                ++i1;
            } else {
                hist[i1] -= discaredRest;
                discaredRest = 0;
            }
        }

        discaredRest = cntDiscared;
        size_t i2 = 255;

        while (discaredRest > 0) {
            if (hist[i2] <= discaredRest) {
                discaredRest -= hist[i2];
                hist[i2] = 0;
                --i2;
            } else {
                hist[i2] -= discaredRest;
                discaredRest = 0;
            }
        }

        while (hist[i1] == 0)
            ++i1;
        while (hist[i2] == 0)
            --i2;

        auto func = [i1, i2] (size_t val) -> size_t {
            return normalizeRes(round((val - i1) * 255.0 / (i2 - i1)));
        };

        Image ans(image.n_rows, image.n_cols);

        for (size_t row = 0; row < image.n_rows; ++row) {
            for (size_t col = 0; col < image.n_cols; ++col) {
                auto rgb = image(row, col);
                ans(row, col) = std::make_tuple(normalizeRes(func(std::get<0>(rgb))),
                                                normalizeRes(func(std::get<1>(rgb))),
                                                normalizeRes(func(std::get<2>(rgb))));
            }
        }

        return ans;
    }

private:
    double shareOfDiscared;
};
