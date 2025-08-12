#ifndef error_hpp
#define error_hpp

#include <string>
#include <vector>

enum ERROR_TYPE
{
    SyntaxError,
    IdentifierError,
    TypeError,
};

std::ostream &operator<<(std::ostream &os, const ERROR_TYPE &e);

struct Error
{
public:
    ERROR_TYPE type;
    std::string text;

};

int ThrowError(Error error, int start, int end, std::vector<int> line_numbers, std::vector<std::string> lines);

std::pair<std::pair<int, int>, std::pair<int, int>> GetPositions(std::vector<int> lines, int start, int end);

#endif