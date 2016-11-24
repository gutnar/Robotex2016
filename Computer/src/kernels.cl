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

    int h = in[j + 0];
    int s = in[j + 1];
    int v = in[j + 2];

    /*
    if (v < 50) {
        out[y*640 + x] = 0;
    } else if (v < 100) {
         out[y*640 + x] = 1;
     } else if (v < 150) {
          out[y*640 + x] = 2;
      } else if (v < 200) {
       out[y*640 + x] = 3;
   } else {
            out[y*640 + x] = 4;
        }
    */

    // Light color
    if (v >= 200) {
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