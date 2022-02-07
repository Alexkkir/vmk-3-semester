# for x in range(-8, 9, 2):
# 	for y in range(-8, 9, 2):
# 		print(f"\
# error = GetErrorSAD_2x2_my(cur, prev + {y} * width_ext + {x}, width_ext);\
# if (error < best_vector.error) {{\
# best_vector.x = {x};\
# best_vector.y = {y};\
# best_vector.shift_dir = ShiftDir::NONE;\
# best_vector.error = error;\
# finished = (error < 22 * BLOCK_SIZE * BLOCK_SIZE);\
# }}")

for x in range(-1, 2):
	for y in range(-1, 2):
		print(f"\
e = elem(q, i-2, -2); x += e.first; y += e.second;\n")
