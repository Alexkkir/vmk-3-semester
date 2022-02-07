import matplotlib.pyplot as plt
import numpy as np

with open("output.csv", 'r') as f:
    lines = f.readlines()


def to_int(x):
    try:
        return int(x)
    except:
        return None


nums = [to_int(x) for x in lines if to_int(x) is not None]

med = np.average(nums)

_ = plt.hist(nums, bins='auto')
plt.xlim([0, 50 * 64])
plt.axvline(x=med, color="black", linestyle="--")
plt.title("MSE distr")
plt.show()
print("success end")
