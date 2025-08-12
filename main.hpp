#ifndef main_hpp
#define main_hpp

#include <string>

int main(int argc, char *argv[]);

std::pair<std::pair<int, int>, std::pair<int, int>> GetPositions(std::vector<int> lines, int start, int end);

#endif