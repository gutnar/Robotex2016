//
// Created by jk on 16.10.16.
//

#ifndef ROBOTEX2016_COMMON_H
#define ROBOTEX2016_COMMON_H

#include <string>
#include <limits>

#define IMAGE_WIDTH 640
#define IMAGE_HALF_WIDTH 320
#define IMAGE_HEIGHT 480
#define IMAGE_PIXELS 307200

#define DISTANCE_A -48.822563620046886
#define DISTANCE_B 25192.00778679133
#define DISTANCE_C 34.17273426556493

inline std::string itos(int n)
{
    const int max_size = std::numeric_limits<int>::digits10 + 1 + 1;
    char buffer[max_size] = {0};
    sprintf(buffer, "%d", n);
    return std::string(buffer);
}

#define NUMBER_OF_COLORS 6
std::string const COLORS[NUMBER_OF_COLORS] = {"BLACK", "BLUE","GREEN","ORANGE","YELLOW","WHITE"};

#endif //ROBOTEX2016_COMMON_H
