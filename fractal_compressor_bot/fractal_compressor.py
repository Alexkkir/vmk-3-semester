import os
import itertools

from collections import namedtuple

# Additional Modules

import matplotlib.pyplot as plt
import numpy as np

from skimage import io
from skimage import data, img_as_float64
from skimage.metrics import mean_squared_error as mse, peak_signal_noise_ratio as psnr
from skimage.transform import resize
from skimage.color import rgb2gray, rgb2yuv, yuv2rgb

from tqdm.contrib.telegram import tqdm  # tqdm for telegram

BlockTransform = namedtuple('BlockTransform', ['x', 'y', 'col', 'tr', 'mse', 'size', 'x_or', 'y_or'])
ColorChange = namedtuple('ColorChange', ['p', 'r'])

FractalCompressionParams = namedtuple(
    'FractalCompressionParams', [
        'height',
        'width',
        'is_colored',
        'block_size',
        #         'spatial_scale',
        #         'intensity_scale',
        'stride'
    ]
)


def derive_num_bits(length, stride):
    return np.ceil(np.log2(length / stride)).astype(int)


def is_colored(image):
    if len(image.shape) == 2:
        return False
    elif len(image.shape) == 3 and image.shape[-1] == 3:
        return True
    else:
        message = 'Invalid shape of the image: `{}`'
        raise ValueError(message.format(image.shape))


# GRADED CELL: find_block_transform

def find_block_transform(image, image_2, resized_image, resized_image_2, x, y, block_size, stride, to_print=False):
    '''Find best transformation for given rank block.

    Parameters
    ----------
    image : np.array
        Source B/W image.

    resized_image: np.array
        Resized source image.

    x, y: int, int
        Coordinates of the rank block.

    block_size: int
        Size of rank block.

    stride: int
        Vertical and horizontal stride for domain block search.

    Returns
    -------
    best_transform: BlockTransform
        Best transformation.
    '''

    def find_contrast_params(R, R_2, D, D_2):
        R = R.astype('float32')
        D = D.astype('float32')
        sum_R = np.sum(R)
        sum_D = np.sum(D)
        sum_R2 = np.sum(R_2)
        sum_D2 = np.sum(D_2)
        sum_RD = sum_DR = np.sum(D * R)
        n = R.shape[0]
        n_2 = R.size
        divider = (sum_D2 * n_2 - sum_D ** 2)
        if divider == 0: return 1, 0, 10000000000
        p = (sum_DR * n_2 - sum_D * sum_R) / divider
        if abs(p) > 0.99:
            p = 0.99 if p > 0 else -0.99
        p = int(16 * p) / 16
        r = (sum_R - p * sum_D) / (n ** 2)
        r = round(r / 2) * 2
        MSE = sum_R2 + (p ** 2) * sum_D2 + (n * r) ** 2 - 2 * p * sum_RD - 2 * r * sum_R + 2 * p * r * sum_D
        # MSE *= n ** 0.25
        # p = 0.75
        # X = R - D * p
        # r = (np.sum(X)) / D.size
        # r = int(r)
        # MSE = np.sum((X - r)**2)

        return p, r, MSE

    def trans(X, i):
        for j in range(i % 4):
            X = np.rot90(X)
        if (i >= 4):
            X = X.T
        return X

    # resized_image = resized_image * 0.75
    trans_im = [trans(resized_image, i) for i in range(8)]
    trans_im_2 = [trans(resized_image_2, i) for i in range(8)]
    R = image[x:x + block_size, y:y + block_size]
    R_2 = image_2[x:x + block_size, y:y + block_size]
    size = image.shape[0]
    best_t = None
    best_MSE = 10000000000000
    for xx, yy in itertools.product(range(0, size // 2 - block_size + 1, stride),
                                    range(0, size // 2 - block_size + 1, stride)):
        for i in range(8):
            D = trans_im[i][xx:xx + block_size, yy:yy + block_size]
            D_2 = trans_im_2[i][xx:xx + block_size, yy:yy + block_size]
            p, r, MSE = find_contrast_params(R, R_2, D, D_2)
            if MSE < best_MSE:
                best_MSE = MSE
                best_t = BlockTransform(xx, yy, ColorChange(p, r), i, MSE, block_size, x, y)

    if to_print == True:
        plt.figure(figsize=(15, 5))
        plt.subplot(1, 3, 1)
        plt.imshow(image, cmap='gray', vmin=0, vmax=255)

        plt.subplot(1, 3, 2)
        plt.imshow(R, cmap='gray', vmin=0, vmax=255)

        t = best_t
        xx = t.x
        yy = t.y
        p = t.col.p
        r = t.col.r
        # coords on source image
        D = trans_im[t.tr][xx:xx + block_size, yy:yy + block_size]
        X = D * p + r
        plt.subplot(1, 3, 3)
        plt.imshow(X, cmap='gray', vmin=0, vmax=255)
    return best_t


# GRADED CELL: perform_transform

def perform_transform(image, resized_image, transforms, block_size):
    '''Perform IFS on given image.

    Parameters
    ----------
    image : np.array
        Source image.

    resized_image: np.array
        Resized source image.

    transforms: list of BlockTransform's
        Given IFS, Iterated Function System

    block_size: int
        Size of rank block.

    Returns
    -------
    transformed_image: np.array
        Transformed image.
    '''

    def trans(X, i):
        for j in range(i % 4):
            X = np.rot90(X)
        if (i >= 4):
            X = X.T
        return X

    class Reader:
        def __init__(self, t):
            self.pos = 0
            self.body = t

        def read(self):
            ret = self.body[self.pos]
            self.pos += 1
            return ret

    def decode_block(x, y, block_size):
        elem = rdr.read()
        if elem != 'split':
            xx = elem.x;
            yy = elem.y;
            p = elem.col.p;
            r = elem.col.r  # coords on source image
            D = trans_im[elem.tr][xx:xx + block_size, yy:yy + block_size]
            X = D * p + r
            transformed_image[x:x + block_size, y:y + block_size] = X
        else:
            block_size //= 2
            decode_block(x, y, block_size)
            decode_block(x + block_size, y, block_size)
            decode_block(x, y + block_size, block_size)
            decode_block(x + block_size, y + block_size, block_size)

    image_size = image.shape[0]
    # resized_image = resized_image * 0.75
    transformed_image = np.zeros(image.shape)
    rdr = Reader(transforms)

    trans_im = [trans(resized_image, i) for i in range(8)]
    coordinates = itertools.product(range(0, image_size, block_size), range(0, image_size, block_size))
    for x, y in coordinates:
        decode_block(x, y, block_size)
    return transformed_image


# GRADED CELL: BitBuffer

class BitBuffer:
    '''Class that provides storing and and reading integer numbers
    in continuous bytearray.

    Parameters
    ----------
    buffer : bytearray, optional (default=None)
        Input bytearray, for initialization.

    Attributes
    ----------
    _buffer : bytearray
        Bytearray that can contain any information.

    Examples
    --------
    >>> buffer = BitBuffer()
    >>> buffer.push(1, 1)
    >>> x = buffer.pop(1)
    >>> print(x)
    1
    >>> buffer.push(125, 18)
    >>> x = buffer.pop(18)
    >>> print(x)
    125
    >>> buffer.push(5, 3)
    >>> x = buffer.pop(3)
    >>> print(x)
    5

    >>> dy = transform.y // stride
    >>> buffer.push(dy, self._num_bits_ver)
    '''

    def __init__(self, buffer=None):
        self.chunk = 8
        if buffer is not None:
            self.from_raw(buffer)
        else:
            self.rescale = 1
            self.pos = 0
            self.size = self.chunk * self.rescale
            self.body = np.zeros(self.rescale, dtype=np.int8)

    def to_bytearray(self):
        '''Convert to bytearray.

        Returns
        -------
        buffer: bytearray
            Bytearray that contains all data.
        '''
        cur_pos = self.pos % 8
        self.push(0, 8 - cur_pos)
        self.push(0, 5)
        self.push(cur_pos, 3)
        return bytearray(self.body)

    def from_raw(self, buf):
        self.body = np.array(buf)
        self.pos = self.body.size * self.chunk
        self.size = self.pos
        cur_pos = self.pop(3)
        self.pop(5)
        self.pop(8 - cur_pos)

    def push(self, x, n_bits):
        '''Push given integer to buffer.

        Parameters
        ----------
        x : int
            Input number.

        n_bits: int
            Number of bits for store input number,
            should be greater than log2(x).
        '''

        # YOUR CODE HERE
        while self.pos + n_bits > self.size:
            self.body = np.concatenate((self.body, np.zeros(self.rescale, dtype=np.int8)))
            self.size += self.rescale * self.chunk
        for i in range(self.pos, self.pos + n_bits):
            bit = (x % 2) << (i % self.chunk)
            self.body[i // self.chunk] |= bit
            x >>= 1
        self.pos += n_bits

    def pop(self, n_bits):
        '''Pop n_bits from buffer and transform it to a number.

        Parameters
        ----------
        n_bits: int
            Number of bits for pop from buffer.

        Returns
        -------
        x: int
            Extracted number.
        '''

        # YOUR CODE HERE
        x = 0
        for i in range(self.pos - 1, self.pos - n_bits - 1, -1):
            x <<= 1
            bias = i % self.chunk
            bit = 1 << bias
            x += (self.body[i // self.chunk] & bit) >> bias
            self.body[i // self.chunk] &= ~np.int8(bit)
        self.pos -= n_bits
        return x


# GRADED CELL: FractalCompressor

class Node:
    def __init__(self, t=None, x=0, y=0, size=4):
        self.childs = [None] * 4
        self.t = t
        self.x = x
        self.y = y
        self.size = size
        self.split = False

    def divide(self):
        self.split = True
        vals = {0: {'x': self.x, 'y': self.y, 'size': self.size // 2},
                1: {'x': self.x + self.size // 2, 'y': self.y, 'size': self.size // 2},
                2: {'x': self.x, 'y': self.y + self.size // 2, 'size': self.size // 2},
                3: {'x': self.x + self.size // 2, 'y': self.y + self.size // 2, 'size': self.size // 2},
                }
        for i in range(4):
            self.childs[i] = Node(**vals[i])

    def flatten(self, node=None):
        res = []
        node = node or self
        if node.split:
            res.append('split')
            for i in range(4):
                res += self.flatten(node.childs[i])
        else:
            res.append(node.t)
        return res

    def print(self, b=0):
        node = self
        print('\t' * b + f"[{node.x} {node.y} {node.size}] = {b}")
        for node in self.childs:
            if node:
                node.print(b + 1)


class MaxHeap:
    def __init__(self, maxsize):
        # 'x', 'y', 'col', 'tr', 'mse', 'size', 'x_or', 'y_or'

        self.maxsize = maxsize
        self.size = 0
        self.Heap = [Node(t=BlockTransform(-1, -1, ColorChange(-1, -1), -1, -1, -1, -1, -1))] * (self.maxsize + 1)
        self.Heap[0] = Node(t=BlockTransform(-1, -1, ColorChange(-1, -1), -1, 10000000000000, -1, -1, -1))
        self.FRONT = 1

    def parent(self, pos):
        return pos // 2

    def leftChild(self, pos):
        return 2 * pos

    def rightChild(self, pos):
        return (2 * pos) + 1

    def isLeaf(self, pos):
        if (self.size // 2) <= pos <= self.size:
            return True
        return False

    def swap(self, fpos, spos):
        self.Heap[fpos], self.Heap[spos] = (self.Heap[spos],
                                            self.Heap[fpos])

    def val(self, elem: Node):
        return elem.t.mse

    def maxHeapify(self, pos):
        if not self.isLeaf(pos):
            if (self.val(self.Heap[pos]) < self.val(self.Heap[self.leftChild(pos)]) or
                    self.val(self.Heap[pos]) < self.val(self.Heap[self.rightChild(pos)])):
                if (self.val(self.Heap[self.leftChild(pos)]) >
                        self.val(self.Heap[self.rightChild(pos)])):
                    self.swap(pos, self.leftChild(pos))
                    self.maxHeapify(self.leftChild(pos))
                else:
                    self.swap(pos, self.rightChild(pos))
                    self.maxHeapify(self.rightChild(pos))

    def insert(self, element: Node):
        if self.size >= self.maxsize:
            print('exceeded max heap size')
            return
        self.size += 1
        self.Heap[self.size] = element
        current = self.size
        while (self.val(self.Heap[current]) >
               self.val(self.Heap[self.parent(current)])):
            self.swap(current, self.parent(current))
            current = self.parent(current)

    def print(self):
        for i in range(1, (self.size // 2) + 1):
            print(" PARENT : " + str(self.Heap[i].t) +
                  " LEFT CHILD : " + str(self.Heap[2 * i].t) +
                  " RIGHT CHILD : " + str(self.Heap[2 * i + 1].t))

    def pop(self):
        popped = self.Heap[self.FRONT]
        self.Heap[self.FRONT] = self.Heap[self.size]
        self.size -= 1
        self.maxHeapify(self.FRONT)
        return popped


class FractalCompressor:
    '''Class that performs fractal compression/decompression of images.

    Attributes
    ----------
    _num_bits_ver : int
        Number of bits for store VERTICAL OFFSET for each transformation.

    _num_bits_hor : int
        Number of bits for store HORIZONTAL OFFSET for each transformation.

    _num_bits_pix : int
        Number of bits for store INTENSITY OFFSET for each transformation.

    _num_bits_tfm : int
        Number of bits for store TRANFORMATION INDEX for each transformation.

    Examples
    --------
    >>> comp = FractalCompressor()
    >>> compressed_image = comp.get_image(image, block_size=8, stride=2)
    >>> decompressed_image = comp.decompress(compressed_image, num_iters=9)
    >>> yet_another_compressed_image = comp.get_image(image, 8, 4, 0.5, 0.7)
    >>> yet_another_decompressed_image = comp.get_image(yet_another_compressed_image, 5)
    '''

    def __init__(self, telegram_token=None, chat_id=None):
        self.TELEGRAM_TOKEN = telegram_token
        self.CHAT_ID = chat_id
        pass

    def _add_header(self, buffer, params):
        '''Store header in buffer.

        Parameters
        ----------
        buffer: BitBuffer

        params: FractalCompressionParams
            Parameters that should be stored in buffer.

        Note
        ----
        This method must be consistent with `_read_header`.
        '''

        #         FractalCompressionParams = namedtuple(
        #             'FractalCompressionParams', [
        #                 'height',
        #                 'width',
        #                 'is_colored',
        #                 'block_size',
        #                / 'spatial_scale',
        #                / 'intensity_scale',
        #                 'stride'
        #             ]
        #         )
        # YOUR CODE HERE

        buffer.push(params.height, 16)
        buffer.push(params.width, 16)
        buffer.push(params.is_colored, 1)
        buffer.push(params.block_size, 8)
        #         buffer.push(params.spatial_scale)
        #         buffer.push(intensity_scale)
        buffer.push(params.stride, 8)

    def _read_header(self, buffer):
        '''Read header from buffer.

        Parameters
        ----------
        buffer: BitiBuffer

        Returns
        -------
        params: FractalCompressionParams
            Extracted parameters.

        Note
        ----
        This method must be consistent with `_add_header`.
        '''

        # YOUR CODE HERE
        stride = buffer.pop(8)
        intensity_scale = 0.75
        spatial_scale = 0.5
        block_size = buffer.pop(8)
        is_colored = buffer.pop(1)
        width = buffer.pop(16)
        height = buffer.pop(16)

        params = FractalCompressionParams(height, width, is_colored, block_size, stride)
        return params

    def _add_transform(self, buffer, transform, stride):
        '''Store block transformation in buffer.

        Parameters
        ----------
        buffer: BitBuffer

        transform: BlockTransform

        stride: int
            Vertical and horizontal stride for domain block search.

        Note
        ----
        This method must be consistent with `_read_transform`.
        '''

        if transform == 'split':
            buffer.push(0, 1)
            buffer.push(1, 1)
        else:
            buffer.push(transform.x // 16, 3)
            buffer.push(transform.y // 16, 3)
            buffer.push(int(16 * transform.col.p) + 16, 5)
            r = transform.col.r
            abs_r = abs(r);
            sign_r = 1 if r > 0 else 0
            not_char = 1 if abs(r) > 255 else 0
            buffer.push(abs_r // 2, 7)
            buffer.push(not_char, 1)
            buffer.push(sign_r, 1)
            buffer.push(transform.tr, 3)
            buffer.push(0, 1)

    def _read_transform(self, buffer, stride):
        '''Read block transformation from buffer.

        Parameters
        ----------
        buffer: BitBuffer


        stride: int
            Vertical and horizontal stride for domain block search.

        Returns
        -------
        transform: BlockTransform
            Extracted block transformation.

        Note
        ----
        This method must be consistent with `_add_to_buffer`.
        '''

        bit = buffer.pop(1)
        if bit == 0:
            tr = buffer.pop(3)
            sign_r = buffer.pop(1) * 2 - 1
            not_char = buffer.pop(1)
            abs_r = buffer.pop(7) * 2
            r = sign_r * (256 * not_char + abs_r)
            p = 0.75
            p = (buffer.pop(5) - 16) / 16
            y = buffer.pop(3) * 16
            x = buffer.pop(3) * 16

            transform = BlockTransform(x, y, ColorChange(p, r), tr, -1, -1, -1, -1)
        elif bit == 1:
            bit = buffer.pop(1)
            if bit == 0:
                transform = 'split'
            elif bit == 1:
                transform = 'end'
        return transform

    def _read_layer(self, buffer, stride):
        trans = []
        while True:
            t = self._read_transform(buffer, stride)
            if t != 'end':
                trans.append(t)
            elif t == 'end':
                return trans

    def _ifs2buf(self, params, transformations):
        '''Store compression parameters and IFS in buffer.

        Parameters
        ----------
        params: FractalCompressionParams
            Parameters of the compression.

        transformations: list of BlockTransform's
            Given IFS.

        Returns
        -------
        buffer: BitBuffer

        Note
        ----
        This method must be consistent with `_buf2ifs`.
        '''

        buffer = BitBuffer()
        for t_ch in transformations[::-1]:
            buffer.push(1, 1)  # b11
            buffer.push(1, 1)
            for t in t_ch[::-1]:
                self._add_transform(buffer, t, params.stride)
        self._add_header(buffer, params)
        return buffer.to_bytearray()

    def _buf2ifs(self, buffer):
        '''Store compression parameters and IFS in buffer.

        Parameters
        ----------
        buffer: BitBuffer

        Returns
        -------
        params: FractalCompressionParams
            Extracted compression parameters.

        transforms: list of BlockTransform's
            Extracted IFS.

        Note
        ----
        This method must be consistent with `_ifs2buf`.
        '''

        params = self._read_header(buffer)

        #         self._num_bits_ver = derive_num_bits(params.height, params.stride)
        #         self._num_bits_hor = derive_num_bits(params.width, params.stride)
        #         num_transforms = int(params.height * params.width / params.block_size ** 2)
        n_ch = 3 if params.is_colored else 1
        transforms = [
            self._read_layer(buffer, params.stride)
            for _ in range(n_ch)
        ]

        return params, transforms

    def compress2(self, image, quality=40):
        n_div = [int(x) for x in np.linspace(50, 1000, 6)]
        params = {q: d for q, d in zip(range(0, 100 + 1, 20), n_div)}
        return self.compress(image, n_div=params[quality])

    def compress(self, image, block_size=16, stride=16, spatial_scale=0.5, intensity_scale=0.75,
                 threshold=1000, min_block=4, n_div=100):
        params = FractalCompressionParams(image.shape[0], image.shape[1], is_colored(image),
                                          block_size, stride)

        def encode(image, image_2, resized_image, resized_image_2, n_div):
            tree = Node(t=BlockTransform(-1, -1, ColorChange(-1, -1), -1, -1, -1, -1, -1), x=0, y=0, size=256)
            heap = MaxHeap(n_div * 5 + 3)
            heap.insert(tree)
            stride = {32: 16, 16: 16, 8: 16, 4: 16, 2: 32}
            for i in tqdm(range(n_div), token=self.TELEGRAM_TOKEN, chat_id=self.CHAT_ID):
                node = heap.pop()
                node.divide()
                for child in node.childs:
                    # print(child.x, child.y, child.size)
                    t = find_block_transform(
                        image, image_2, resized_image, resized_image_2,
                        child.x, child.y, child.size, stride.get(child.size, 16)
                    )
                    # print(t)
                    child.t = t
                    heap.insert(child)
            trans = tree.flatten()
            return trans

        im = image.astype('float32')
        rim = np.rint(resize(im, (128, 128))).astype('float32')
        if is_colored(im):
            im = (rgb2yuv(im))
            rim = (rgb2yuv(rim))
            im_channels = [im[:, :, i] for i in range(3)]
            rim_channels = [rim[:, :, i] for i in range(3)]

            t1 = encode(im_channels[0], im_channels[0] ** 2, rim_channels[0], rim_channels[0] ** 2,
                        n_div)
            t2 = encode(im_channels[1], im_channels[1] ** 2, rim_channels[0], rim_channels[0] ** 2,
                        n_div // 8)
            t3 = encode(im_channels[2], im_channels[2] ** 2, rim_channels[0], rim_channels[0] ** 2,
                        n_div // 8)
            # return self._ifs2buf(params, [t1, t2, t3]), (params, [t1, t2, t3])
            return self._ifs2buf(params, [t1, t2, t3])

    def decompress(self, byte_array, num_iters=10):
        params, t = self._buf2ifs(BitBuffer(byte_array))
        if params.is_colored:
            im_channels = [np.ones((256, 256)).astype('float32') * 128 for _ in range(3)]
            rim_channels = [np.zeros((128, 128)).astype('float32') * 128 for _ in range(3)]
            for n_ch in [0]:
                for i in range(num_iters):
                    im_channels[n_ch] = perform_transform(im_channels[n_ch], rim_channels[n_ch], t[n_ch],
                                                          params.height)
                    rim_channels[n_ch] = resize(im_channels[n_ch], (128, 128))
            for n_ch in [1, 2]:
                im_channels[n_ch] = perform_transform(im_channels[0], rim_channels[0], t[n_ch], params.height)
            image = np.dstack(tuple(im_channels[i] for i in range(3)))
            image = yuv2rgb(image)
            for i in range(256):
                for j in range(256):
                    for k in range(3):
                        if image[i][j][k] < 0:
                            image[i][j][k] = 0
                        elif image[i][j][k] > 255:
                            image[i][j][k] = 255
            return np.rint(image).astype('uint8')
        else:
            t = t[0]
            image = np.zeros((params.height, params.width))
            resized_image = resize(image, (params.height // 2, params.width // 2))
            for i in range(num_iters):
                image = perform_transform(image, resized_image, t, params.block_size)
                resized_image = resize(image, (params.height // 2, params.width // 2))
        return image


def weighted_psnr(ref, img):
    assert ref.shape == img.shape, "Shape mismatch"
    if is_colored(img):
        ref_yuv = rgb2yuv(ref)
        img_yuv = rgb2yuv(img)

        return (4 * psnr(ref_yuv[..., 0], img_yuv[..., 0]) +
                psnr(ref_yuv[..., 1], img_yuv[..., 1]) +
                psnr(ref_yuv[..., 2], img_yuv[..., 2])
                ) / 6
    else:
        return psnr(ref, img)
