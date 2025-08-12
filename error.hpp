#ifndef error_hpp
#define error_hpp

#include <string>

enum ERROR_TYPE
{
    SyntaxError,
};

std::ostream& operator<<(std::ostream& os, const ERROR_TYPE& e);

struct Error
{
public:
    ERROR_TYPE type;
    std::string text;

};

#endif