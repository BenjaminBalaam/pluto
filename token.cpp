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
    else if (t.type == "Control")
    {
        return os << (Control&)t;
    }
    else if (t.type == "Bracket")
    {
        return os << (Bracket&)t;
    }
    else if (t.type == "Operator")
    {
        return os << (Operator&)t;
    }
    
    return os;
}

Integer::Integer(int number) : number(number)
{
    this->type = "Integer";
}

ostream& operator<<(ostream& os, const Integer& i)
{
    return os << i.number;
}

Float::Float(float number) : number(number)
{
    this->type = "Float";
}

ostream& operator<<(ostream& os, const Float& f)
{
    return os << f.number;
}

String::String(string content) : content(content)
{
    this->type = "String";
}

ostream& operator<<(ostream& os, const String& s)
{
    return os << "\"" << s.content << "\"";
}

Identifier::Identifier(string name) : name(name)
{
    this->type = "Identifier";
}

ostream& operator<<(ostream& os, const Identifier& id)
{
    return os << id.name;
}

Keyword::Keyword(string name) : name(name)
{
    this->type = "Keyword";
}

ostream& operator<<(ostream& os, const Keyword& k)
{
    return os << k.name;
}

Control::Control(string value) : value(value)
{
    this->type = "Control";
}

ostream& operator<<(ostream& os, const Control& c)
{
    return os << c.value;
}

Bracket::Bracket(string value) : value(value)
{
    this->type = "Bracket";
}

ostream& operator<<(ostream& os, const Bracket& b)
{
    return os << b.value;
}

Operator::Operator(string value) : value(value)
{
    this->type = "Operator";
}

ostream& operator<<(ostream& os, const Operator& o)
{
    return os << o.value;
}