import re
from matplotlib import pyplot as plt

filePath_angle = "./angle_map_0605_33.txt"
def read_angleMap():
    angle_map = []
    with open(filePath_angle, "r", ) as f:
        lines = f.readlines()
        lines = [line.strip().replace("'", "\"") for line in lines]
        for line in lines:
            (_, x, y) = list(map(float, re.split(r'[,:]', line)))
            angle_map.append((x, y))
    return angle_map

angle_map = read_angleMap()

x = []
y = []
for angle in angle_map:
    x.append(angle[0])
    y.append(angle[1])

plt.figure(figsize=(10, 10))
for i in range(9):
    plt.text(y[i], x[i], "num"+str(i))

plt.plot(y, x, 'sr', markersize=10)
plt.show()
