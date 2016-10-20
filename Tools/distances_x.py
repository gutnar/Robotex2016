# Open a file_
f = open("checkerboard-x.txt")

pixelsX, pixelsY, distances = [], [], []

lineNumber = 0
lines = f.readlines()
distance = 0
for rawline in lines:
    line = rawline.strip().split(" ")
    lineNumber += 1

    if len(line) == 1 and line[0] != "":
        distance = float(line[0])
    elif len(line) == 2:
        pixelsY.append(480 - float(line[1]))
        pixelsX.append(float(line[0]))
        distances.append(distance)

f.close()

# Least squares
sum1, sum2 = 0, 0
for i in range(len(distances)):
    sum1 += distances[i]*pixelsX[i]/pixelsY[i]
    sum2 += pixelsX[i]**2/pixelsY[i]**2
C = sum1/sum2
print("C", C)

for i in range(len(distances)):
    print(distances[i], C*pixelsX[i]/pixelsY[i])
