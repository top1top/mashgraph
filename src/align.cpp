#include "align.h"
#include "align_help.h"
#include "filters.h"

#include <string>
#include <vector>
#include <algorithm>
#include <cassert>
#include <numeric>
#include <initializer_list>
#include <stdexcept>
#include <cmath>
#include <queue>

using std::string;
using std::cout;
using std::endl;

Image gray_world(Image src_image) {
    unsigned long long sumR = 0, sumG = 0, sumB = 0;
    for (size_t row = 0; row < src_image.n_rows; ++row) {
        for (size_t col = 0; col < src_image.n_cols; ++col) {
            sumR += std::get<0>(src_image(row, col));
            sumG += std::get<1>(src_image(row, col));
            sumB += std::get<2>(src_image(row, col));
        }
    }

    long double mR = static_cast<long double>(sumR) / (src_image.n_rows * src_image.n_cols);
    long double mG = static_cast<long double>(sumG) / (src_image.n_rows * src_image.n_cols);
    long double mB = static_cast<long double>(sumB) / (src_image.n_rows * src_image.n_cols);
    long double middle = (mR + mG + mB) / 3;

    long double kR = middle / mR;
    long double kG = middle / mG;
    long double kB = middle / mB;

    Image ans(src_image.n_rows, src_image.n_cols);

    for (size_t row = 0; row < src_image.n_rows; ++row) {
        for (size_t col = 0; col < src_image.n_cols; ++col) {
            std::get<0>(ans(row, col)) = std::min(static_cast<size_t>(round(std::get<0>(src_image(row, col)) * kR)), 255ul);
            std::get<1>(ans(row, col)) = std::min(static_cast<size_t>(round(std::get<1>(src_image(row, col)) * kG)), 255ul);
            std::get<2>(ans(row, col)) = std::min(static_cast<size_t>(round(std::get<2>(src_image(row, col)) * kB)), 255ul);
        }
    }

    return ans;
}

Image bicubicResize(Image srcImage, double scale) {
    Image resImage(srcImage.n_rows * scale, srcImage.n_cols * scale);

    for (size_t row = 0; row < resImage.n_rows; ++row) {
        for (size_t col = 0; col < resImage.n_cols; ++col) {
            double srcRow = 1.0 * row / scale;
            double srcCol = 1.0 * col / scale;

            size_t i1, j1;
            i1 = std::max(static_cast<int>(floor(srcRow)), 0);
            if (i1 > srcImage.n_rows - 4)
                i1 = srcImage.n_rows - 4;
            j1 = std::max(static_cast<int>(floor(srcCol)), 0);
            if (j1 > srcImage.n_cols - 4)
                j1 = srcImage.n_cols - 4;

            double y = srcRow - i1;
            double x = srcCol - j1;

            std::tuple<uint, uint, uint> q[16];
            double k[16];

            k[0] = 0.25 * (x - 1) * (x - 2) * (x + 1) * (y - 1) * (y - 2) * (y + 1);
            k[1] = -0.25 * x * (x + 1) * (x - 2) * (y - 1) * (y - 2) * (y + 1);
            k[2] = -0.25 * y * (x - 1) * (x - 2) * (x + 1) * (y + 1) * (y - 2);
            k[3] = 0.25 * x * y * (x + 1) * (x - 2) * (y + 1) * (y - 2);
            k[4] = -1.0 / 12 * x * (x - 1) * (x - 2) * (y - 1) * (y - 2) * (y + 1);
            k[5] = -1.0 / 12 * y * (x - 1) * (x - 2) * (x + 1) * (y - 1) * (y - 2);
            k[6] = 1.0 / 12 * x * y * (x - 1) * (x - 2) * (y + 1) * (y - 2);
            k[7] = 1.0 / 12 * x * y * (x + 1) * (x - 2) * (y - 1) * (y - 2);
            k[8] = 1.0 / 12 * x * (x - 1) * (x + 1) * (y - 1) * (y - 2) * (y + 1);
            k[9] = 1.0 / 12 * y * (x - 1) * (x - 2) * (x + 1) * (y - 1) * (y + 1);
            k[10] = 1.0 / 36 * x * y * (x - 1) * (x - 2) * (y - 1) * (y - 2);
            k[11] = -1.0 / 12 * x * y * (x - 1) * (x + 1) * (y + 1) * (y - 2);
            k[12] = -1.0 / 12 * x * y * (x + 1) * (x - 2) * (y - 1) * (y + 1);
            k[13] = -1.0 / 36 * x * y * (x - 1) * (x + 1) * (y - 1) * (y - 2);
            k[14] = -1.0 / 36 * x * y * (x - 1) * (x - 2) * (y - 1) * (y + 1);
            k[15] = 1.0 / 36 * x * y * (x - 1) * (x + 1) * (y - 1) * (y + 1);

            q[0] = srcImage(i1 + 1, j1 + 1);
            q[1] = srcImage(i1 + 1, j1 + 2);
            q[2] = srcImage(i1 + 2, j1 + 1);
            q[3] = srcImage(i1 + 2, j1 + 2);
            q[4] = srcImage(i1 + 1, j1);
            q[5] = srcImage(i1, j1 + 1);
            q[6] = srcImage(i1 + 2, j1);
            q[7] = srcImage(i1, j1 + 2);
            q[8] = srcImage(i1 + 1, j1 + 3);
            q[9] = srcImage(i1 + 3, j1);
            q[10] = srcImage(i1, j1);
            q[11] = srcImage(i1 + 2, j1 + 3);
            q[12] = srcImage(i1 + 3, j1 + 2);
            q[13] = srcImage(i1, j1 + 3);
            q[14] = srcImage(i1 + 3, j1);
            q[15] = srcImage(i1 + 3, j1 + 3);

            double valR = 0, valG = 0, valB = 0;
            for (size_t i = 0; i < 16; ++i) {
                valR += std::get<0>(q[i]) * k[i];
                valG += std::get<1>(q[i]) * k[i];
                valB += std::get<2>(q[i]) * k[i];
            }

            resImage(row, col) = std::make_tuple(normalizeRes(valR), normalizeRes(valG), normalizeRes(valB));
        }
    }

    return resImage;
}

Image resize(Image srcImage, double scale) {
    Image resImage(srcImage.n_rows * scale, srcImage.n_cols * scale);

    for (size_t row = 0; row < resImage.n_rows; ++row) {
        for (size_t col = 0; col < resImage.n_cols; ++col) {
            double srcRow = 1.0 * row / scale;
            double srcCol = 1.0 * col / scale;

            size_t i1, j1;
            i1 = std::max(static_cast<int>(floor(srcRow)), 0);
            if (i1 > srcImage.n_rows - 2)
                i1 = srcImage.n_rows - 2;
            j1 = std::max(static_cast<int>(floor(srcCol)), 0);
            if (j1 > srcImage.n_cols - 2)
                j1 = srcImage.n_cols - 2;

            std::tuple<uint, uint, uint> q11, q21, q22, q12;
            q11 = srcImage(i1, j1);
            q21 = srcImage(i1 + 1, j1);
            q12 = srcImage(i1, j1 + 1);
            q22 = srcImage(i1 + 1, j1 + 1);

            double k11 = (i1 + 1 - srcRow) * (j1 + 1 - srcCol);
            double k21 = (srcRow - i1) * (j1 + 1 - srcCol);
            double k12 = (i1 + 1 - srcRow) * (srcCol - j1);
            double k22 = (srcRow - i1) * (srcCol - j1);

            resImage(row, col) =
                std::make_tuple(normalizeRes(std::get<0>(q11) * k11 + std::get<0>(q21) * k21 + std::get<0>(q12) * k12 + std::get<0>(q22) * k22),
                                normalizeRes(std::get<1>(q11) * k11 + std::get<1>(q21) * k21 + std::get<1>(q12) * k12 + std::get<1>(q22) * k22),
                                normalizeRes(std::get<2>(q11) * k11 + std::get<2>(q21) * k21 + std::get<2>(q12) * k12 + std::get<2>(q22) * k22));
        }
    }

    return resImage;
}

static bool isNoMax(const Matrix<double>& gradLength, const Matrix<double>& gradDirection, size_t row, size_t col) {
    auto dir = gradDirection(row, col);
    auto len = gradLength(row, col);
    if (dir < 0)
        dir += M_PI;
    static const std::pair<int, int> dr[5] = {{0, 1}, {-1, 1}, {-1, 0}, {-1, -1}, {0, -1}};
    double stepAngle = M_PI / 4;

    auto isGreater = [&gradLength, &len] (int nrow, int ncol) {
        static const double eps = 0.000000001;
        if (nrow < 0 || nrow >= static_cast<int>(gradLength.n_rows) || ncol < 0 || ncol >= static_cast<int>(gradLength.n_cols))
            return false;
        return gradLength(nrow, ncol) > len - eps;
    };

    size_t id = static_cast<size_t>(dir / stepAngle);
    if (id > 4)
        id = 4;

    return isGreater(static_cast<int>(row) + dr[id].first, static_cast<int>(col) + dr[id].second) ||
                isGreater(static_cast<int>(row) - dr[id].first, static_cast<int>(col) - dr[id].second);
}

static void bfs(std::queue<std::pair<size_t, size_t>> q, Matrix<int>& mp) {
    while (!q.empty()) {
        size_t row = q.front().first;
        size_t col = q.front().second;
        q.pop();
        for (int drow = -1; drow <= 1; ++drow) {
            for (int dcol = -1; dcol <= 1; ++dcol) {
                if (drow == 0 && dcol == 0)
                    continue;
                int nrow = static_cast<int>(row) + drow;
                int ncol = static_cast<int>(col) + dcol;
                if (nrow >= 0 && nrow < static_cast<int>(mp.n_rows) && ncol >= 0 && ncol < static_cast<int>(mp.n_cols) && mp(nrow, ncol) == 1) {
                    mp(nrow, ncol) = 2;
                    q.push({nrow, ncol});
                }
            }
        }
    }
}

Image canny(Image src_image, int threshold1, int threshold2) {
    Image bluringImage = GaussFilter(2, 1.4).applyToImage(src_image);

    Image derivativeX = SobelKernelX().applyToImage(bluringImage);

    Image derivativeY = SobelKernelY().applyToImage(bluringImage);

    if (derivativeX.n_rows != derivativeY.n_rows || derivativeX.n_cols != derivativeY.n_cols)
        throw std::logic_error("non correct size image after unary_map function call");

    Matrix<double> gradLength(src_image.n_rows, src_image.n_cols), gradDirection(src_image.n_rows, src_image.n_cols);
    for (size_t row = 0; row < derivativeX.n_rows; ++row) {
        for (size_t col = 0; col < derivativeX.n_cols; ++col) {
            auto dx = std::get<0>(derivativeX(row, col));
            auto dy = std::get<0>(derivativeY(row, col));
            gradLength(row, col) = sqrt(dx * dx + dy * dy);
            gradDirection(row, col) = atan2(dy, dx);
        }
    }

    Matrix<int> mp(src_image.n_rows, src_image.n_cols);

    for (size_t row = 0; row < src_image.n_rows; ++row) {
        for (size_t col = 0; col < src_image.n_cols; ++col) {
            if (isNoMax(gradLength, gradDirection, row, col)) {
                mp(row, col) = 0;
            } else {
                if (gradLength(row, col) < threshold1)
                    mp(row, col) = 0;
                else if (gradLength(row, col) >= threshold1 && gradLength(row, col) <= threshold2)
                    mp(row, col) = 1;
                else
                    mp(row, col) = 2;
            }
        }
    }

    std::queue<std::pair<size_t, size_t>> q;
    for (size_t row = 0; row < src_image.n_rows; ++row) {
        for (size_t col = 0; col < src_image.n_cols; ++col) {
            if (mp(row, col) == 2)
                q.push({row, col});
        }
    }

    bfs(q, mp);

    Image borderImage(src_image.n_rows, src_image.n_cols);
    for (size_t row = 0; row < src_image.n_rows; ++row) {
        for (size_t col = 0; col < src_image.n_cols; ++col) {
            if (mp(row, col) == 2)
                borderImage(row, col) = std::make_tuple(255, 255, 255);
            else
                borderImage(row, col) = std::make_tuple(0, 0, 0);
        }
    }

    return borderImage;
}
