#include <iostream>
#include <string>
#include <tuple>
#include <optional>
#include <vector>

#include "node.hpp"
#include "error.hpp"

using namespace std;

Node::Node()
{
    this->type = "";
}

ostream& operator<<(ostream& os, const Node& n)
{
    if (n.type == "Literal")
    {
        return os << (Literal&)n;
    }
    else if (n.type == "CodeBlock")
    {
        return os << (CodeBlock&)n;
    }
    else if (n.type == "GetVariable")
    {
        return os << (GetVariable&)n;
    }
    else if (n.type == "AssignVariable")
    {
        return os << (AssignVariable&)n;
    }
    else if (n.type == "FunctionCall")
    {
        return os << (FunctionCall&)n;
    }
    
    return os;
}

Literal::Literal()
{
    this->type = "Literal";
}

ostream& operator<<(ostream& os, const Literal& data)
{
    if (data.l_integer)
    {
        return os << data.l_integer.value();
    }
    else if (data.l_float)
    {
        return os << data.l_float.value();
    }
    else if (data.l_string)
    {
        return os << data.l_string.value();
    }

    return os;
}

CodeBlock::CodeBlock(std::string return_type, vector<tuple<string, string, optional<Node*>>> parameters, vector<Node*> content) : return_type(return_type), parameters(parameters), content(content)
{
    this->type = "CodeBlock";
}

ostream& operator<<(ostream& os, const CodeBlock& data)
{
    os << data.return_type << " (";

    tuple<string, string, optional<Node*>> a = data.parameters[0];

    for (tuple<string, string, optional<Node*>> parameter : data.parameters)
    {
        if (!get<2>(parameter))
        {
            os << get<0>(parameter) << " " << get<1>(parameter) << ", ";
        }
        else
        {
            os << get<0>(parameter) << " " << get<1>(parameter) << " = " << *get<2>(parameter).value() << ", ";
        }
    }

    os << ") {\n";

    for (Node* node : data.content)
    {
        os << node << "\n";
    }

    os << "}\n";

    return os;
}

GetVariable::GetVariable(string name) : name(name)
{
    this->type = "GetVariable";
}

ostream& operator<<(ostream& os, const GetVariable& data)
{
    return os << data.name;
}

AssignVariable::AssignVariable(string name, Node* value) : name(name), value(value)
{
    this->type = "AssignVariable";
}

ostream& operator<<(ostream& os, const AssignVariable& data)
{
    return os << data.name << " = " << *data.value;
}

FunctionCall::FunctionCall(string name, vector<Node*> arguments) : name(name), arguments(arguments)
{
    this->type = "CallFunction";
}

ostream& operator<<(ostream& os, const FunctionCall& data)
{
    os << data.name << "(";

    for (Node* node : data.arguments)
    {
        os << node << ", ";
    }

    return os << ")";
}