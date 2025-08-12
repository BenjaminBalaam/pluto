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
        std::optional<float> l_float;
        std::optional<std::string> l_string;

        Literal();

        friend std::ostream& operator<<(std::ostream& os, const Literal& data);
};

class CodeBlock : public Node
{
    public:
        std::string return_type;
        std::vector<Parameter> parameters;
        std::vector<Node*> content;

        CodeBlock(std::string return_type, std::vector<Parameter> parameters, std::vector<Node*> content);

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
        std::string name;
        Node* value;

        AssignVariable(std::string name, Node* value);

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
        std::string type_name;
        std::string name;
        std::optional<Node*> default_argument;
        ARGUMENT_EXPANSION argument_expansion;

        Parameter(std::string type_name, std::string name, std::optional<Node*> default_argument, ARGUMENT_EXPANSION argument_expansion);

        friend std::ostream& operator<<(std::ostream& os, const Parameter& data);
};

#endif