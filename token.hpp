#ifndef token_hpp
#define token_hpp

#include <string>
#include <vector>
#include <iostream>
#include <optional>

#include "error.hpp"

extern std::vector<std::string> Keywords;

class Token
{
    public:
        std::optional<Error> error;

        std::string type;
        int start;
        int end;

        friend std::ostream& operator<<(std::ostream& os, const Token& t);
};

class Integer : public Token
{
    public:
        int number;

        Integer(int number);

        friend std::ostream& operator<<(std::ostream& os, const Integer& i);
};

class Float : public Token
{
    public:
        float number;

        Float(float number);

        friend std::ostream& operator<<(std::ostream& os, const Float& f);
};

class String : public Token
{
    public:
        std::string content;

        String(std::string content);

        friend std::ostream& operator<<(std::ostream& os, const String& s);
};

class Identifier : public Token
{
    public:
        std::string name;
        
        Identifier(std::string name);

        friend std::ostream& operator<<(std::ostream& os, const Identifier& id);
};

class Keyword : public Token
{
    public:
        std::string name;

        Keyword(std::string name);

        friend std::ostream& operator<<(std::ostream& os, const Keyword& k);
};

class Control : public Token
{
    public:
        std::string value;

        Control(std::string value);

        friend std::ostream& operator<<(std::ostream& os, const Control& c);
};

class Bracket : public Token
{
    public:
        std::string value;

        Bracket(std::string value);

        friend std::ostream& operator<<(std::ostream& os, const Bracket& b);
};

class Operator : public Token
{
    public:
        std::string value;

        Operator(std::string value);

        friend std::ostream& operator<<(std::ostream& os, const Operator& o);
};

std::tuple<std::string, int, float> GetTokenValue(Token* token);

#endif