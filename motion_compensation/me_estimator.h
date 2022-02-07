#pragma once
#include "me_field.h"

#include <array>
#include <cassert>
#include <limits>
#include <memory>
#include <optional>

#if 1
#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
#else
#include "C:\Users\1\AppData\Local\Programs\Python\Python39\Lib\site-packages\pybind11\include\pybind11\pybind11.h"
#include "C:\Users\1\AppData\Local\Programs\Python\Python39\Lib\site-packages\pybind11\include\pybind11\numpy.h"
#endif

#define pow_2(v)    ((v) * (v))

namespace py = pybind11;

class MotionEstimator {
public:
    /// Constructor
    MotionEstimator(size_t width, size_t height, unsigned char quality, bool use_half_pixel);

    /// Destructor
    ~MotionEstimator();

    /// Copy constructor (deleted)
    MotionEstimator(const MotionEstimator&) = delete;

    /// Move constructor
    MotionEstimator(MotionEstimator&&) = default;

    /// Copy assignment (deleted)
    MotionEstimator& operator=(const MotionEstimator&) = delete;

    /// Move assignment
    MotionEstimator& operator=(MotionEstimator&&) = default;

    /**
     * Estimate motion between two frames
     *
     * @param[in] cur_Y numpy array (uint8) of pixels of the current frame
     * @param[in] prev_Y numpy array (uint8) of pixels of the previous frame
     * @param[out] MEFielf structure of motion vectors for frame
     */
    MEField Estimate(
            py::array_t<unsigned char> cur_Y,
            py::array_t<unsigned char> prev_Y
    );

    std::pair<int, int> inline elem(MEField& mvectors, size_t i, size_t j) const;
    std::pair<int, int> inline elem_v(std::vector<MV> &q, size_t i, size_t j) const;
    std::pair<int, int> inline medium_elem_5x5(MEField &q, size_t i, size_t j) const;
    std::pair<int, int> inline medium_elem_3x3(MEField &q, size_t i, size_t j) const;

    /**
     * Size of the borders added to frames by the template, in pixels.
     * This is the most pixels your motion vectors can extend past the image border.
     */
    static constexpr int BORDER = 16;

    /// Size of a block covered by a motion vector. Do not change.
    static constexpr size_t BLOCK_SIZE = 2;

private:
    /// Frame width (not including borders)
    const size_t width;

    /// Frame height (not including borders)
    const size_t height;

    /// Quality
    const unsigned char quality, q_threshold, q_diap;

    /// Whether to use half-pixel precision
    const bool use_half_pixel;

    /// Extended frame width (including borders)
    const size_t width_ext;

    /// Number of blocks per X-axis
    const size_t num_blocks_hor;

    /// Number of blocks per Y-axis
    const size_t num_blocks_vert;

    /// Position of the first pixel of the frame in the extended frame
    const size_t first_row_offset;

    /// Me field
    MEField me_field, me_field2, me_field3;

    /// width + 2 * BORDER
    size_t width_borders;
    /// height + 2 * BORDER
    size_t height_borders;

    /// Buffers for subpixel frames
    unsigned char* prev_Y_up_borders;
    unsigned char* prev_Y_up_left_borders;
    unsigned char* prev_Y_left_borders;

    /// Buffers for frames
    unsigned char* cur_Y_borders;
    unsigned char* prev_Y_borders;

    int n_frame;

    /**
     * Estimate motion between two frames
     *
     * @param[in] cur_Y array of pixels of the current frame
     * @param[in] prev_Y array of pixels of the previous frame
     * @param[in] prev_Y_up array of pixels of the previous frame shifted half a pixel up,
     *   only valid if use_half_pixel is true
     * @param[in] prev_Y_left array of pixels of the previous frame shifted half a pixel left,
     *   only valid if use_half_pixel is true
     * @param[in] prev_Y_upleft array of pixels of the previous frame shifted half a pixel up left,
     *   only valid if use_half_pixel is true
     * @param[out] mvectors output array of motion vectors
     */
    void CEstimate(const unsigned char* cur_Y,
                   const unsigned char* prev_Y,
                   const unsigned char* prev_Y_up,
                   const unsigned char* prev_Y_left,
                   const unsigned char* prev_Y_upleft,
                   MEField &mvectors,
                   MEField &mvectors2,
                   MEField &mvectors3) const;
};

const std::pair<int, int> spiral_4[] = {std::pair<int, int> {-4, -4}, std::pair<int, int> {-4, -3}, std::pair<int, int> {-4, -2}, std::pair<int, int> {-4, -1}, std::pair<int, int> {-4, 0}, std::pair<int, int> {-4, 1}, std::pair<int, int> {-4, 2}, std::pair<int, int> {-4, 3}, std::pair<int, int> {-4, 4}, std::pair<int, int> {-3, -4}, std::pair<int, int> {-3, -3}, std::pair<int, int> {-3, -2}, std::pair<int, int> {-3, -1}, std::pair<int, int> {-3, 0}, std::pair<int, int> {-3, 1}, std::pair<int, int> {-3, 2}, std::pair<int, int> {-3, 3}, std::pair<int, int> {-3, 4}, std::pair<int, int> {-2, -4}, std::pair<int, int> {-2, -3}, std::pair<int, int> {-2, -2}, std::pair<int, int> {-2, -1}, std::pair<int, int> {-2, 0}, std::pair<int, int> {-2, 1}, std::pair<int, int> {-2, 2}, std::pair<int, int> {-2, 3}, std::pair<int, int> {-2, 4}, std::pair<int, int> {-1, -4}, std::pair<int, int> {-1, -3}, std::pair<int, int> {-1, -2}, std::pair<int, int> {-1, -1}, std::pair<int, int> {-1, 0}, std::pair<int, int> {-1, 1}, std::pair<int, int> {-1, 2}, std::pair<int, int> {-1, 3}, std::pair<int, int> {-1, 4}, std::pair<int, int> {0, -4}, std::pair<int, int> {0, -3}, std::pair<int, int> {0, -2}, std::pair<int, int> {0, -1}, std::pair<int, int> {0, 0}, std::pair<int, int> {0, 1}, std::pair<int, int> {0, 2}, std::pair<int, int> {0, 3}, std::pair<int, int> {0, 4}, std::pair<int, int> {1, -4}, std::pair<int, int> {1, -3}, std::pair<int, int> {1, -2}, std::pair<int, int> {1, -1}, std::pair<int, int> {1, 0}, std::pair<int, int> {1, 1}, std::pair<int, int> {1, 2}, std::pair<int, int> {1, 3}, std::pair<int, int> {1, 4}, std::pair<int, int> {2, -4}, std::pair<int, int> {2, -3}, std::pair<int, int> {2, -2}, std::pair<int, int> {2, -1}, std::pair<int, int> {2, 0}, std::pair<int, int> {2, 1}, std::pair<int, int> {2, 2}, std::pair<int, int> {2, 3}, std::pair<int, int> {2, 4}, std::pair<int, int> {3, -4}, std::pair<int, int> {3, -3}, std::pair<int, int> {3, -2}, std::pair<int, int> {3, -1}, std::pair<int, int> {3, 0}, std::pair<int, int> {3, 1}, std::pair<int, int> {3, 2}, std::pair<int, int> {3, 3}, std::pair<int, int> {3, 4}, std::pair<int, int> {4, -4}, std::pair<int, int> {4, -3}, std::pair<int, int> {4, -2}, std::pair<int, int> {4, -1}, std::pair<int, int> {4, 0}, std::pair<int, int> {4, 1}, std::pair<int, int> {4, 2}, std::pair<int, int> {4, 3}, std::pair<int, int> {4, 4}};
const std::pair<int, int> spiral_2[] = {std::pair<int, int> {-2, -2}, std::pair<int, int> {-2, -1}, std::pair<int, int> {-2, 0}, std::pair<int, int> {-2, 1}, std::pair<int, int> {-2, 2}, std::pair<int, int> {-1, -2}, std::pair<int, int> {-1, -1}, std::pair<int, int> {-1, 0}, std::pair<int, int> {-1, 1}, std::pair<int, int> {-1, 2}, std::pair<int, int> {0, -2}, std::pair<int, int> {0, -1}, std::pair<int, int> {0, 0}, std::pair<int, int> {0, 1}, std::pair<int, int> {0, 2}, std::pair<int, int> {1, -2}, std::pair<int, int> {1, -1}, std::pair<int, int> {1, 0}, std::pair<int, int> {1, 1}, std::pair<int, int> {1, 2}, std::pair<int, int> {2, -2}, std::pair<int, int> {2, -1}, std::pair<int, int> {2, 0}, std::pair<int, int> {2, 1}, std::pair<int, int> {2, 2}};

int inline GetErrorSAD_8x8_my(const unsigned char* block1, const unsigned char *block2, const int stride);
int inline GetErrorSAD_4x4_my(const unsigned char* block1, const unsigned char *block2, const int stride);
int inline GetErrorSAD_2x2_my(const unsigned char *block1, const unsigned char *block2, const int stride);
int inline GetErrorSAD_1x1_my(const unsigned char *block1, const unsigned char *block2, const int stride);