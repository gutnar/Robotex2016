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

//#define DISTANCE_A -48.822563620046886
//#define DISTANCE_B 25192.00778679133
//#define DISTANCE_C 34.17273426556493

//A -38.14499320591934 B 19717.04509951454
//C 28.465393566494686
#define DISTANCE_A -31.117684461866233
#define DISTANCE_B 16937.139960237193
#define DISTANCE_C 29.455683523836083
//28.465393566494686 A -31.117684461866233 B 16937.139960237193

#define ROBOT_RADIUS 13
#define WHEEL_0 2.61799388
#define WHEEL_1 0.523598776
#define WHEEL_2 4.71238898

inline std::string itos(int n)
{
    const int max_size = std::numeric_limits<int>::digits10 + 1 + 1;
    char buffer[max_size] = {0};
    sprintf(buffer, "%d", n);
    return std::string(buffer);
}

#define NUMBER_OF_COLORS 6
std::string const COLORS[NUMBER_OF_COLORS] = {"WHITE", "YELLOW", "BLUE", "BLACK", "GREEN", "ORANGE"};

#endif //ROBOTEX2016_COMMON_H
