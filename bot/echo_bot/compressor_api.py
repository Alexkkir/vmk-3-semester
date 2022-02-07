import fractal_compressor as fc
from skimage.transform import resize
import matplotlib.pyplot as plt
from skimage import io
import numpy as np
from PIL import Image


def perform_compression(path2image, path2bin, quality, token, chat_id):
    image = io.imread(path2image)
    image = np.rint(resize(image, (256, 256)) * 255).astype('uint8')

    comp = fc.FractalCompressor(token, chat_id)
    encoded = comp.compress2(image, quality=quality)
    dec_im = comp.decompress(encoded)

    plt.figure(figsize=(12, 6))

    plt.subplot(1, 2, 1)
    plt.title("Original", fontsize=20)
    plt.imshow(image)
    plt.xticks([])
    plt.yticks([])

    plt.subplot(1, 2, 2)
    plt.title("Compressed", fontsize=20)
    plt.imshow(dec_im)
    plt.xticks([])
    plt.yticks([])
    plt.xlabel(f'PSNR = %.3f' % fc.weighted_psnr(image, dec_im), fontsize=20)
    # plt.show()

    plt.savefig(path2image)
    with open(path2bin, 'wb') as new_file:
        new_file.write(encoded)


def perform_decompression(path2bin, path2image, token, chat_id):
    encoded = bytearray(open(path2bin, 'rb').read())

    comp = fc.FractalCompressor(token, chat_id)
    dec_im = comp.decompress(encoded)

    # plt.figure(figsize=(6, 6))
    # # plt.title("Decompressed")
    # plt.imshow(dec_im)
    # plt.xticks([])
    # plt.yticks([])
    # plt.xlim(0, dec_im.shape[0])
    # plt.ylim(0, dec_im.shape[1])
    #
    # plt.savefig(path2image)
    plt.imsave(path2image, dec_im)
