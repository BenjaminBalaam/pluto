#include <string>

enum ERROR_TYPE
{
    SyntaxError,
};

struct Error
{
public:
    ERROR_TYPE type;
    std::string text;

};