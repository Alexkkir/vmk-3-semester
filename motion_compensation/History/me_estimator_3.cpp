#include "me_estimator.h"
#include "metric.h"
#include <iostream>


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

std::pair<int, int> MotionEstimator::medium_elem(MEField &q, size_t i, size_t j) const {
    int x = 0, y = 0;
    for (size_t k = i - 1; k <= i + 1; k++) {
        for (size_t l = j - 1; l <= j + 1; l++) {
            x += elem(q, i, j).first;
            y += elem(q, i, j).second;
        }
    }
    x /= 9;
    y /= 9;
    return std::pair<int, int>{x, y};
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

    for (size_t i = 0; i < num_blocks_vert; i++) {
        for (size_t j = (i) & 1; j < num_blocks_hor; j += 2) {
            const auto hor_offset = j * BLOCK_SIZE;
            const auto vert_offset = first_row_offset + i * BLOCK_SIZE * width_ext;
            const auto cur = cur_Y + vert_offset + hor_offset;

            MV best_vector;

            int x_prev = 0, y_prev = 0, x_acc = 0, y_acc = 0, x_vel = 0, y_vel = 0, err_prev = 10000;
            int small_search_th = 5, med_search_th = 10;

            if (n_frame >= 5) {
//                int th = 17;
                auto m2 = medium_elem_3x3(mvectors2, i, j), m3 = medium_elem_3x3(mvectors3, i, j);
                x_acc = 2 * m2.first - m3.first;
                y_acc = 2 * m2.second - m3.second;
                err_prev = mvectors2.get_mv(j, i).error;
//                x_vel = m2.first;
//                y_vel = m2.second;
//                if (abs(x_acc) > th || abs(y_acc) > th) x_acc = 0, y_acc = 0;
//                std::cout << m2.first << " " << m3.first << " | " <<  m2.second << " " << m3.second << std::endl;
            }

            best_vector.error = std::numeric_limits<long>::max();

            enum SearchType {
                Ideal, Small, Medium, Wide
            } search_type;

            if (abs(x_acc) <= small_search_th && abs(y_acc) <= small_search_th) {
                search_type = Small;
                x_prev = x_acc, y_prev = y_acc;
            } else if (abs(x_acc) <= med_search_th && abs(y_acc) <= med_search_th) {
                search_type = Medium;
                x_prev = x_acc, y_prev = y_acc;
            } else {
                search_type = Wide;
            }

            for (const auto &prev_pair : prev_map) {
                const auto prev = prev_pair.second + vert_offset + hor_offset;
                int x_new = 0, y_new = 0;

                int *biases, b_len = 0;
                int biases_small[] = {2, 1, 1}, biases_medium[] = {4, 2, 1, 1}, biases_wide[] = {8, 4, 2, 1};

                // Is ideal case?
//                int err = GetErrorSAD_8x8_my(cur, prev, width_ext, 8);
//                std::cout << err << std::endl;
//                if (err_prev < 10) {
//                    best_vector.x = 0;
//                    best_vector.y = 0;
//                    best_vector.shift_dir = prev_pair.first;
//                    best_vector.error = GetErrorSAD_8x8_my(cur, prev, width_ext, 8);
//                    break;
//                }

                if (search_type == Small) {
                    biases = biases_small;
                    b_len = sizeof(biases_small) / sizeof(int);
                } else if (search_type == Medium) {
                    biases = biases_medium;
                    b_len = sizeof(biases_medium) / sizeof(int);
                } else {
                    biases = biases_wide;
                    b_len = sizeof(biases_wide) / sizeof(int);
                }

                for (int n_iter = 0, bias; n_iter < b_len; n_iter++) {
                    bias = biases[n_iter];
                    for (int y = y_prev - bias; y <= y_prev + bias; y += bias) {
                        for (int x = x_prev - bias; x <= x_prev + bias; x += bias) {
                            const auto comp = prev + y * width_ext + x;
                            const int error = GetErrorSAD_8x8_my(cur, comp, width_ext, 8);
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
                }
            }
//            std::cout <<
//            abs(best_vector.x - x_acc) << " " <<
//            abs(best_vector.y - y_acc) << " | " <<
//            best_vector.x << " " <<
//            best_vector.y << " | " <<
//            x_acc << " " <<
//            y_acc << std::endl;
//            std::cout << best_vector.error << std::endl;

            mvectors.set_mv(j, i, best_vector);
        }
    }
    for (size_t i = 0; i < num_blocks_vert; i++) {
        for (size_t j = (i + 1) & 1; j < num_blocks_hor; j += 2) {
            const auto hor_offset = j * BLOCK_SIZE;
            const auto vert_offset = first_row_offset + i * BLOCK_SIZE * width_ext;
            const auto cur = cur_Y + vert_offset + hor_offset;

            MV best_vector;
            best_vector.error = std::numeric_limits<long>::max();

            std::pair<int, int>
                    p1 = elem(mvectors, i - 1, j),
                    p2 = elem(mvectors, i + 1, j),
                    p3 = elem(mvectors, i, j - 1),
                    p4 = elem(mvectors, i, j + 1);
            int prev_x =
                    p1.first +
                    p2.first +
                    p3.first +
                    p4.first;
            prev_x /= 4;
            int prev_y =
                    p1.second +
                    p2.second +
                    p3.second +
                    p4.second;
            prev_y /= 4;

            enum Dir {
                VERT, HOR
            };
            int biases[] = {2, 2, 1, 1, 1};
            for (const auto &prev_pair : prev_map) {
                Dir dir = VERT;
                const auto prev = prev_pair.second + vert_offset + hor_offset;
                int new_x = 0, new_y = 0;
                for (int count = 0; count < sizeof(biases) / sizeof(biases[0]); count++) {
                    int bias = biases[count];
                    int x_bias = (dir == VERT ? bias : 0);
                    int y_bias = (dir == HOR ? bias : 0);
                    for (int y = prev_y - y_bias, f_y = 1; f_y && y <= prev_y + y_bias; y += y_bias, f_y = y_bias) {
                        for (int x = prev_x - x_bias, f_x = 1; f_x && x <= prev_x + x_bias; x += x_bias, f_x = x_bias) {
                            const auto comp = prev + y * width_ext + x;
                            const int error = GetErrorSAD_8x8_my(cur, comp, width_ext, 8);
                            if (error < best_vector.error) {
                                best_vector.x = x;
                                best_vector.y = y;
                                best_vector.shift_dir = prev_pair.first;
                                best_vector.error = error;
                                new_x = x, new_y = y;
                            }
                        }
                    }
                    prev_x = new_x;
                    prev_y = new_y;
                    if (dir == HOR) {
                        dir = VERT;
                    } else {
                        dir = HOR;
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

int
GetErrorSAD_8x8_my(const unsigned char *block1, const unsigned char *block2, const int stride, const int block_size) {
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