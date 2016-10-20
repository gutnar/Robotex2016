# Open a file_
f = open("checkerboard.txt")

distances = []
pixels = []
sumPixels, count = 0, 0

lineNumber = 0
lines = f.readlines()

for rawline in lines:
    line = rawline.strip().split(" ")
    lineNumber += 1

    if len(line) == 1 and line[0] != "":
        distances.append(float(line[0]))
    elif len(line) == 2:
        sumPixels += 480 - float(line[1])
        count += 1

    if line[0] == "" or lineNumber == len(lines):
        pixels.append(sumPixels/count)
        sumPixels, count = 0, 0

f.close()

# Least squares
N = len(distances)

a = N
b = sum([1/y for y in pixels])
c = -sum(distances)
d = sum([1/y/y for y in pixels])
e = -sum([distances[i]/pixels[i] for i in range(N)])

B = (-e+b*c/a)/(d-b**2/a)
A = (-c-b*B)/a

print("A", A, "B", B)

for i in range(N):
    print(distances[i], A+B/pixels[i])

print(A+B/(480-207))
