#include "plugin_manager.h"
#include "filters.h"

inline size_t getSrcCoordInMirror1D(size_t coord, size_t newLen, size_t radius) {
    if (newLen < 3 * radius)
        throw std::logic_error("too big radius");
    if (coord >= radius && coord < newLen - radius)
        return coord - radius;
    if (coord < radius)
        return radius - (coord + 1);
    size_t rshift = coord - (newLen - radius);
    return newLen - 2 * radius - 1 - rshift;
}

Image mirror(const Image& srcImage, size_t radius) {
    if (radius > std::min(srcImage.n_rows, srcImage.n_cols))
        throw std::logic_error("too big radius");

    Image resImage(srcImage.n_rows + 2 * radius, srcImage.n_cols + 2 * radius);

    for (size_t row = 0; row < resImage.n_rows; ++row) {
        for (size_t col = 0; col < resImage.n_cols; ++col) {
            size_t srcRow = getSrcCoordInMirror1D(row, resImage.n_rows, radius);
            size_t srcCol = getSrcCoordInMirror1D(col, resImage.n_cols, radius);
            resImage(row, col) = srcImage(srcRow, srcCol);
        }
    }

    return resImage;
}

Image unMirror(const Image& srcImage, size_t radius) {
    return srcImage.submatrix(radius, radius, srcImage.n_rows - 2 * radius, srcImage.n_cols - 2 * radius);
}

class UnSharpFilter : public BaseFilterWrapper {
public:
    UnSharpFilter() : impl({{-1.0 / 6, -2.0 / 3, -1.0 / 6}, {-2.0 / 3, 4 + 1.0 / 3, -2.0 / 3}, {-1.0 / 6, -2.0 / 3, -1.0 / 6}}) {}

    Image applyToImage(const Image& image) const override {
        return unMirror(mirror(image, 1).unary_map(impl), 1);
    }

private:
    KernelFilterImpl<double> impl;
};

class UnSharpPlugin : public IFilterPlugin {
public:
    Image applyToImage(const Image& image) const override {
        return impl->applyToImage(image);
    }

    std::string getName() const override {
        return "unsharp";
    }

private:
    std::unique_ptr<BaseFilterWrapper> impl = std::make_unique<UnSharpFilter>();
};

class UnSharpFactory : public IFilterPluginFactory {
public:
    IFilterPlugin* createObject() override {
        return new UnSharpPlugin;
    }
};

extern "C" void registerPlugin(FilterPluginManager& manager) {
    UnSharpFactory factory;
    manager.registerPlugin(factory);
}
