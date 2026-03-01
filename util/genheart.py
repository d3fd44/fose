import numpy as np

t = np.linspace(0, 2*np.pi, 512, endpoint=False)

xt = [16 * (np.sin(i) ** 3) for i in t]
yt = [13 * np.cos(i) - 5 * np.cos(2 * i) - 2 * np.cos(3 * i) - np.cos(4 * i) for i in t]

for i,j in zip(xt, yt):
    print(f"{i:.15f}, {j:.15f}")
