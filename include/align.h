#pragma once

#include "io.h"
#include "matrix.h"

Image gray_world(Image src_image);

Image resize(Image src_image, double scale);

Image bicubicResize(Image srcImage, double scale);

Image autocontrast(Image src_image, double fraction);

Image canny(Image src_image, int threshold1, int threshold2);
