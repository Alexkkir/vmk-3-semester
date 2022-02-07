#include "me_estimator.h"
#include "metric.h"
#include <iostream>
#include <stdio.h>


MotionEstimator::MotionEstimator(size_t width, size_t height, unsigned char quality, bool use_half_pixel) :
        width(width),
        height(height),
        quality(quality),
        use_half_pixel(use_half_pixel),
        width_ext(width + 2 * BORDER),
        num_blocks_hor((width + BLOCK_SIZE - 1) / BLOCK_SIZE),
        num_blocks_vert((height + BLOCK_SIZE - 1) / BLOCK_SIZE),
        first_row_offset(width_ext * BORDER + BORDER),
        width_borders(width + 2 * BORDER),
        height_borders(height + 2 * BORDER),
        n_frame(0),
        me_field(num_blocks_hor, num_blocks_vert, BLOCK_SIZE),
        me_field2(num_blocks_hor, num_blocks_vert, BLOCK_SIZE),
        me_field3(num_blocks_hor, num_blocks_vert, BLOCK_SIZE) {
    cur_Y_borders = new unsigned char[width_borders * height_borders];
    prev_Y_borders = new unsigned char[width_borders * height_borders];

    if (use_half_pixel) {
        prev_Y_up_borders = new unsigned char[width_borders * height_borders];
        prev_Y_up_left_borders = new unsigned char[width_borders * height_borders];
        prev_Y_left_borders = new unsigned char[width_borders * height_borders];
    }
    // PUT YOUR CODE HERE
}


MotionEstimator::~MotionEstimator() {
    delete[] cur_Y_borders;
    delete[] prev_Y_borders;
    if (use_half_pixel) {
        delete[] prev_Y_up_borders;
        delete[] prev_Y_up_left_borders;
        delete[] prev_Y_left_borders;
    }
    // PUT YOUR CODE HERE
}

std::pair<int, int> inline MotionEstimator::elem(MEField &q, size_t i, size_t j) const {
    if (0 <= i && i < num_blocks_vert &&
        0 <= j && j < num_blocks_hor) {
        MV v = q.get_mv(j, i);
        return std::pair<int, int>{v.x, v.y};
    } else {
        return std::pair<int, int>{0, 0};
    }
}

std::pair<int, int> inline MotionEstimator::elem_v(std::vector<MV> &q, size_t i, size_t j) const {
    if (0 <= i && i < num_blocks_vert &&
        0 <= j && j < num_blocks_hor) {
        MV v = q[i * num_blocks_hor + j];
        return std::pair<int, int>{v.x, v.y};
    } else {
        return std::pair<int, int>{0, 0};
    }
}

enum {
    BLOCK_1 = 8,
};

void MotionEstimator::CEstimate(const unsigned char *cur_Y,
                                const unsigned char *prev_Y,
                                const uint8_t *prev_Y_up,
                                const uint8_t *prev_Y_left,
                                const uint8_t *prev_Y_upleft,
                                MEField &mvectors,
                                MEField &mvectors2,
                                MEField &mvectors3) const {
    std::unordered_map<ShiftDir, const uint8_t *, ShiftDirHash> prev_map{
            {ShiftDir::NONE, prev_Y}
    };

    if (use_half_pixel) {
        prev_map.emplace(ShiftDir::UP, prev_Y_up);
        prev_map.emplace(ShiftDir::LEFT, prev_Y_left);
        prev_map.emplace(ShiftDir::UPLEFT, prev_Y_upleft);
    }
    for (size_t i = 0; i < num_blocks_vert; i++) {
        for (size_t j = 0; j < num_blocks_hor; j += 1) {
            const auto hor_offset = j * BLOCK_SIZE;
            const auto vert_offset = first_row_offset + i * BLOCK_SIZE * width_ext;
            const auto cur = cur_Y + vert_offset + hor_offset;

            MV best_vector;
            best_vector.error = std::numeric_limits<long>::max();

            int err_0 = GetErrorSAD_8x8_my(cur, prev_Y + vert_offset + hor_offset, width_ext);
            if (err_0 < 30 * BLOCK_SIZE * BLOCK_SIZE) {
                best_vector.x = 0;
                best_vector.y = 0;
                best_vector.shift_dir = ShiftDir::NONE;
                best_vector.error = err_0;
                mvectors.set_mv(j, i, best_vector);
                continue;
            }

//            if (err_0 / v.error > 2) {
//                int x_fr = v.x, y_fr = v.y;
//                long err_1 = GetErrorSAD_8x8_my(cur, prev_Y + y_fr * width_ext + x_fr, width_ext);
//                if (err_1 < 100 && err_0 / err_1 > 2) {
//                    best_vector.x = x_fr;
//                    best_vector.y = y_fr;
//                    best_vector.error = err_1;
//                }
//            }

            /// Brute
            for (const auto &prev_pair : prev_map) {
                const auto prev = prev_pair.second + vert_offset + hor_offset;
                int x_init = 0, y_init = 0;
                int diap = 12, step = 4, finished = 0;
                for (int y = y_init - diap; !finished && y <= y_init + diap; y += step) {
                    for (int x = x_init - diap; !finished && x <= x_init + diap; x += step) {
                        const auto comp = prev + y * width_ext + x;
                        const int error = GetErrorSAD_8x8_my(cur, comp, width_ext);
                        if (error < best_vector.error) {
                            best_vector.x = x;
                            best_vector.y = y;
                            best_vector.shift_dir = ShiftDir::NONE;
                            best_vector.error = error;
                            finished = (error < 0 * BLOCK_SIZE * BLOCK_SIZE);
                        }
                    }
                }
            }

            int x_init = (best_vector.x), y_init = (best_vector.y);
            int diap_2 = 3, step_2 = 6;
            int finished = 0;
            /// Search for bigger accuracy
            for (const auto& prev_pair : prev_map) {
                const auto prev = prev_pair.second + vert_offset + hor_offset;
                for (int y_init_2 = y_init - diap_2, tmp = 0; y_init_2 <= y_init + diap_2; y_init_2 += step_2, tmp++) {
                    for (int x_init_2 = x_init - diap_2 + diap_2 * (tmp & 1); x_init_2 <= x_init + diap_2; x_init_2 += step_2) {
                        int bias = 2, x_prev = x_init_2, y_prev = y_init_2, x_new = 0, y_new = 0;
                        while (bias != 0) {
                            for (int y = y_prev - bias; y <= y_prev + bias; y += bias) {
                                for (int x = x_prev - bias; x <= x_prev + bias; x += bias) {
                                    const auto comp = prev + y * width_ext + x;
                                    const int error = GetErrorSAD_8x8_my(cur, comp, width_ext);
                                    if (error < best_vector.error) {
                                        best_vector.x = x;
                                        best_vector.y = y;
                                        best_vector.shift_dir = prev_pair.first;
                                        best_vector.error = error;
                                        x_new = x, y_new = y;
                                    }
                                }
                            }
                            x_prev = x_new;
                            y_prev = y_new;
                            bias >>= 1;
                        }
                    }
                }
            }


//            if (n_frame >= 5 && 8<= i && i <= num_blocks_vert - 8 && 8 <= j && j <= num_blocks_hor - 8) {
//                int x_vel = mvectors2.get_mv(j, i).x, y_prevfr = mvectors2.get_mv(j, i).y;
//                MV v = mvectors.get_mv(j + y_prevfr, i + x_vel);
//                int x_fr = v.x, y_fr = v.y;
//                if (abs(x_fr) <= 10 && abs(y_fr) <= 10) {
//                    long err_1 = GetErrorSAD_8x8_my(cur, prev_Y + vert_offset + hor_offset + y_fr * width_ext + x_fr,
//                                                    width_ext);
//                    printf("best, zero, vel + mot: %ld\t%d\t%ld\t|||\t%d\t%d\t|%d\t%d\n",
//                           best_vector.error / 64,
//                           err_0 / 64,
//                           err_1 / 64,
//                           best_vector.x,
//                           best_vector.y,
//                           v.x,
//                           v.y
//                    );
//                }
//            }

            /// end
            mvectors.set_mv(j, i, best_vector);
        }
    }
//    for (size_t i = 0; i < num_blocks_vert; i++) {
//        for (size_t j = (i + 1) & 1; j < num_blocks_hor; j += 2) {
//            const auto hor_offset = j * BLOCK_SIZE;
//            const auto vert_offset = first_row_offset + i * BLOCK_SIZE * width_ext;
//            const auto cur = cur_Y + vert_offset + hor_offset;
//
//            MV best_vector;
//            best_vector.error = std::numeric_limits<long>::max();
//
//            std::pair<int, int>
//                    p1 = elem(mvectors, i - 1, j),
//                    p2 = elem(mvectors, i + 1, j),
//                    p3 = elem(mvectors, i, j - 1),
//                    p4 = elem(mvectors, i, j + 1);
//            int prev_x =
//                    p1.first +
//                    p2.first +
//                    p3.first +
//                    p4.first;
//            prev_x /= 4;
//            int prev_y =
//                    p1.second +
//                    p2.second +
//                    p3.second +
//                    p4.second;
//            prev_y /= 4;
//
//            /// Logarithmic search
//            for (const auto &prev_pair : prev_map) {
//                const auto prev = prev_pair.second + vert_offset + hor_offset;
//                int bias = 4, new_prev_x = 0, new_prev_y = 0;
//                while (bias != 0) {
//                    for (int y = prev_y - bias; y <= prev_y + bias; y += bias) {
//                        for (int x = prev_x - bias; x <= prev_x + bias; x += bias) {
//                            const auto comp = prev + y * width_ext + x;
//                            const int error = GetErrorSAD_8x8_my(cur, comp, width_ext);
//                            if (error < best_vector.error) {
//                                best_vector.x = x;
//                                best_vector.y = y;
//                                best_vector.shift_dir = prev_pair.first;
//                                best_vector.error = error;
//                                new_prev_x = x, new_prev_y = y;
//                            }
//                        }
//                    }
//                    prev_x = new_prev_x;
//                    prev_y = new_prev_y;
//                    bias >>= 1;
//                }
//            }
//
//            mvectors.set_mv(j, i, best_vector);
//        }
//    }
}

void extend_with_borders(
        unsigned char *input,
        unsigned char *output,
        size_t height,
        size_t width,
        size_t border_size
) {
    // Copy frame to center of new
    size_t new_width = width + 2 * border_size;
    auto p_output = output + new_width * border_size + border_size;
    auto p_input = input;
    for (size_t y = 0; y < height; ++y, p_output += 2 * border_size) {
        for (size_t x = 0; x < width; ++x, ++p_output, ++p_input) {
            *p_output = *p_input;
        }
    }

    // Left and right borders.
    p_output = output + new_width * border_size;
    for (size_t y = 0; y < height; ++y) {
        memset(p_output, p_output[border_size], border_size);
        p_output += border_size + width;
        memset(p_output, p_output[-1], border_size);
        p_output += border_size;
    }

    // Top and bottom borders.
    p_output = output;
    auto p_output_reference_row = p_output + new_width * border_size;

    for (size_t y = 0; y < border_size; ++y) {
        memcpy(p_output, p_output_reference_row, new_width);
        p_output += new_width;
    }
    p_output = output + new_width * (height + border_size);
    p_output_reference_row = p_output_reference_row - new_width;

    for (size_t y = 0; y < border_size; ++y) {
        memcpy(p_output, p_output_reference_row, new_width);
        p_output += new_width;
    }
}

void generate_subpixel_arrays(
        unsigned char *input,
        unsigned char *output_up,
        unsigned char *output_left,
        unsigned char *output_up_left,
        size_t height,
        size_t width
) {
    for (size_t y = 0; y < height; ++y) {
        for (size_t x = 0; x < width; ++x) {
            size_t cur_pixel_pos = y * width + x;
            size_t left_pixel_pos = y * width + x - 1;
            size_t left_up_pixel_pos = (y - 1) * width + x - 1;
            size_t up_pixel_pos = (y - 1) * width + x;

            if (y > 0) {
                output_up[cur_pixel_pos] = (int(input[cur_pixel_pos]) + input[up_pixel_pos]) / 2;
            } else {
                output_up[cur_pixel_pos] = input[cur_pixel_pos];
            }
            if (x > 0) {
                output_left[cur_pixel_pos] = (int(input[cur_pixel_pos]) + input[left_pixel_pos]) / 2;
            } else {
                output_left[cur_pixel_pos] = input[cur_pixel_pos];
            }

            if (x > 0 && y > 0) {
                output_up_left[cur_pixel_pos] = (
                                                        int(input[cur_pixel_pos]) +
                                                        input[left_pixel_pos] +
                                                        input[left_up_pixel_pos] +
                                                        input[up_pixel_pos]
                                                ) / 4;
            } else if (y == 0) {
                output_up_left[cur_pixel_pos] = output_left[cur_pixel_pos];
            } else {
                output_up_left[cur_pixel_pos] = output_up[cur_pixel_pos];
            }
        }
    }
}

MEField MotionEstimator::Estimate(
        py::array_t<unsigned char> cur_Y,
        py::array_t<unsigned char> prev_Y
) {

    extend_with_borders((unsigned char *) cur_Y.request().ptr, cur_Y_borders, height, width, BORDER);
    extend_with_borders((unsigned char *) prev_Y.request().ptr, prev_Y_borders, height, width, BORDER);

    if (cur_Y.size() != prev_Y.size()) {
        throw std::runtime_error("Input shapes must match");
    }

    if (use_half_pixel) {
        generate_subpixel_arrays(
                prev_Y_borders,
                prev_Y_up_borders,
                prev_Y_left_borders,
                prev_Y_up_left_borders,
                width_borders,
                height_borders
        );
    }

    MotionEstimator::CEstimate(
            cur_Y_borders,
            prev_Y_borders,
            prev_Y_up_borders,
            prev_Y_left_borders,
            prev_Y_up_left_borders,
            me_field,
            me_field2,
            me_field3
    );

    me_field3 = me_field2;
    me_field2 = me_field;
    n_frame++;

    return me_field;
}

int inline GetErrorSAD_8x8_my(const unsigned char *block1, const unsigned char *block2, const int stride) {
    int sum = 0;
    sum += pow_2(int(block1[0 * stride + 0]) -int(block2[0 * stride + 0]));
    sum += pow_2(int(block1[0 * stride + 1]) -int(block2[0 * stride + 1]));
    sum += pow_2(int(block1[0 * stride + 2]) -int(block2[0 * stride + 2]));
    sum += pow_2(int(block1[0 * stride + 3]) -int(block2[0 * stride + 3]));
    sum += pow_2(int(block1[0 * stride + 4]) -int(block2[0 * stride + 4]));
    sum += pow_2(int(block1[0 * stride + 5]) -int(block2[0 * stride + 5]));
    sum += pow_2(int(block1[0 * stride + 6]) -int(block2[0 * stride + 6]));
    sum += pow_2(int(block1[0 * stride + 7]) -int(block2[0 * stride + 7]));
    sum += pow_2(int(block1[1 * stride + 0]) -int(block2[1 * stride + 0]));
    sum += pow_2(int(block1[1 * stride + 1]) -int(block2[1 * stride + 1]));
    sum += pow_2(int(block1[1 * stride + 2]) -int(block2[1 * stride + 2]));
    sum += pow_2(int(block1[1 * stride + 3]) -int(block2[1 * stride + 3]));
    sum += pow_2(int(block1[1 * stride + 4]) -int(block2[1 * stride + 4]));
    sum += pow_2(int(block1[1 * stride + 5]) -int(block2[1 * stride + 5]));
    sum += pow_2(int(block1[1 * stride + 6]) -int(block2[1 * stride + 6]));
    sum += pow_2(int(block1[1 * stride + 7]) -int(block2[1 * stride + 7]));
    sum += pow_2(int(block1[2 * stride + 0]) -int(block2[2 * stride + 0]));
    sum += pow_2(int(block1[2 * stride + 1]) -int(block2[2 * stride + 1]));
    sum += pow_2(int(block1[2 * stride + 2]) -int(block2[2 * stride + 2]));
    sum += pow_2(int(block1[2 * stride + 3]) -int(block2[2 * stride + 3]));
    sum += pow_2(int(block1[2 * stride + 4]) -int(block2[2 * stride + 4]));
    sum += pow_2(int(block1[2 * stride + 5]) -int(block2[2 * stride + 5]));
    sum += pow_2(int(block1[2 * stride + 6]) -int(block2[2 * stride + 6]));
    sum += pow_2(int(block1[2 * stride + 7]) -int(block2[2 * stride + 7]));
    sum += pow_2(int(block1[3 * stride + 0]) -int(block2[3 * stride + 0]));
    sum += pow_2(int(block1[3 * stride + 1]) -int(block2[3 * stride + 1]));
    sum += pow_2(int(block1[3 * stride + 2]) -int(block2[3 * stride + 2]));
    sum += pow_2(int(block1[3 * stride + 3]) -int(block2[3 * stride + 3]));
    sum += pow_2(int(block1[3 * stride + 4]) -int(block2[3 * stride + 4]));
    sum += pow_2(int(block1[3 * stride + 5]) -int(block2[3 * stride + 5]));
    sum += pow_2(int(block1[3 * stride + 6]) -int(block2[3 * stride + 6]));
    sum += pow_2(int(block1[3 * stride + 7]) -int(block2[3 * stride + 7]));
    sum += pow_2(int(block1[4 * stride + 0]) -int(block2[4 * stride + 0]));
    sum += pow_2(int(block1[4 * stride + 1]) -int(block2[4 * stride + 1]));
    sum += pow_2(int(block1[4 * stride + 2]) -int(block2[4 * stride + 2]));
    sum += pow_2(int(block1[4 * stride + 3]) -int(block2[4 * stride + 3]));
    sum += pow_2(int(block1[4 * stride + 4]) -int(block2[4 * stride + 4]));
    sum += pow_2(int(block1[4 * stride + 5]) -int(block2[4 * stride + 5]));
    sum += pow_2(int(block1[4 * stride + 6]) -int(block2[4 * stride + 6]));
    sum += pow_2(int(block1[4 * stride + 7]) -int(block2[4 * stride + 7]));
    sum += pow_2(int(block1[5 * stride + 0]) -int(block2[5 * stride + 0]));
    sum += pow_2(int(block1[5 * stride + 1]) -int(block2[5 * stride + 1]));
    sum += pow_2(int(block1[5 * stride + 2]) -int(block2[5 * stride + 2]));
    sum += pow_2(int(block1[5 * stride + 3]) -int(block2[5 * stride + 3]));
    sum += pow_2(int(block1[5 * stride + 4]) -int(block2[5 * stride + 4]));
    sum += pow_2(int(block1[5 * stride + 5]) -int(block2[5 * stride + 5]));
    sum += pow_2(int(block1[5 * stride + 6]) -int(block2[5 * stride + 6]));
    sum += pow_2(int(block1[5 * stride + 7]) -int(block2[5 * stride + 7]));
    sum += pow_2(int(block1[6 * stride + 0]) -int(block2[6 * stride + 0]));
    sum += pow_2(int(block1[6 * stride + 1]) -int(block2[6 * stride + 1]));
    sum += pow_2(int(block1[6 * stride + 2]) -int(block2[6 * stride + 2]));
    sum += pow_2(int(block1[6 * stride + 3]) -int(block2[6 * stride + 3]));
    sum += pow_2(int(block1[6 * stride + 4]) -int(block2[6 * stride + 4]));
    sum += pow_2(int(block1[6 * stride + 5]) -int(block2[6 * stride + 5]));
    sum += pow_2(int(block1[6 * stride + 6]) -int(block2[6 * stride + 6]));
    sum += pow_2(int(block1[6 * stride + 7]) -int(block2[6 * stride + 7]));
    sum += pow_2(int(block1[7 * stride + 0]) -int(block2[7 * stride + 0]));
    sum += pow_2(int(block1[7 * stride + 1]) -int(block2[7 * stride + 1]));
    sum += pow_2(int(block1[7 * stride + 2]) -int(block2[7 * stride + 2]));
    sum += pow_2(int(block1[7 * stride + 3]) -int(block2[7 * stride + 3]));
    sum += pow_2(int(block1[7 * stride + 4]) -int(block2[7 * stride + 4]));
    sum += pow_2(int(block1[7 * stride + 5]) -int(block2[7 * stride + 5]));
    sum += pow_2(int(block1[7 * stride + 6]) -int(block2[7 * stride + 6]));
    sum += pow_2(int(block1[7 * stride + 7]) -int(block2[7 * stride + 7]));
    return sum;
}

int inline GetErrorSAD_4x4_my(const unsigned char *block1, const unsigned char *block2, const int stride) {
    int sum = 0;
    sum += pow_2(int(block1[0 * stride + 0]) -int(block2[0 * stride + 0]));
    sum += pow_2(int(block1[0 * stride + 1]) -int(block2[0 * stride + 1]));
    sum += pow_2(int(block1[0 * stride + 2]) -int(block2[0 * stride + 2]));
    sum += pow_2(int(block1[0 * stride + 3]) -int(block2[0 * stride + 3]));
    sum += pow_2(int(block1[1 * stride + 0]) -int(block2[1 * stride + 0]));
    sum += pow_2(int(block1[1 * stride + 1]) -int(block2[1 * stride + 1]));
    sum += pow_2(int(block1[1 * stride + 2]) -int(block2[1 * stride + 2]));
    sum += pow_2(int(block1[1 * stride + 3]) -int(block2[1 * stride + 3]));
    sum += pow_2(int(block1[2 * stride + 0]) -int(block2[2 * stride + 0]));
    sum += pow_2(int(block1[2 * stride + 1]) -int(block2[2 * stride + 1]));
    sum += pow_2(int(block1[2 * stride + 2]) -int(block2[2 * stride + 2]));
    sum += pow_2(int(block1[2 * stride + 3]) -int(block2[2 * stride + 3]));
    sum += pow_2(int(block1[3 * stride + 0]) -int(block2[3 * stride + 0]));
    sum += pow_2(int(block1[3 * stride + 1]) -int(block2[3 * stride + 1]));
    sum += pow_2(int(block1[3 * stride + 2]) -int(block2[3 * stride + 2]));
    sum += pow_2(int(block1[3 * stride + 3]) -int(block2[3 * stride + 3]));
    return sum;
}

int inline GetErrorSAD_2x2_my(const unsigned char *block1, const unsigned char *block2, const int stride) {
    int sum = 0;
    sum += pow_2(int(block1[0 * stride + 0]) -int(block2[0 * stride + 0]));
    sum += pow_2(int(block1[0 * stride + 1]) -int(block2[0 * stride + 1]));
    sum += pow_2(int(block1[1 * stride + 0]) -int(block2[1 * stride + 0]));
    sum += pow_2(int(block1[1 * stride + 1]) -int(block2[1 * stride + 1]));
    return sum;
}

int inline GetErrorSAD_1x1_my(const unsigned char *block1, const unsigned char *block2, const int stride) {
    int sum = 0;
    sum += pow_2(int(block1[0 * stride + 0]) -int(block2[0 * stride + 0]));
    return sum;
}

std::pair<int, int> inline MotionEstimator::medium_elem_3x3(MEField &q, size_t i, size_t j) const {
    int x = 0, y = 0;
    std::pair<int, int> e;
    e = elem(q, -1, -1);
    x += e.first;
    y += e.second;
    e = elem(q, -1, 0);
    x += e.first;
    y += e.second;
    e = elem(q, -1, 1);
    x += e.first;
    y += e.second;
    e = elem(q, 0, -1);
    x += e.first;
    y += e.second;
    e = elem(q, 0, 0);
    x += e.first;
    y += e.second;
    e = elem(q, 0, 1);
    x += e.first;
    y += e.second;
    e = elem(q, 1, -1);
    x += e.first;
    y += e.second;
    e = elem(q, 1, 0);
    x += e.first;
    y += e.second;
    e = elem(q, 1, 1);
    x += e.first;
    y += e.second;
    x /= 9;
    y /= 9;
    return std::pair<int, int>{x, y};
}

std::pair<int, int> inline MotionEstimator::medium_elem_5x5(MEField &q, size_t i, size_t j) const {
    int x = 0, y = 0;
    std::pair<int, int> e;
    e = elem(q, -2, -2);
    x += e.first;
    y += e.second;
    e = elem(q, -2, -1);
    x += e.first;
    y += e.second;
    e = elem(q, -2, 0);
    x += e.first;
    y += e.second;
    e = elem(q, -2, 1);
    x += e.first;
    y += e.second;
    e = elem(q, -2, 2);
    x += e.first;
    y += e.second;
    e = elem(q, -1, -2);
    x += e.first;
    y += e.second;
    e = elem(q, -1, -1);
    x += e.first;
    y += e.second;
    e = elem(q, -1, 0);
    x += e.first;
    y += e.second;
    e = elem(q, -1, 1);
    x += e.first;
    y += e.second;
    e = elem(q, -1, 2);
    x += e.first;
    y += e.second;
    e = elem(q, 0, -2);
    x += e.first;
    y += e.second;
    e = elem(q, 0, -1);
    x += e.first;
    y += e.second;
    e = elem(q, 0, 0);
    x += e.first;
    y += e.second;
    e = elem(q, 0, 1);
    x += e.first;
    y += e.second;
    e = elem(q, 0, 2);
    x += e.first;
    y += e.second;
    e = elem(q, 1, -2);
    x += e.first;
    y += e.second;
    e = elem(q, 1, -1);
    x += e.first;
    y += e.second;
    e = elem(q, 1, 0);
    x += e.first;
    y += e.second;
    e = elem(q, 1, 1);
    x += e.first;
    y += e.second;
    e = elem(q, 1, 2);
    x += e.first;
    y += e.second;
    e = elem(q, 2, -2);
    x += e.first;
    y += e.second;
    e = elem(q, 2, -1);
    x += e.first;
    y += e.second;
    e = elem(q, 2, 0);
    x += e.first;
    y += e.second;
    e = elem(q, 2, 1);
    x += e.first;
    y += e.second;
    e = elem(q, 2, 2);
    x += e.first;
    y += e.second;
    x /= 25;
    y /= 25;
    return std::pair<int, int>{x, y};
}