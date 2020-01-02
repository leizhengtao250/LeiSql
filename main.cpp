
#include "project/src/Hash/Extendible_hash.h"
#include "project/src/Hash/Extendible_hash.cpp"
#include <map>
#include <iostream>
#include <cmath>
#include <vector>

int main() {

    std::unique_ptr<int> a = std::make_unique<int>(4);

    std::cout<< *a;

    return 0;
}
