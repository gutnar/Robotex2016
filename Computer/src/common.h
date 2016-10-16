//
// Created by jk on 16.10.16.
//

#ifndef ROBOTEX2016_COMMON_H
#define ROBOTEX2016_COMMON_H

#define IMAGE_WIDTH 640
#define IMAGE_HALF_WIDTH 320
#define IMAGE_HEIGHT 480

inline std::string itos(int n)
{
    const int max_size = std::numeric_limits<int>::digits10 + 1 + 1;
    char buffer[max_size] = {0};
    sprintf(buffer, "%d", n);
    return std::string(buffer);
}

#endif //ROBOTEX2016_COMMON_H
