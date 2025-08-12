#include <iostream>
#include <string>
#include <tuple>

#include "token.hpp"
#include "error.hpp"

using namespace std;

vector<string> Keywords = { "public", "static", "const", "class", "if", "else", "switch", "case", "default", "for", "while", "return", "break", "continue" };

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
    else if (t.type == "Boolean")
    {
        return os << (Boolean&)t;
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

Boolean::Boolean(bool boolean) : boolean(boolean)
{
    this->type = "Boolean";
}

ostream &operator<<(ostream &os, const Boolean &b)
{
    return os << (b.boolean ? "true" : "false");
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

tuple<string, int, double, bool> GetTokenValue(Token *token)
{
    if (token->type == "Integer")
    {
        return tuple<string, int, double, bool> { "", ((Integer*)token)->number, 0.0f, false };
    }
    else if (token->type == "Float")
    {
        return tuple<string, int, double, bool> { "", 0, ((Float*)token)->number, false };
    }
    else if (token->type == "String")
    {
        return tuple<string, int, double, bool> { ((String*)token)->content, 0, 0.0f, false };
    }
    else if (token->type == "Boolean")
    {
        return tuple<string, int, double, bool> { "", 0, 0.0f, ((Boolean*)token)->boolean };
    }
    else if (token->type == "Identifier")
    {
        return tuple<string, int, double, bool> { ((Identifier*)token)->name, 0, 0.0f, false };
    }
    else if (token->type == "Keyword")
    {
        return tuple<string, int, double, bool> { ((Keyword*)token)->name, 0, 0.0f, false };
    }
    else if (token->type == "Control")
    {
        return tuple<string, int, double, bool> { ((Control*)token)->value, 0, 0.0f, false };
    }
    else if (token->type == "Bracket")
    {
        return tuple<string, int, double, bool> { ((Bracket*)token)->value, 0, 0.0f, false };
    }
    else if (token->type == "Operator")
    {
        return tuple<string, int, double, bool> { ((Operator*)token)->value, 0, 0.0f, false };
    }

    return tuple<string, int, double, bool> { "", 0, 0.0f, false };
}