#include <string>

#include "error.hpp"

int x = 0;

std::string Keywords[] = { "if" };

class Token
{
    public:
        Error error;

        int start;
        int end;
};

class Integer : public Token
{
    public:
        int number;

        Integer(int number);
};

class Float : public Token
{
    public:
        float number;

        Float(float number);
};

class String : public Token
{
    public:
        std::string content;

        String(std::string content);
};

class Identifier : public Token
{
    public:
        std::string name;
        
        Identifier(std::string name);
};

class Keyword : public Token
{
    public:
        std::string name;

        Keyword(std::string name);
};