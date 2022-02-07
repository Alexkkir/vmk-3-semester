#include "me_estimator.h"
#include "metric.h"
#include <iostream>


MotionEstimator::MotionEstimator(size_t width, size_t height, unsigned char quality, bool use_half_pixel) :
        width(width),
        height(height),
        quality(quality),
        q_threshold(102 - quality),
        q_diap(4 + quality / 20),
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

std::pair<int, int> MotionEstimator::elem(MEField &q, size_t i, size_t j) const {
    if (0 <= i && i < num_blocks_vert &&
        0 <= j && j < num_blocks_hor) {
        MV v = q.get_mv(j, i);
        return std::pair<int, int>{v.x, v.y};
    } else {
        return std::pair<int, int>{0, 0};
    }
}

std::pair<int, int> MotionEstimator::elem_v(std::vector<MV> &q, size_t i, size_t j) const {
    if (0 <= i && i < num_blocks_vert &&
        0 <= j && j < num_blocks_hor) {
        MV v = q[i * num_blocks_hor + j];
        return std::pair<int, int>{v.x, v.y};
    } else {
        return std::pair<int, int>{0, 0};
    }
}

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

    for (size_t i = 0; i < num_blocks_vert; ++i) {
        for (size_t j = i & 1; j < num_blocks_hor; j += 2) {
            MV best_vector;
            best_vector.error = std::numeric_limits<long>::max();
            for (const auto &prev_pair : prev_map) {
                const auto hor_offset = j * BLOCK_SIZE;
                const auto vert_offset = first_row_offset + i * BLOCK_SIZE * width_ext;
                const auto prev = prev_pair.second + vert_offset + hor_offset;
                const auto cur = cur_Y + vert_offset + hor_offset;


                // PUT YOUR CODE HERE
                int err_0 = GetErrorSAD_2x2_my(cur, prev_Y + vert_offset + hor_offset, width_ext);
//            std::cout << err_0 << std::endl;
                if (err_0 < 20 * BLOCK_SIZE * BLOCK_SIZE) {
                    best_vector.x = 0;
                    best_vector.y = 0;
                    best_vector.shift_dir = ShiftDir::NONE;
                    best_vector.error = err_0;
                    mvectors.set_mv(j, i, best_vector);
                    continue;
                }

                int finished = 0;
//            std::pair<int, int> m2 = medium_elem_5x5(mvectors2, i, j);
//            int x_prev_fr = m2.first, y_pref_fr = m2.second;

                // Brute force
                int x_init = 0, y_init = 0;
                int diap = q_diap;
                for (int y = y_init - diap; !finished && y <= y_init + diap; y += 2) {
                    for (int x = x_init - diap; !finished && x <= x_init + diap; x += 2) {
                        const auto comp = prev + y * width_ext + x;
                        const int error = GetErrorSAD_2x2_my(cur, comp, width_ext);
                        if (error < best_vector.error) {
                            best_vector.x = x;
                            best_vector.y = y;
                            best_vector.shift_dir = ShiftDir::NONE;
                            best_vector.error = error;
                            finished = (error < (102 - quality) * BLOCK_SIZE * BLOCK_SIZE);
                        }
                    }
                }

                x_init = (best_vector.x), y_init = (best_vector.y);
                diap = 2;
                for (int y = y_init - diap; !finished && y <= y_init + diap; y += 1) {
                    for (int x = x_init - diap; !finished && x <= x_init + diap; x += 1) {
                        const auto comp = prev + y * width_ext + x;
                        const int error = GetErrorSAD_2x2_my(cur, comp, width_ext);
                        if (error < best_vector.error) {
                            best_vector.x = x;
                            best_vector.y = y;
                            best_vector.shift_dir = ShiftDir::NONE;
                            best_vector.error = error;
                            finished = (error < q_threshold * BLOCK_SIZE * BLOCK_SIZE);
                        }
                    }
                }

            }
            mvectors.set_mv(j, i, best_vector);
        }
    }
    for (size_t i = 0; i < num_blocks_vert; ++i) {
        for (size_t j = (i & 1) ^1; j < num_blocks_hor; j += 2) {
            MV best_vector;
            best_vector.error = std::numeric_limits<long>::max();
            for (const auto &prev_pair : prev_map) {
                const auto hor_offset = j * BLOCK_SIZE;
                const auto vert_offset = first_row_offset + i * BLOCK_SIZE * width_ext;
                const auto prev = prev_pair.second + vert_offset + hor_offset;
                const auto cur = cur_Y + vert_offset + hor_offset;


                // PUT YOUR CODE HERE
                int err_0 = GetErrorSAD_2x2_my(cur, prev_Y + vert_offset + hor_offset, width_ext);
//            std::cout << err_0 << std::endl;
                if (err_0 < 20 * BLOCK_SIZE * BLOCK_SIZE) {
                    best_vector.x = 0;
                    best_vector.y = 0;
                    best_vector.shift_dir = ShiftDir::NONE;
                    best_vector.error = err_0;
                    mvectors.set_mv(j, i, best_vector);
                    continue;
                }

                std::pair<int, int>
                        p1 = elem(mvectors, i - 1, j),
                        p2 = elem(mvectors, i + 1, j),
                        p3 = elem(mvectors, i, j - 1),
                        p4 = elem(mvectors, i, j + 1);
                int x_init =
                        p1.first +
                        p2.first +
                        p3.first +
                        p4.first;
                x_init /= 4;
                int y_init =
                        p1.second +
                        p2.second +
                        p3.second +
                        p4.second;
                y_init /= 4;

                // Brute force
                int diap = 2, finished = 0;
                int error;
                for (int y = y_init - diap; !finished && y <= y_init + diap; y += 1) {
                    for (int x = x_init - diap; !finished && x <= x_init + diap; x += 1) {
                        error = GetErrorSAD_2x2_my(cur, prev + y * width_ext + x, width_ext);
                        if (error < best_vector.error) {
                            best_vector.x = x;
                            best_vector.y = y;
                            best_vector.shift_dir = ShiftDir::NONE;
                            best_vector.error = error;
                            finished = (error < q_threshold * BLOCK_SIZE * BLOCK_SIZE);
                        }
                    }
                }

            }
            mvectors.set_mv(j, i, best_vector);
        }
    }
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

int GetErrorSAD_8x8_my(const unsigned char *block1, const unsigned char *block2, const int stride) {
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

int GetErrorSAD_4x4_my(const unsigned char *block1, const unsigned char *block2, const int stride) {
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

int GetErrorSAD_1x1_my(const unsigned char *block1, const unsigned char *block2, const int stride) {
    int sum = 0;
    sum += pow_2(int(block1[0 * stride + 0]) -int(block2[0 * stride + 0]));
    return sum;
}

std::pair<int, int> MotionEstimator::medium_elem_3x3(MEField &q, size_t i, size_t j) const {
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

std::pair<int, int> MotionEstimator::medium_elem_5x5(MEField &q, size_t i, size_t j) const {
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