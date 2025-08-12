#include <iostream>
#include <string>
#include <tuple>

#include "token.hpp"
#include "error.hpp"

using namespace std;

vector<string> Keywords = { "public", "static", "const", "if", "else", "switch", "case", "default", "for", "while" };

ostream &operator<<(ostream &os, const Token &t)
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

bool Token::operator==(const Token &other)
{
    if (this->type != other.type)
    {
        return false;
    }

    if (this->type == "Keyword")
    {
        if (((Keyword*)this)->name != ((Keyword&)other).name)
        {
            return false;
        }
    }
    else if (this->type == "Control")
    {
        if (((Control*)this)->value != ((Control&)other).value)
        {
            return false;
        }
    }
    else if (this->type == "Bracket")
    {
        if (((Bracket*)this)->value != ((Bracket&)other).value)
        {
            return false;
        }
    }
    else if (this->type == "Operator")
    {
        if (((Operator*)this)->value == "")
        {
            return true;
        }

        if (((Operator*)this)->value != ((Operator&)other).value)
        {
            return false;
        }
    }

    return true;
}

Integer::Integer(int number) : number(number)
{
    this->type = "Integer";
}

ostream &operator<<(ostream &os, const Integer &i)
{
    return os << i.number;
}

Float::Float(double number) : number(number)
{
    this->type = "Float";
}

ostream &operator<<(ostream &os, const Float &f)
{
    return os << f.number;
}

String::String(string content) : content(content)
{
    this->type = "String";
}

ostream &operator<<(ostream &os, const String &s)
{
    return os << "\"" << s.content << "\"";
}

Identifier::Identifier(string name) : name(name)
{
    this->type = "Identifier";
}

ostream &operator<<(ostream &os, const Identifier &id)
{
    return os << id.name;
}

Keyword::Keyword(string name) : name(name)
{
    this->type = "Keyword";
}

ostream &operator<<(ostream &os, const Keyword &k)
{
    return os << k.name;
}

Control::Control(string value) : value(value)
{
    this->type = "Control";
}

ostream &operator<<(ostream &os, const Control &c)
{
    return os << c.value;
}

Bracket::Bracket(string value) : value(value)
{
    this->type = "Bracket";
}

ostream &operator<<(ostream &os, const Bracket &b)
{
    return os << b.value;
}

Operator::Operator(string value) : value(value)
{
    this->type = "Operator";
}

ostream &operator<<(ostream &os, const Operator &o)
{
    return os << o.value;
}

tuple<string, int, double> GetTokenValue(Token *token)
{
    if (token->type == "Integer")
    {
        return tuple<string, int, double> { "", ((Integer*)token)->number, 0.0f };
    }
    else if (token->type == "Float")
    {
        return tuple<string, int, double> { "", 0.0f, ((Float*)token)->number };
    }
    else if (token->type == "String")
    {
        return tuple<string, int, double> { ((String*)token)->content, 0, 0.0f };
    }
    else if (token->type == "Identifier")
    {
        return tuple<string, int, double> { ((Identifier*)token)->name, 0, 0.0f };
    }
    else if (token->type == "Keyword")
    {
        return tuple<string, int, double> { ((Keyword*)token)->name, 0, 0.0f };
    }
    else if (token->type == "Control")
    {
        return tuple<string, int, double> { ((Control*)token)->value, 0, 0.0f };
    }
    else if (token->type == "Bracket")
    {
        return tuple<string, int, double> { ((Bracket*)token)->value, 0, 0.0f };
    }
    else if (token->type == "Operator")
    {
        return tuple<string, int, double> { ((Operator*)token)->value, 0, 0.0f };
    }

    return tuple<string, int, double> { "", 0, 0.0f };
}