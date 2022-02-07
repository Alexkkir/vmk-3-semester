import matplotlib.pyplot as plt
import numpy as np

with open(r"../stuff/prak.csv", "r") as f:
    lines = f.readlines()

def to_int(x):
    try:
        return int(x)
    except:
        return None

raw_vals = [line.split()[1].split(',')[1] for line in  lines[1:]]
vals = np.array([to_int(x) for x in raw_vals if to_int(x) is not None])
length = len(vals)
vals.sort()

def window_medium(sor_arr, win_size):
    bias = win_size / 2
    ranges = [(int(max(0, i-bias)), int(min(i+bias, length - 1))) for i in range(length)]
    med_vals = [np.mean(vals[a: b]) for a, b in  ranges]
    return med_vals


n_subplots = 2

plt.subplot(1, n_subplots, 1)
plt.hist(vals, bins = 'auto')
# plt.axvline(x = np.median(vals), color = 'black', linestyle = '--', label = 'median')
plt.axvline(x = np.mean(vals), color = 'blue', linestyle = '--', label = 'Среднее')
plt.legend()
plt.title("Распределение по баллам")
plt.xlabel("баллы")
plt.ylabel("число людей\nв заданном диапазоне")

plt.subplot(1, n_subplots, 2)
data = window_medium(vals, 8)
plt.plot(range(length), data)
plt.axhline(y = np.mean(data), color = 'blue', linestyle = '--', label = 'Среднее')
plt.title("Отсрортированные значение баллов за прак")
plt.xlabel("порядковый номер человека")
plt.ylabel("баллы")
plt.legend()
plt.show()
