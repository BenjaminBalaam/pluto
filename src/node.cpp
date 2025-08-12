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
    else if (n.type == "Type")
    {
        return os << (Type&)n;
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
    else if (n.type == "Parameter")
    {
        return os << (Parameter&)n;
    }
    else if (n.type == "Qualifier")
    {
        return os << (Qualifier&)n;
    }
    
    return os;
}

Type::Type(string name, bool is_array, vector<Type> content) : name(name), is_array(is_array), content(content)
{
    this->type = "Type";
}

ostream& operator<<(ostream& os, const Type& data)
{
    if (data.is_array)
    {
        return os << data.name << "[]";
    }
    else
    {
        os << data.name << "<";

        for (Type t : data.content)
        {
            os << t << ", ";
        }

        return os << ">";
    }

    return os;
}

Parameter::Parameter(Type type_data, string name, optional<Node*> default_argument, ARGUMENT_EXPANSION argument_expansion) : type_data(type_data), name(name), default_argument(default_argument), argument_expansion(argument_expansion)
{
    this->type = "Parameter";
}

ostream& operator<<(ostream& os, const Parameter& data)
{
    if (data.argument_expansion == None)
    {
        os << data.type_data << " " << data.name;
    }
    else if (data.argument_expansion == Array)
    {
        os << data.type_data << " *" << data.name;
    }
    else if (data.argument_expansion == Dictionary)
    {
        os << data.type_data << " **" << data.name;
    }

    if (data.default_argument)
    {
        os << " = " << *data.default_argument.value();
    }

    return os;
}

Qualifier::Qualifier(vector<string> qualifiers) : qualifiers(qualifiers)
{
    this->type = "Qualifier";
}

ostream& operator<<(ostream& os, const Qualifier& data)
{
    for (string qualifier : data.qualifiers)
    {
        os << qualifier << " ";
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

CodeBlock::CodeBlock(optional<Type> return_type, vector<Parameter> parameters, vector<Node*> content) : return_type(return_type), parameters(parameters), content(content)
{
    this->type = "CodeBlock";
}

ostream& operator<<(ostream& os, const CodeBlock& data)
{
    if (data.return_type)
    {
        os << data.return_type.value() << " (";
    }
    else
    {
        os << "(";
    }

    for (Parameter parameter : data.parameters)
    {
        os << parameter << ", ";
    }

    os << ") {\n";

    for (Node *node : data.content)
    {
        os << *node << "\n";
    }

    os << "}";

    return os;
}

Operation::Operation(string operator_string, Node *left, Node *right) : operator_string(operator_string), left(left), right(right)
{
    this->type = "Operation";
}

ostream& operator<<(ostream& os, const Operation& data)
{
    return os << *data.left << data.operator_string << *data.right;
}

GetVariable::GetVariable(string name) : name(name)
{
    this->type = "GetVariable";
}

ostream& operator<<(ostream& os, const GetVariable& data)
{
    return os << data.name;
}

AssignVariable::AssignVariable(Qualifier *qualifier, Type variable_type, string name, Node *value) : qualifier(qualifier), variable_type(variable_type), name(name), value(value)
{
    this->type = "AssignVariable";
}

ostream& operator<<(ostream& os, const AssignVariable& data)
{
    return os << *data.qualifier << data.variable_type << " " << data.name << " = " << *data.value << ";";
}

FunctionCall::FunctionCall(string name, vector<Node*> arguments) : name(name), arguments(arguments)
{
    this->type = "FunctionCall";
}

ostream& operator<<(ostream& os, const FunctionCall& data)
{
    os << data.name << "(";

    for (Node *node : data.arguments)
    {
        os << *node << ", ";
    }

    return os << ")";
}