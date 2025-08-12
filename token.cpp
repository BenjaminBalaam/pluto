#include <iostream>
#include <string>

#include "token.hpp"

using namespace std;

vector<string> Keywords = { "if" };

ostream& operator<<(ostream& os, const Token& t)
{
    if (t.type == "Integer")
    {
        return os << (Integer&)t;
    }
    else if (t.type == "Float")
    {
        return os << (Float&)t;
    }
    else if (t.type == "String")
    {
        return os << (String&)t;
    }
    else if (t.type == "Identifier")
    {
        return os << (Identifier&)t;
    }
    else if (t.type == "Keyword")
    {
        return os << (Keyword&)t;
    }
    
    return os;
}

Integer::Integer(int number) : number(number)
{
    this->type = "Integer";
}

ostream& operator<<(ostream& os, Integer const& i)
{
    return os << i.number;
}

Float::Float(float number) : number(number)
{
    this->type = "Float";
}

ostream& operator<<(ostream& os, Float const& f)
{
    return os << f.number;
}

String::String(string content) : content(content)
{
    this->type = "String";
}

ostream& operator<<(ostream& os, String const& s)
{
    return os << "\"" << s.content << "\"";
}

Identifier::Identifier(string name) : name(name)
{
    this->type = "Identifier";
}

ostream& operator<<(ostream& os, Identifier const& id)
{
    return os << id.name;
}

Keyword::Keyword(string name) : name(name)
{
    this->type = "Keyword";
}

ostream& operator<<(ostream& os, Keyword const& k)
{
    return os << k.name;
}