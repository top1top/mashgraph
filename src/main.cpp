#include "mvc/model.h"
#include "mvc/console_controller.h"

#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include <initializer_list>
#include <limits>

using std::string;
using std::stringstream;
using std::cout;
using std::cerr;
using std::endl;
using std::numeric_limits;

#include "align.h"

template<typename ValueType>
ValueType read_value(string s)
{
    stringstream ss(s);
    ValueType res;
    ss >> res;
    if (ss.fail() or not ss.eof())
        throw string("bad argument: ") + s;
    return res;
}

template<typename ValueType>
bool check_value(string s)
{
    stringstream ss(s);
    ValueType res;
    ss >> res;
    if (ss.fail() or not ss.eof())
        return false;
    return true;
}

template<typename ValueT>
void check_number(string val_name, ValueT val, ValueT from,
                  ValueT to=numeric_limits<ValueT>::max())
{
    if (val < from)
        throw val_name + string(" is too small");
    if (val > to)
        throw val_name + string(" is too big");
}

void check_argc(int argc, int from, int to=numeric_limits<int>::max())
{
    if (argc < from)
        throw string("too few arguments for operation");

    if (argc > to)
        throw string("too many arguments for operation");
}

Matrix<double> parse_kernel(string kernel)
{
    // Kernel parsing implementation here
    return Matrix<double>(0, 0);
}

void parse_args(char **argv, int argc, bool *isPostprocessing, string *postprocessingType, double *fraction, bool *isMirror,
            bool *isInterp, bool *isSubpixel, double *subScale)
{
    for (int i = 4; i < argc; i++) {
        string param(argv[i]);

        if (param == "--gray-world" || param == "--unsharp" ||
        param == "--white-balance" || param == "--autocontrast") {
            *isPostprocessing = true;
            *postprocessingType = param;
            if ((param == "--autocontrast") && ((i+1) < argc) && check_value<double>(argv[i+1])) {
                *fraction = read_value<double>(argv[++i]);
            }
        } else if (param == "--subpixel") {
            *isSubpixel = true;
            if (((i+1) < argc) && check_value<double>(argv[i+1])) {
                *subScale = read_value<double>(argv[++i]);
            }
        } else if (param == "--bicubic-interp") {
            *isInterp = true;
        } else if (param == "--mirror") {
            *isMirror = true;
        }else
            throw string("unknown option for --align ") + param;
    }
}

int main(int argc, char **argv)
{
    Model model;
    ConsoleController consoleController(&model);
    consoleController.run(argc, argv);
    return 0;
/*
    try {
        check_argc(argc, 2);
        if (string(argv[1]) == "--help") {
            print_help(argv[0]);
            return 0;
        }

        check_argc(argc, 4);
        Image src_image = load_image(argv[1]), dst_image;

        string action(argv[3]);

        if (action == "--sobel-x") {
            check_argc(argc, 4, 4);
            dst_image = sobel_x(src_image);
        } else if (action == "--sobel-y") {
            check_argc(argc, 4, 4);
            dst_image = sobel_y(src_image);
        } else if (action == "--unsharp") {
            check_argc(argc, 4, 4);
            dst_image = unsharp(src_image);
        } else if (action == "--gray-world") {
            check_argc(argc, 4, 4);
            dst_image = gray_world(src_image);
        } else if (action == "--resize") {
            check_argc(argc, 5, 5);
            double scale = read_value<double>(argv[4]);
            dst_image = resize(src_image, scale);
        } else if (action == "--resize-bicubic") {
            check_argc(argc, 5, 5);
            double scale = read_value<double>(argv[4]);
            dst_image = bicubicResize(src_image, scale);
        } else if (action == "--custom") {
            check_argc(argc, 5, 5);
            Matrix<double> kernel = parse_kernel(argv[4]);
            dst_image = custom(src_image, kernel);
        } else if (action == "--autocontrast") {
            check_argc(argc, 4, 5);
            double fraction = 0.0;
            if (argc == 5) {
                fraction = read_value<double>(argv[4]);
                check_number("fraction", fraction, 0.0, 0.4);
            }
            dst_image = autocontrast(src_image, fraction);
        } else if (action == "--gaussian" || action == "--gaussian-separable") {
            check_argc(argc, 5, 6);
            double sigma = read_value<double>(argv[4]);
            check_number("sigma", sigma, 0.1, 100.0);
            int radius = 3 * sigma;
            if (argc == 6) {
                radius = read_value<int>(argv[5]);
                check_number("radius", radius, 1);
            }
            if (action == "--gaussian") {
                dst_image = gaussian(src_image, sigma, radius);
            } else {
                dst_image = gaussian_separable(src_image, sigma, radius);
            }
        } else if (action == "--canny") {
            check_argc(6, 6);
            int threshold1 = read_value<int>(argv[4]);
            check_number("threshold1", threshold1, 0, 360);
            int threshold2 = read_value<int>(argv[5]);
            check_number("threshold2", threshold2, 0, 360);
            if (threshold1 >= threshold2)
                throw string("threshold1 must be less than threshold2");
            dst_image = canny(src_image, threshold1, threshold2);
        } else if (action == "--median" || action == "--median-linear" ||
                    action == "--median-const") {
            check_argc(argc, 4, 5);
            int radius = 1;
            if (argc == 5) {
                radius = read_value<int>(argv[4]);
                check_number("radius", radius, 1);
            }
            if (action == "--median") {
                dst_image = median(src_image, radius);
            } else if (action == "--median-linear") {
                dst_image = median_linear(src_image, radius);
            } else {
                dst_image = median_const(src_image, radius);
            }
        } else if (action == "--align") {
            bool isPostprocessing = false, isInterp = false,
                isSubpixel = false, isMirror = false;

            string postprocessingType;

            double fraction = 0.0, subScale = 2.0;

            if (argc >= 5) {
                parse_args(argv, argc, &isPostprocessing, &postprocessingType, &fraction, &isMirror,
                    &isInterp, &isSubpixel, &subScale);
            }

            dst_image = align(src_image, isPostprocessing, postprocessingType, fraction, isMirror,
                isInterp, isSubpixel, subScale);
        } else {
            throw string("unknown action ") + action;
        }
        save_image(dst_image, argv[2]);
    } catch (const string &s) {
        cerr << "Error: " << s << endl;
        cerr << "For help type: " << endl << argv[0] << " --help" << endl;
        return 1;
    }
*/
}
