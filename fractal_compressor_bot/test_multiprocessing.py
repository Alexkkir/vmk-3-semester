from multiprocessing import Process, Queue
from time import sleep


def f():
    for i in range(8):
        print(i)
        sleep(0.5)


def g(pid: Process):
    sleep(2)
    p.terminate()


def main():
    p = Process(target=f)
    p.start()

    p2 = Process(target=g, args=(p.pid,))
    p2.start()

    p.join()
    p2.join()

    print('end')


if __name__ == '__main__':
    main()
