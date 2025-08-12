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

extern std::vector<std::string> Operator_Preference;

void ThrowError(int start, int end, Error error);

class Node
{
    public:
        std::optional<Error> error;

        std::string type;
        int start;
        int end;

        Node();

        void CheckSemantics(std::vector<Node*> call_stack);

        friend std::ostream &operator<<(std::ostream &os, const Node &n);
};

class TypeExpression : public Node
{
    public:
        std::string name;
        bool is_array;
        std::vector<TypeExpression> content;

        TypeExpression(std::string name, bool is_array, std::vector<TypeExpression> content);

        void CheckSemantics(std::vector<Node*> call_stack);

        friend std::ostream &operator<<(std::ostream &os, const TypeExpression &data);
};

class ParameterExpression : public Node
{
    public:
        TypeExpression type_data;
        std::string name;
        std::optional<Node*> default_argument;
        ARGUMENT_EXPANSION argument_expansion;

        ParameterExpression(TypeExpression type_data, std::string name, std::optional<Node*> default_argument, ARGUMENT_EXPANSION argument_expansion);

        friend std::ostream &operator<<(std::ostream &os, const ParameterExpression &data);
};

class QualifierExpression : public Node
{
    public:
        std::vector<std::string> qualifiers;

        QualifierExpression(std::vector<std::string> qualifiers);

        bool Contains(std::string qualifier);

        friend std::ostream &operator<<(std::ostream &os, const QualifierExpression &data);
};

class Literal : public Node
{
    public:
        std::optional<int> l_integer;
        std::optional<double> l_float;
        std::optional<std::string> l_string;
        std::optional<bool> l_boolean;

        Literal();

        void CheckSemantics(std::vector<Node*> call_stack);

        friend std::ostream &operator<<(std::ostream &os, const Literal &data);
};

class CodeBlock : public Node
{
    public:
        TypeExpression return_type;
        std::vector<ParameterExpression> parameters;
        std::vector<Node*> content;

        CodeBlock(TypeExpression return_type, std::vector<ParameterExpression> parameters, std::vector<Node*> content);

        void CheckSemantics(std::vector<Node*> call_stack);

        friend std::ostream &operator<<(std::ostream &os, const CodeBlock &data);
};

class Operation : public Node
{
    public:
        std::string operator_string;
        Node *left;
        Node *right;

        Operation(std::string operator_string, Node *left, Node *right);

        void CheckSemantics(std::vector<Node*> call_stack);

        friend std::ostream &operator<<(std::ostream &os, const Operation &data);
};

class GetVariable : public Node
{
    public:
        std::string name;

        GetVariable(std::string name);

        void CheckSemantics(std::vector<Node*> call_stack);

        friend std::ostream &operator<<(std::ostream &os, const GetVariable &data);
};

class DeclareVariable : public Node
{
    public:
        QualifierExpression *qualifier;
        TypeExpression variable_type;
        std::string name;
        Node *value;

        DeclareVariable(QualifierExpression *qualifier, TypeExpression variable_type, std::string name, Node *value);

        void CheckSemantics(std::vector<Node*> call_stack);

        friend std::ostream &operator<<(std::ostream &os, const DeclareVariable &data);
};

class FunctionCall : public Node
{
    public:
        std::string name;
        std::vector<Node*> arguments;

        FunctionCall(std::string name, std::vector<Node*> arguments);

        void CheckSemantics(std::vector<Node*> call_stack);

        friend std::ostream &operator<<(std::ostream &os, const FunctionCall &data);
};

class ClassDefinition : public Node
{
    public:
        std::string name;
        std::string interface;
        std::vector<Node*> body;
        
        ClassDefinition(std::string name, std::string interface, std::vector<Node*> body);

        void CheckSemantics(std::vector<Node*> call_stack);

        friend std::ostream &operator<<(std::ostream &os, const ClassDefinition &data);
};

class MemberAccess : public Node
{
    public:
        std::string name;
        Node *statement;

        MemberAccess(std::string name, Node *statement);

        void CheckSemantics(std::vector<Node*> call_stack);

        friend std::ostream &operator<<(std::ostream &os, const MemberAccess &data);
};

class IfStatement : public Node
{
    public:
        Node *if_expression;
        CodeBlock *if_code_block;
        std::vector<Node*> else_if_expressions;
        std::vector<CodeBlock*> else_if_code_blocks;
        CodeBlock *else_code_block;

        IfStatement(Node *if_expression, CodeBlock *if_code_block, std::vector<Node*> else_if_expressions, std::vector<CodeBlock*> else_if_code_blocks, CodeBlock *else_code_block);

        void CheckSemantics(std::vector<Node*> call_stack);

        friend std::ostream &operator<<(std::ostream &os, const IfStatement &data);
};

class SwitchStatement : public Node
{
    public:
        Node *switch_expression;
        std::vector<Node*> case_expressions;
        std::vector<CodeBlock*> case_code_blocks;
        CodeBlock *default_code_block;

        SwitchStatement(Node *switch_expression, std::vector<Node*> case_expressions, std::vector<CodeBlock*> case_code_blocks, CodeBlock *default_code_block);

        void CheckSemantics(std::vector<Node*> call_stack);

        friend std::ostream &operator<<(std::ostream &os, const SwitchStatement &data);
};

class ForLoop : public Node
{
    public:
        Node *declaration_expression;
        Node *condition_expression;
        Node *iteration_expression;
        CodeBlock *for_code_block;

        ForLoop(Node *declaration_expression, Node *condition_expression, Node *iteration_expression, CodeBlock *for_code_block);

        void CheckSemantics(std::vector<Node*> call_stack);

        friend std::ostream &operator<<(std::ostream &os, const ForLoop &data);
};

class ForEachLoop : public Node
{
    public:
        Node *declaration_expression;
        Node *iteration_expression;
        CodeBlock *for_code_block;

        ForEachLoop(Node *declaration_expression, Node *iteration_expression, CodeBlock *for_code_block);

        void CheckSemantics(std::vector<Node*> call_stack);

        friend std::ostream &operator<<(std::ostream &os, const ForEachLoop &data);
};

class WhileLoop : public Node
{
    public:
        Node *condition;
        CodeBlock *while_code_block;

        WhileLoop(Node *condition, CodeBlock *while_code_block);

        void CheckSemantics(std::vector<Node*> call_stack);

        friend std::ostream &operator<<(std::ostream &os, const WhileLoop &data);
};

class Return : public Node
{
    public:
        Node *expression;

        Return(Node *expression);

        void CheckSemantics(std::vector<Node*> call_stack);

        friend std::ostream &operator<<(std::ostream &os, const Return &data);
};

class Break : public Node
{
    public:
        Break();

        void CheckSemantics(std::vector<Node*> call_stack);

        friend std::ostream &operator<<(std::ostream &os, const Break &data);
};

class Continue : public Node
{
    public:
        Continue();

        void CheckSemantics(std::vector<Node*> call_stack);

        friend std::ostream &operator<<(std::ostream &os, const Continue &data);
};

class StatementEnd : public Node
{
    public:
        StatementEnd();

        friend std::ostream &operator<<(std::ostream &os, const StatementEnd &data);
};

#endif