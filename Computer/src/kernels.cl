__kernel void ndrange_parallelism () {
    int i = get_global_id(0);
    int j = get_global_id(1);

    printf("%d\n", i*j);
}

__kernel void mark_pixels_physicum(__global const int* in, __global int* out) {
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
        // Most likely yellow or orange (WAS 100 BEFORE)
        if (h < 30 && s > 100) {
            out[i] = 5; // orange = 5
        }

        // Most likely white
        else {
            out[i] = 0; // white = 0
        }
    }

    // Most likely black, could be blue or green (ENNE OLI 50, GRETE TRENNI AJAL PANIN 70)
    else if (v <= 100) {
        /*
        if (v >= 80 && s >= 175) {
            out[i] = 2; // blue = 2
        } else {
            out[i] = 3; // black = 3
        }
        */
        if (h < 100) {
            out[i] = 4;
        } else {
            out[i] = 3;
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

    // Yellow or orange
    if (h < 50 && s > 120 && v > 150) {
        out[i] = 5;
    }

    // Blue
        else if (h > 90 && h < 150 && s > 140) {
            out[i] = 2;
        }

    // Green
    else if (h < 100 && v < 200) {
        out[i] = 4;
    }


    // White
    else if (s < 150 && v > 100) {
        out[i] = 0;
    }

    // Unknown
    else {
        out[i] = 3;
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
