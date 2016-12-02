__kernel void ndrange_parallelism () {
    int i = get_global_id(0);
    int j = get_global_id(1);

    printf("%d\n", i*j);
}

/*
[WHITE]
H_MIN = 136
H_MAX = 164
S_MIN = 17
S_MAX = 45
V_MIN = 255
V_MAX = 255

[ORANGE]
H_MIN = 0
H_MAX = 16
S_MIN = 128
S_MAX = 288
V_MIN = 220
V_MAX = 288


[YELLOW]
H_MIN = 9
H_MAX = 13
S_MIN = 149
S_MAX = 213
V_MIN = 202
V_MAX = 294


[BLUE]
H_MIN = 90
H_MAX = 142
S_MIN = 159
S_MAX = 211
V_MIN = 124
V_MAX = 208


[BLACK]
H_MIN = 4
H_MAX = 276
S_MIN = 99
S_MAX = 183
V_MIN = 46
V_MAX = 82


[GREEN]
H_MIN = 67
H_MAX = 87
S_MIN = 35
S_MAX = 83
V_MIN = 102
V_MAX = 150
*/

__kernel void mark_pixels(__global const int* in, __global int* out) {
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
            out[i] = 5; // orange = 5
        }

        // Most likely white
        else {
            out[i] = 0; // white = 0
        }
    }

    // Most likely black, could be blue or green (ENNE OLI 50, GRETE TRENNI AJAL PANIN 70)
    else if (v <= 70) {
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

__kernel void mark_pixels_new(__global const int* in, __global int* out, __global const int* colors) {
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

    // Pixel value
    int h = in[j + 0];
    int s = in[j + 1];
    int v = in[j + 2];

    // Lowest difference and closest color index
    int closestColor = 0;
    float minDifference = 5000;

    // Compare to colors
    for (int c = 0; c < 6; ++c) {
        float difference = (float) (h - colors[c*3+0])*(h - colors[c*3+0]) / 180/180 +
                           (float) (s - colors[c*3+1])*(s - colors[c*3+1]) / 255/255 +
                           (float) (v - colors[c*3+2])*(v - colors[c*3+2]) / 255/255;

        if (difference < minDifference) {
            minDifference = difference;
            closestColor = c;
        }
    }

    /*
    if (closestColor == 1) {
        closestColor = 5;
    }
    */

    out[i] = closestColor;
}


__kernel void mark_pixels_grete (__global const int* in, __global int* out) {
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
    // Black
    if (v < 50) {
        out[i] = 3;
    }
    // White
     else if (s < 80) {
        out[i] = 0;
     }

     else if (v < 185 && v > 30 && s > 80) {
        // Green
        if (s < 200) {
            out[i] = 4;
            }
         // Blue
         else {
            out[i] = 2;
         }
     }
     else {
        // Yellow
        if (v > 234) {
            out[i] = 5;
        }
        // Orange
        else {
            out[i] = 1;
        }
     }

}

__kernel void mark_pixels_ranges(__global const int* in, __global int* out, __global const int* colors) {
    int x = get_global_id(0);
    int y = get_global_id(1);

    int i = y*640 + x;
    int j = i*3;

    // Glass
    if (y > 430 && ((x > 75 && x < 200) || (x > 445 && x < 570))) {
        out[i] = 4;
        return;
    }

    int h = in[j + 0];
    int s = in[j + 1];
    int v = in[j + 2];

    out[i] = 3;

    // Compare to colors
    for (int c = 0; c < 6; ++c) {
        if (c == 3) continue;

        if (h < colors[c*6+0] || h > colors[c*6+1]) continue;
        if (s < colors[c*6+2] || s > colors[c*6+3]) continue;
        //if (v < colors[c*6+4] || v > colors[c*6+5]) continue;

        out[i] = c;
        break;
    }
}
