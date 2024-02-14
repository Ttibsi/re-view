#include "utils.h"

std::string repeat(int count, std::string c) {
    std::string ret = "";
    for (int i = 0; i < count; i++)
        ret += c;
    return ret;
}
