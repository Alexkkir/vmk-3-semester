import csv
import os
import argparse
import signal
from contextlib import contextmanager
from skimage.transform import resize
from skimage import io
import numpy as np
from skimage.metrics import peak_signal_noise_ratio as psnr


FILE = 'file'
QUALITY = 'quality'
ORIGINAL_SIZE = 'original_size'
COMPRESSED_SIZE = 'compressed_size'
PSNR = 'psnr'
CONCLUSION = 'conclusion'

DIR_TEST_FILES = 'test_files'
FILE_RESULTS = 'results.csv'


class TestDirectoryNotFoundError(FileNotFoundError):
    pass


class TimeoutException(Exception): pass


@contextmanager
def time_limit(seconds):
    def signal_handler(signum, frame):
        raise TimeoutException("Timed out!")
    signal.signal(signal.SIGALRM, signal_handler)
    signal.alarm(seconds)
    try:
        yield
    finally:
        signal.alarm(0)


quality = [0, 20, 40, 60, 80, 100]


def test_image(dir, name_img, timeout):
    img = np.rint(resize(io.imread(os.path.join(dir, name_img)), (256, 256)) * 255).astype('uint8')
    comp = FractalCompressor()
    results = []
    for q in quality:
        try:
            with time_limit(timeout):
                compressed_image = comp.compress2(img, quality=q)
                if not isinstance(compressed_image, bytearray):
                    results.append((name_img, q, None, None, None, 'FE'))
                decompressed_image = comp.decompress(compressed_image)
            if is_colored(img):
                results.append((name_img, q, img.size, len(compressed_image), weighted_psnr(img, decompressed_image), 'OK'))
            else:
                results.append((name_img, q, img.size, len(compressed_image), psnr(img, decompressed_image), 'OK'))
        except TimeoutException:
            results.append((name_img, q, None, None, None, 'TL'))
        except Exception as e:
            print(e)
            results.append((name_img, q, None, None, None, 'RT'))

    return results


def run_tests(timeout=300):
    results = []
    
    for name in os.listdir(DIR_TEST_FILES):
        results += test_image(DIR_TEST_FILES, name, timeout)
        
    return tuple(results)


def save_results(results, filename_score=FILE_RESULTS):
    header = [FILE,
              QUALITY,
              ORIGINAL_SIZE,
              COMPRESSED_SIZE,
              PSNR,
              CONCLUSION]
    with open(filename_score, 'w', newline='') as resfile:
        writer = csv.writer(resfile)
        writer.writerow(header)
        resfile.flush()
        for row in results:
            writer.writerow(row)
            resfile.flush()


def main():
    parser = argparse.ArgumentParser(description='Testing script')
    parser.add_argument('--timeout', type=float, default=300)

    args = parser.parse_args()

    try:
        results = run_tests(timeout=args.timeout)
    except TestDirectoryNotFoundError as e:
        print('Failed to find test directory')
        print(e)
        return

    save_results(results)
    print('Results saved to: {}'.format(FILE_RESULTS))


if __name__ == '__main__':
    main()
