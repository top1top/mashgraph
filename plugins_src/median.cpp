#include "plugin_manager.h"
#include "filters.h"

#include <sstream>
#include <cassert>

class MedianSimpleFilter : public BaseFilterWrapper {
public:
    MedianSimpleFilter(size_t radius_) : radius(radius_) {}

    Image applyToImage(const Image& image) const override {
        Image ans = image.deep_copy();
        for (size_t row = radius; row < image.n_rows - radius; ++row) {
            for (size_t col = radius; col < image.n_cols - radius; ++col) {
                std::vector<size_t> valuesR, valuesG, valuesB;
                for (int dr = -radius; dr <= static_cast<int>(radius); ++dr) {
                    for (int dc = -radius; dc <= static_cast<int>(radius); ++dc) {
                        valuesR.push_back(std::get<0>(image(row + dr, col + dc)));
                        valuesG.push_back(std::get<1>(image(row + dr, col + dc)));
                        valuesB.push_back(std::get<2>(image(row + dr, col + dc)));
                    }
                }

                std::nth_element(valuesR.begin(), valuesR.begin() + valuesR.size() / 2, valuesR.end());
                auto valR = valuesR[valuesR.size() / 2];
                std::nth_element(valuesG.begin(), valuesG.begin() + valuesG.size() / 2, valuesG.end());
                auto valG = valuesG[valuesG.size() / 2];
                std::nth_element(valuesB.begin(), valuesB.begin() + valuesB.size() / 2, valuesB.end());
                auto valB = valuesB[valuesB.size() / 2];

                ans(row, col) = std::make_tuple(valR, valG, valB);
            }
        }
        return ans;
    }

private:
    size_t radius;
};

class MedianLinearFilter : public BaseFilterWrapper {
public:
    MedianLinearFilter(size_t radius_) : radius(radius_) {}

    Image applyToImage(const Image& image) const override {
        if (2 * radius + 1 > image.n_rows || 2 * radius + 1 > image.n_cols)
            return image.deep_copy();

        Image ans = image.deep_copy();

        Histogram histR, histG, histB;

        for (size_t row = radius; row < image.n_rows - radius; ++row) {
            for (size_t col = radius; col < image.n_cols - radius; ++col) {
                if (col == radius) {
                    histR.clear();
                    histG.clear();
                    histB.clear();
                    for (int dr = -static_cast<int>(radius); dr <= static_cast<int>(radius); ++dr) {
                        for (int dc = -static_cast<int>(radius); dc <= static_cast<int>(radius); ++dc) {
                            histR.add(std::get<0>(image(row + dr, col + dc)));
                            histG.add(std::get<1>(image(row + dr, col + dc)));
                            histB.add(std::get<2>(image(row + dr, col + dc)));
                        }
                    }
                } else {
                    for (int dr = -static_cast<int>(radius); dr <= static_cast<int>(radius); ++dr) {
                        histR.remove(std::get<0>(image(row + dr, col - radius - 1)));
                        histG.remove(std::get<1>(image(row + dr, col - radius - 1)));
                        histB.remove(std::get<2>(image(row + dr, col - radius - 1)));
                        histR.add(std::get<0>(image(row + dr, col + radius)));
                        histG.add(std::get<1>(image(row + dr, col + radius)));
                        histB.add(std::get<2>(image(row + dr, col + radius)));
                    }
                }
                ans(row, col) = std::make_tuple(histR.findMedian(), histG.findMedian(), histB.findMedian());
            }
        }

        return ans;
    }

private:
    size_t radius;
};

class MedianConstFilter : public BaseFilterWrapper {
public:
    MedianConstFilter(size_t radius_) : radius(radius_) {}

    Image applyToImage(const Image& image) const override {
        if (2 * radius + 1 > image.n_rows || 2 * radius + 1 > image.n_cols)
            return image.deep_copy();

        Image ans = image.deep_copy();

        std::tuple<Histogram, Histogram, Histogram> kernel;
        std::vector<std::tuple<Histogram, Histogram, Histogram>> vertHists(image.n_cols);

        for (size_t col = 0; col < image.n_cols; ++col) {
            for (size_t row = 0; row < 2 * radius + 1; ++row) {
                std::get<0>(vertHists[col]).add(std::get<0>(image(row, col)));
                std::get<1>(vertHists[col]).add(std::get<1>(image(row, col)));
                std::get<2>(vertHists[col]).add(std::get<2>(image(row, col)));
            }
        }

        for (size_t row = radius; row < image.n_rows - radius; ++row) {
            if (row != radius) {
                for (size_t col = 0; col < image.n_cols; ++col) {
                    std::get<0>(vertHists[col]).remove(std::get<0>(image(row - radius - 1, col)));
                    std::get<1>(vertHists[col]).remove(std::get<1>(image(row - radius - 1, col)));
                    std::get<2>(vertHists[col]).remove(std::get<2>(image(row - radius - 1, col)));
                    std::get<0>(vertHists[col]).add(std::get<0>(image(row + radius, col)));
                    std::get<1>(vertHists[col]).add(std::get<1>(image(row + radius, col)));
                    std::get<2>(vertHists[col]).add(std::get<2>(image(row + radius, col)));
                }
            }
            for (size_t col = radius; col < image.n_cols - radius; ++col) {
                if (col == radius) {
                    std::get<0>(kernel).clear();
                    std::get<1>(kernel).clear();
                    std::get<2>(kernel).clear();
                    for (int dc = -radius; dc <= static_cast<int>(radius); ++dc) {
                        std::get<0>(kernel).addHist(std::get<0>(vertHists[col + dc]));
                        std::get<1>(kernel).addHist(std::get<1>(vertHists[col + dc]));
                        std::get<2>(kernel).addHist(std::get<2>(vertHists[col + dc]));
                    }
                } else {
                    std::get<0>(kernel).subHist(std::get<0>(vertHists[col - radius - 1]));
                    std::get<1>(kernel).subHist(std::get<1>(vertHists[col - radius - 1]));
                    std::get<2>(kernel).subHist(std::get<2>(vertHists[col - radius - 1]));
                    std::get<0>(kernel).addHist(std::get<0>(vertHists[col + radius]));
                    std::get<1>(kernel).addHist(std::get<1>(vertHists[col + radius]));
                    std::get<2>(kernel).addHist(std::get<2>(vertHists[col + radius]));
                }
                ans(row, col) = std::make_tuple(std::get<0>(kernel).findMedian(), std::get<1>(kernel).findMedian(), std::get<2>(kernel).findMedian());
            }
        }

        return ans;
    }

private:
    size_t radius;
};

class MedianPlugin : public IFilterPlugin {
public:
    Image applyToImage(const Image& image) const override {
        return impl->applyToImage(image);
    }

    std::string getUserInvitation() const override {
        std::string inv = "";
        if (!typeIsInit) {
            inv = "choose type of implementation:\n"
                  "    [0] simple;\n"
                  "    [1] linear;\n"
                  "    [2] const;";
        } else if (!radiusIsInit) {
            inv = "enter radius:";
        }
        return inv;
    }

    void sendUserOutput(const std::string& s) override {
        std::istringstream iss(s);
        if (!typeIsInit) {
            int t;
            iss >> t;
            type = static_cast<FilterType>(t);
            typeIsInit = true;
        } else {
            iss >> radius;
            radiusIsInit = true;
            switch (type) {
                case SIMPLE:
                    impl = std::make_unique<MedianSimpleFilter>(radius);
                    break;
                case LINEAR:
                    impl = std::make_unique<MedianLinearFilter>(radius);
                    break;
                case CONST:
                    impl = std::make_unique<MedianConstFilter>(radius);
                    break;
                default:
                    assert(false);
            }
        }
    }

    std::string getName() const override {
        return "median";
    }

private:
    bool typeIsInit = false;
    bool radiusIsInit = false;
    std::unique_ptr<BaseFilterWrapper> impl;

    enum FilterType {
        SIMPLE, LINEAR, CONST
    };

    FilterType type;
    size_t radius;
};


class MedianFactory : public IFilterPluginFactory {
public:
    IFilterPlugin* createObject() override {
        return new MedianPlugin;
    }
};

extern "C" void registerPlugin(FilterPluginManager& manager) {
    MedianFactory factory;
    manager.registerPlugin(factory);
}
