n = 2
diap = [*range(-n, n + 1, 1)]
from copy import deepcopy
template = ['Empty'] * len(diap)
arr = []
for i in range(len(diap)):
    arr.append(template[:])

class Dir:
    def __init__(self):
        self.dir = "Right"
    def rotate(self):
        if self.dir == 'Right':
            self.dir = 'Down'
        elif self.dir == 'Down':
            self.dir = 'Left'
        elif self.dir == 'Left':
            self.dir = 'Up'
        elif self.dir == 'Up':
            self.dir = 'Right'

class Walker:
    def __init__(self):
        self.dir = Dir()
        self.i = 0
        self.j = 0
        self.count = 0
    def next(self):
        if self.dir.dir == 'Right':
            return  self.i, self.j + 1
        elif self.dir.dir == 'Down':
            return self.i + 1, self.j
        elif self.dir.dir == 'Left':
            return self.i, self.j - 1
        elif self.dir.dir == 'Up':
            return self.i - 1, self.j

    def step(self, arr):
        arr[self.i][self.j] = (self.i - n, self.j - n)
        self.count += 1
        i_new, j_new = self.next()
        if not 0 <= i_new <= 2 * n or not 0 <= j_new <= 2 * n:
            self.dir.rotate()
            i_new, j_new = self.next()
        elif arr[i_new][j_new] != 'Empty':
            self.dir.rotate()
            i_new, j_new = self.next()
        self.i = i_new
        self.j = j_new
        if arr[self.i][self.j] == 'Empty':
            return 0
        else:
            return -1

w = Walker()
while w.step(arr) != -1: pass

for line in arr:
    for elem in line:
        print(f"std::pair<int, int> {{{elem[0]}, {elem[1]}}}", end = ", ")
