# GRADED CELL: BitBuffer
import numpy as np


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
        self.rescale = 6
        self.body = np.zeros(self.rescale, dtype=np.int8)
        self.pos = 0
        self.chunk = 8
        self.size = self.chunk * self.rescale

    def to_bytearray(self):
        '''Convert to bytearray.

        Returns
        -------
        buffer: bytearray
            Bytearray that contains all data.
        '''

        return self._buffer

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
        if self.pos + n_bits >= self.size:
            self.body = np.concatenate((self.body, np.zeros(self.rescale, dtype=np.int64)))
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
            self.body[i // self.chunk] &= ~np.int64(bit)
        self.pos -= n_bits
        return x

a = BitBuffer()
x = np.float16(1/3)
a.push(x, 16)

# from random import random, seed
# seed(0)
#
#
# def rand():
#     return int(random() * 1000000)
#
# def size(x):
#     return int(np.ceil(np.log2(x + 1)) + 1)
#
# arr = [rand() for i in range(100)]
# for i in range(100):
#     a.push(arr[i], size(arr[i]))
# for i in range(99, -1, -1):
#     x = arr[i]
#     y = a.pop(size(x))
#     print(x, y)
#     if x != y:
#         print('f{x} != {y}')
#         break





