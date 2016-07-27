//
// Created by centos on 7/27/16.
//

#ifndef TENSORFLOWANDROID_YUV2RGB_H
#define TENSORFLOWANDROID_YUV2RGB_H

#include "tensorflow/core/platform/types.h"

using namespace tensorflow;

#ifdef __cplusplus
extern "C" {
#endif

void ConvertYUV420ToARGB8888(const uint8* const yData, const uint8* const uData,
                             const uint8* const vData, uint32* const output,
                             const int width, const int height,
                             const int y_row_stride, const int uv_row_stride,
                             const int uv_pixel_stride);

// Converts YUV420 semi-planar data to ARGB 8888 data using the supplied width
// and height. The input and output must already be allocated and non-null.
// For efficiency, no error checking is performed.
void ConvertYUV420SPToARGB8888(const uint8* const pY, const uint8* const pUV,
                               uint32* const output, const int width,
                               const int height);

// The same as above, but downsamples each dimension to half size.
void ConvertYUV420SPToARGB8888HalfSize(const uint8* const input,
                                       uint32* const output,
                                       int width, int height);

// Converts YUV420 semi-planar data to RGB 565 data using the supplied width
// and height. The input and output must already be allocated and non-null.
// For efficiency, no error checking is performed.
void ConvertYUV420SPToRGB565(const uint8* const input, uint16* const output,
                             const int width, const int height);

#ifdef __cplusplus
}

#endif
#endif // end TENSORFLOWANDROID_YUV2RGB_H
