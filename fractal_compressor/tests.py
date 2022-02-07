from collections import namedtuple

import numpy as np
from skimage.metrics import mean_squared_error as mse


BlockTransform = namedtuple('BlockTransform', ['x', 'y', 'di', 'tr'])


TRANSFORM_UNIT_TESTS = [
    [
        np.array([[3, 6], [9, 12]]),
        np.array([[4, 8], [12, 16]]),
        BlockTransform(x=0, y=0, di=0, tr=0),
    ],
    [
        np.array([[4, 7], [10, 13]]),
        np.array([[4, 8], [12, 16]]),
        BlockTransform(x=0, y=0, di=1, tr=0),
    ],
    [
        np.array([[6, 3], [12, 9]]),
        np.array([[4, 8], [12, 16]]),
        BlockTransform(x=0, y=0, di=0, tr=1),
    ],
    [
        np.array([[6, 12], [3, 9]]),
        np.array([[4, 8], [12, 16]]),
        BlockTransform(x=0, y=0, di=0, tr=2),
    ],
    [
        np.array([[3, 9], [6, 12]]),
        np.array([[4, 8], [12, 16]]),
        BlockTransform(x=0, y=0, di=0, tr=3),
    ],
    [
        np.array([[2, 5], [5, 11]]),
        np.array([[4, 8], [8, 16]]),
        BlockTransform(x=0, y=0, di=-1, tr=0),
    ],
]


def test_transform(find_block_transform, perform_transform):
    block_size = 2
    for rank_block, domain_block, ideal_transform in TRANSFORM_UNIT_TESTS:
        transform = find_block_transform(image=rank_block, resized_image=domain_block, x=0, y=0, block_size=block_size, stride=1)
        transformed = perform_transform(np.zeros_like(rank_block), domain_block, [transform], block_size)
        loss = mse(rank_block, transformed)
        if loss > 1e-5:
            print(f"Rank block to estimate:\n{rank_block}")
            print(f"Selected domain block:\n{domain_block}")
            print(f"Applied transform: {transform}")
            print(f"Transformed domain block:\n{transformed}")
            print(f"MSE={loss} > limit={1e-5}")
            print(f"Probably you need to apply {ideal_transform}")
            print("Not OK")
            return

    return "OK"


def test_bit_buffer(BitBuffer):
    def fill():
        bb = BitBuffer()
        bb.push(9, 6)
        bb.push(18, 5)
        bb.push(1, 1)
        bb.push(123, 40)
        return bb

    bb = fill()
    res1 = []
    res1.append(bb.pop(40))
    res1.append(bb.pop(1))
    res1.append(bb.pop(5))
    res1.append(bb.pop(6))
    if res1 == [123, 1, 18, 9]:
        return "OK"
    bb = fill()
    res2 = []
    res2.append(bb.pop(6))
    res2.append(bb.pop(5))
    res2.append(bb.pop(1))
    res2.append(bb.pop(40))
    if res2 == [9, 18, 1, 123]:
        return "OK"
    return "Not OK"
