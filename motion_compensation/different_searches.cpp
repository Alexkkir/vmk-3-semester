int main() {
    /// ORTHOGONAL
    enum Dir {
        VERT, HOR
    };
    for (const auto& prev_pair : prev_map) {
        Dir dir = VERT;
        const auto prev = prev_pair.second + vert_offset + hor_offset;
        int bias = 4, prev_x = 0, prev_y = 0, new_x = 0, new_y = 0;
        while (bias != 0) {
            int x_bias = (dir == VERT ? bias : 0);
            int y_bias = (dir == HOR ? bias : 0);
            for (int y = prev_y - y_bias, f_y = 1; f_y && y <= prev_y + y_bias; y += y_bias, f_y = y_bias) {
                for (int x = prev_x - x_bias, f_x = 1; f_x && x <= prev_x + x_bias; x += x_bias, f_x = x_bias) {
                    const auto comp = prev + y * width_ext + x;
                    const int error = GetErrorSAD_16x16(cur, comp, width_ext);
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
            bias /= 1;
            if (dir == HOR) {
                dir = VERT;
            } else {
                dir = HOR;
            }
        }
    }

    /// Very good orthogonal
    enum Dir {
        VERT, HOR
    };
    int biases[] = {2, 2, 1, 1, 1};
    for (const auto& prev_pair : prev_map) {
        Dir dir = VERT;
        const auto prev = prev_pair.second + vert_offset + hor_offset;
        int x_prev = 0, y_prev = 0, x_new = 0, y_new = 0;
        for (int count = 0; count < 5; count++) {
            int bias = biases[count];
            int x_bias = (dir == VERT ? bias : 0);
            int y_bias = (dir == HOR ? bias : 0);
            for (int y = y_prev - y_bias, f_y = 1; f_y && y <= y_prev + y_bias; y += y_bias, f_y = y_bias) {
                for (int x = x_prev - x_bias, f_x = 1; f_x && x <= x_prev + x_bias; x += x_bias, f_x = x_bias) {
                    const auto comp = prev + y * width_ext + x;
                    const int error = GetErrorSAD_16x16(cur, comp, width_ext);
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
            if (dir == HOR) {
                dir = VERT;
            } else {
                dir = HOR;
            }
        }
    }

    /// Brute force
    for (const auto& prev_pair : prev_map) {
        const auto prev = prev_pair.second + vert_offset + hor_offset;
        for (int y = -BORDER; y <= BORDER; ++y) {
            for (int x = -BORDER; x <= BORDER; ++x) {
                const auto comp = prev + y * width_ext + x;
                const int error = GetErrorSAD_16x16(cur, comp, width_ext);
                if (error < best_vector.error) {
                    best_vector.x = x;
                    best_vector.y = y;
                    best_vector.shift_dir = prev_pair.first;
                    best_vector.error = error;
                }
            }
        }
    }

    /// Logarithmic
    for (const auto& prev_pair : prev_map) {
        const auto prev = prev_pair.second + vert_offset + hor_offset;
        int bias = 8, x_prev = 0, y_prev = 0, x_new = 0, y_new = 0;
        while (bias != 0) {
            for (int y = y_prev - bias; y <= y_prev + bias; y += bias) {
                for (int x = x_prev - bias; x <= x_prev + bias; x += bias) {
                    const auto comp = prev + y * width_ext + x;
                    const int error = GetErrorSAD_16x16(cur, comp, width_ext);
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

    /// X serach
    for (const auto& prev_pair : prev_map) {
        const auto prev = prev_pair.second + vert_offset + hor_offset;
        int bias = 8, x_prev = 0, y_prev = 0, x_new = 0, y_new = 0;
        while (bias != 0) {
            for (int y = y_prev - bias, tmp = 0; y <= y_prev + bias; y += bias, tmp++) {
                for (int x = x_prev - bias + bias * (tmp % 2); x <= x_prev + bias; x += 2 * bias) {
                    const auto comp = prev + y * width_ext + x;
                    const int error = GetErrorSAD_16x16(cur, comp, width_ext);
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

    /// Ortho variable
    enum Dir {
        VERT, HOR
    } dir = VERT;
    for (const auto &prev_pair : prev_map) {
        const auto prev = prev_pair.second + vert_offset + hor_offset;
        int x_new = 0, y_new = 0;

        int *biases, b_len = 0;
        int biases_small[] = {2, 1}, biases_medium[] = {2, 2, , 1, 1}, biases_wide[] = {4, 2, 1, 1, 1, 1};

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
        for (int n_iter = 0; n_iter < b_len; n_iter++) {
            int bias = biases[n_iter];
            int x_bias = (dir == VERT ? bias : 0);
            int y_bias = (dir == HOR ? bias : 0);
            for (int y = prev_y - y_bias, f_y = 1; f_y && y <= prev_y + y_bias; y += y_bias, f_y = y_bias) {
                for (int x = prev_x - x_bias, f_x = 1; f_x && x <= prev_x + x_bias; x += x_bias, f_x = x_bias) {
                    const auto comp = prev + y * width_ext + x;
                    const int error = GetErrorSAD_my(cur, comp, width_ext, 8);
                    if (error < best_vector.error) {
                        best_vector.x = x;
                        best_vector.y = y;
                        best_vector.shift_dir = prev_pair.first;
                        best_vector.error = error;
                        x_new = x, y_new = y;
                    }
                }
            }
            prev_x = x_new;
            prev_y = y_new;
            if (dir == HOR) {
                dir = VERT;
            } else {
                dir = HOR;
            }
        }
    }
}