import csv
import os
import platform
import argparse
import json
import time
import signal
from contextlib import contextmanager
from skimage.metrics import structural_similarity as compare_ssim, peak_signal_noise_ratio as compare_psnr
from tests.src import *
from tests.command import Command


DIR_TESTS = os.path.split(os.path.abspath(__file__))[0]
DIR_ROOT = os.path.normpath(os.path.join(DIR_TESTS, os.pardir))
DIR_TEST_FILES = os.path.join(DIR_ROOT, 'video')
FILE_RESULTS = os.path.join(DIR_ROOT, 'results.csv')

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

# Простая функция для тестирования ME.
def test_me(source_video, me):
    ssim = []
    psnr = []
    speed = []

    for frame, prev_frame in generate_pair_frames_gray(source_video):
        start = time.time()
        result = me.Estimate(frame, prev_frame) # Запускаем me
        end = time.time()
        speed.append((end - start) * 1000)
        compensated_frame = result.Remap(prev_frame)

        ssim.append(compare_ssim(frame, compensated_frame, multichannel=False))
        psnr.append(compare_psnr(frame, compensated_frame))

    return {
        'ssim': ssim,
        'psnr': psnr,
        'speed': speed,
    }

def run_tests(timeout=480, n_runs=3, qualities=[80], halfpixels=[False, True]):
    result = []
    video_paths = [os.path.join(DIR_TEST_FILES, x) for x in os.listdir(DIR_TEST_FILES)]

    for video_path in video_paths:
        for halfpixel in halfpixels:
            for quality in qualities:

                conclusion = ""
                psnr = []
                ssim = []
                speed = []

                print(f"Testing ME on {video_path} with quality {quality} and halfpixel={halfpixel}")

                for i in range(n_runs):
                    video = VideoReader(video_path)
                    if video.width == 0 or video.height == 0:
                        print(f"Run {i} failed: {video_path} is not a valid video file")
                        break

                    try:
                        with time_limit(timeout):
                            me = MotionEstimator(video.width, video.height, quality, halfpixel)
                            metrics = test_me(video, me)
                            ssim.append(np.mean(metrics['ssim']))
                            psnr.append(np.mean(metrics['psnr']))
                            speed.append(np.mean(metrics['speed']))
                            conclusion = "OK"
                    except TimeoutException:
                        # Считаем OK, если хотя бы одна попытка запуска успешна
                        conclusion = "OK" if conclusion == "OK" else "TL"
                    except Exception as e:
                        conclusion = "OK" if conclusion == "OK" else "RE"

                if speed:
                    idx = np.argmin(speed)
                    speed_std = np.std(speed)
                    speed = speed[idx]
                    psnr = psnr[idx]
                    ssim = ssim[idx]
                else:
                    speed = None
                    speed_std = None
                    psnr = None
                    ssim = None

                result.append(
                    {
                        'conclusion': conclusion,
                        'video': video_path,
                        'halfpixel': halfpixel,
                        'quality': quality,
                        'psnr': psnr,
                        'ssim': ssim,
                        'speed': speed,
                        'speed_std': speed_std
                    }
                )

    return pd.DataFrame(result)

def main():
    parser = argparse.ArgumentParser(description='Testing script', prog='test')
    parser.add_argument('--timeout', type=float, default=480)
    args = parser.parse_args()

    err_code = Command(["python3", "setup.py", "build_ext", "-i"]).run(
                        output_file="compile_log",
                        timeout=args.timeout,
                        working_directory=DIR_ROOT)

    if err_code != "OK":
        print('Failed to compile')
        print(err_code)
        return

    try:
        results = run_tests(timeout=args.timeout)
    except TestDirectoryNotFoundError as e:
        print('Failed to find test directory')
        print(e)
        return

    results.to_csv(FILE_RESULTS, index  =False)
    print('Results saved to: {}'.format(FILE_RESULTS))

if __name__ == '__main__':
    main()
