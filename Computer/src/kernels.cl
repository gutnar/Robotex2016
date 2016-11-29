__kernel void ndrange_parallelism () {
    int i = get_global_id(0);
    int j = get_global_id(1);

    printf("%d\n", i*j);
}

__kernel void mark_pixels (__global const int* in, __global int* out) {
    int x = get_global_id(0);
    int y = get_global_id(1);

    int i = y*640 + x;
    int j = i*3;

    // Glass
    if (y > 430 && ((x > 75 && x < 200) || (x > 445 && x < 570))) {
        out[i] = 4;
        return;
    }

    // Dribbler
    if (y > 450 && x >= 200 && x <= 445) {
        out[i] = 4;
        return;
    }

    int h = in[j + 0];
    int s = in[j + 1];
    int v = in[j + 2];

    // Light color
    if (v >= 175) {
        // Most likely yellow or orange
        if (h < 100) {
            out[i] = 1; // orange = 1
        }

        // Most likely white
        else {
            out[i] = 0; // white = 0
        }
    }

    // Most likely black, could be blue or green
    else if (v <= 50) {
        if (v >= 40 && s >= 175) {
            out[i] = 2; // blue = 2
        } else {
            out[i] = 3; // black = 3
        }
    }

    // Most likely blue
    else if (h >= 100 && h <= 130 && s >= 140) {
        out[i] = 2;
    }

    // Most likely green
    else {
        out[i] = 4;
    }
}