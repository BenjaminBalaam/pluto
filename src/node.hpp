#ifndef node_hpp
#define node_hpp

#include <string>
#include <vector>
#include <iostream>
#include <tuple>
#include <optional>

#include "error.hpp"

enum ARGUMENT_EXPANSION
{
    None,
    Array,
    Dictionary,
};

class Parameter;
class Qualifier;

class Node
{
    public:
        std::optional<Error> error;

        std::string type;
        int start;
        int end;

        Node();

        friend std::ostream& operator<<(std::ostream& os, const Node& n);
};

class Literal : public Node
{
    public:
        std::optional<int> l_integer;
        std::optional<double> l_float;
        std::optional<std::string> l_string;

        Literal();

        friend std::ostream& operator<<(std::ostream& os, const Literal& data);
};

class Type : public Node
{
    public:
        std::string name;
        std::vector<Type> content;

        Type(std::string name, std::vector<Type> content);

        friend std::ostream& operator<<(std::ostream& os, const Type& data);
};

class CodeBlock : public Node
{
    public:
        std::optional<Type> return_type;
        std::vector<Parameter> parameters;
        std::vector<Node*> content;

        CodeBlock(std::optional<Type> return_type, std::vector<Parameter> parameters, std::vector<Node*> content);

        friend std::ostream& operator<<(std::ostream& os, const CodeBlock& data);
};

class GetVariable : public Node
{
    public:
        std::string name;

        GetVariable(std::string name);

        friend std::ostream& operator<<(std::ostream& os, const GetVariable& data);
};

class AssignVariable : public Node
{
    public:
        Qualifier *qualifier;
        Type variable_type;
        std::string name;
        Node *value;

        AssignVariable(Qualifier *qualifier, Type variable_type, std::string name, Node *value);

        friend std::ostream& operator<<(std::ostream& os, const AssignVariable& data);
};

class FunctionCall : public Node
{
    public:
        std::string name;
        std::vector<Node*> arguments;

        FunctionCall(std::string name, std::vector<Node*> arguments);

        friend std::ostream& operator<<(std::ostream& os, const FunctionCall& data);
};

class Parameter : public Node
{
    public:
        Type type_data;
        std::string name;
        std::optional<Node*> default_argument;
        ARGUMENT_EXPANSION argument_expansion;

        Parameter(Type type_data, std::string name, std::optional<Node*> default_argument, ARGUMENT_EXPANSION argument_expansion);

        friend std::ostream& operator<<(std::ostream& os, const Parameter& data);
};

class Qualifier : public Node
{
    public:
        std::vector<std::string> qualifiers;

        Qualifier(std::vector<std::string> qualifiers);

        friend std::ostream& operator<<(std::ostream& os, const Qualifier& data);
};

#endif