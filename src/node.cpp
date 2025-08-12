#include <iostream>
#include <string>
#include <tuple>
#include <optional>
#include <vector>

#include "node.hpp"
#include "error.hpp"
#include "semantics_analyser.hpp"

using namespace std;

vector<string> Operator_Preference = { "^[\\^]$", "^[\\*\\/\\$\\%]$", "^[\\+\\-]$", "^[<>]|([!=<>]=)$", "^&$", "^\\|$", "^[\\+\\-\\*\\/]?=$" };

void ThrowError(int start, int end, Error error)
{
    Node *node = new Node();

    node->start = start;
    node->end = end;
    node->error = error;

    throw node;
}

Node::Node()
{
    this->type = "";
}

void Node::CheckSemantics(vector<Node*> call_stack)
{
    if (this->type == "CodeBlock")
    {
        ((CodeBlock*)this)->CheckSemantics(call_stack);
    }
    else if (this->type == "Operation")
    {
        ((Operation*)this)->CheckSemantics(call_stack);
    }
    else if (this->type == "DeclareVariable")
    {
        ((DeclareVariable*)this)->CheckSemantics(call_stack);
    }
    else if (this->type == "FunctionCall")
    {
        ((FunctionCall*)this)->CheckSemantics(call_stack);
    }
    else if (this->type == "ClassDefinition")
    {
        ((ClassDefinition*)this)->CheckSemantics(call_stack);
    }
    else if (this->type == "MemberAccess")
    {
        ((MemberAccess*)this)->CheckSemantics(call_stack);
    }
    else if (this->type == "IfStatement")
    {
        ((IfStatement*)this)->CheckSemantics(call_stack);
    }
    else if (this->type == "SwitchStatement")
    {
        ((SwitchStatement*)this)->CheckSemantics(call_stack);
    }
    else if (this->type == "ForLoop")
    {
        ((ForLoop*)this)->CheckSemantics(call_stack);
    }
    else if (this->type == "ForEachLoop")
    {
        ((ForEachLoop*)this)->CheckSemantics(call_stack);
    }
    else if (this->type == "WhileLoop")
    {
        ((WhileLoop*)this)->CheckSemantics(call_stack);
    }
    else if (this->type == "Return")
    {
        ((Return*)this)->CheckSemantics(call_stack);
    }
}

ostream &operator<<(ostream &os, const Node &n)
{
    if (n.type == "TypeExpression")
    {
        return os << (TypeExpression&)n;
    }
    else if (n.type == "ParameterExpression")
    {
        return os << (ParameterExpression&)n;
    }
    else if (n.type == "QualifierExpression")
    {
        return os << (QualifierExpression&)n;
    }
    else if (n.type == "Literal")
    {
        return os << (Literal&)n;
    }
    else if (n.type == "CodeBlock")
    {
        return os << (CodeBlock&)n;
    }
    else if (n.type == "Operation")
    {
        return os << (Operation&)n;
    }
    else if (n.type == "GetVariable")
    {
        return os << (GetVariable&)n;
    }
    else if (n.type == "DeclareVariable")
    {
        return os << (DeclareVariable&)n;
    }
    else if (n.type == "FunctionCall")
    {
        return os << (FunctionCall&)n;
    }
    else if (n.type == "ClassDefinition")
    {
        return os << (ClassDefinition&)n;
    }
    else if (n.type == "MemberAccess")
    {
        return os << (MemberAccess&)n;
    }
    else if (n.type == "IfStatement")
    {
        return os << (IfStatement&)n;
    }
    else if (n.type == "SwitchStatement")
    {
        return os << (SwitchStatement&)n;
    }
    else if (n.type == "ForLoop")
    {
        return os << (ForLoop&)n;
    }
    else if (n.type == "ForEachLoop")
    {
        return os << (ForEachLoop&)n;
    }
    else if (n.type == "WhileLoop")
    {
        return os << (WhileLoop&)n;
    }
    else if (n.type == "Return")
    {
        return os << (Return&)n;
    }
    else if (n.type == "Break")
    {
        return os << (Break&)n;
    }
    else if (n.type == "Continue")
    {
        return os << (Continue&)n;
    }
    else if (n.type == "StatementEnd")
    {
        return os << (StatementEnd&)n;
    }
    
    return os;
}

TypeExpression::TypeExpression(string name, bool is_array, vector<TypeExpression> content) : name(name), is_array(is_array), content(content)
{
    this->type = "TypeExpression";
}

ostream &operator<<(ostream &os, const TypeExpression &data)
{
    if (data.is_array)
    {
        return os << data.name << "[]";
    }
    else
    {
        os << data.name << "<";

        for (TypeExpression t : data.content)
        {
            os << t << ", ";
        }

        return os << ">";
    }

    return os;
}

ParameterExpression::ParameterExpression(TypeExpression type_data, string name, optional<Node*> default_argument, ARGUMENT_EXPANSION argument_expansion) : type_data(type_data), name(name), default_argument(default_argument), argument_expansion(argument_expansion)
{
    this->type = "ParameterExpression";
}

ostream &operator<<(ostream &os, const ParameterExpression &data)
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

QualifierExpression::QualifierExpression(vector<string> qualifiers) : qualifiers(qualifiers)
{
    this->type = "QualifierExpression";
}

bool QualifierExpression::Contains(string qualifier)
{
    for (string q : this->qualifiers)
    {
        if (q == qualifier)
        {
            return true;
        }
    }

    return false;
}

ostream &operator<<(ostream &os, const QualifierExpression &data)
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

ostream &operator<<(ostream &os, const Literal &data)
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
    else if (data.l_boolean)
    {
        return os << (data.l_boolean.value() ? "true" : "false");
    }

    return os;
}

CodeBlock::CodeBlock(TypeExpression return_type, vector<ParameterExpression> parameters, vector<Node*> content) : return_type(return_type), parameters(parameters), content(content)
{
    this->type = "CodeBlock";
}

void CodeBlock::CheckSemantics(vector<Node*> call_stack)
{
    vector<Node*> new_call_stack = vector<Node*>(call_stack);
    new_call_stack.push_back(this);

    for (Node *statement : this->content)
    {
        CheckStatement(statement, new_call_stack);
    }
}

ostream &operator<<(ostream &os, const CodeBlock &data)
{
    os << data.return_type << " (";

    for (ParameterExpression parameter : data.parameters)
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

void Operation::CheckSemantics(vector<Node*> call_stack)
{
    CheckExpression(this->left, call_stack);
    CheckExpression(this->right, call_stack);
}

ostream &operator<<(ostream &os, const Operation &data)
{
    if (data.left == NULL)
    {
        return os << "(" << data.operator_string << *data.right << ")";
    }
    else
    {
        return os << "(" << *data.left << data.operator_string << *data.right << ")";
    }
}

GetVariable::GetVariable(string name) : name(name)
{
    this->type = "GetVariable";
}

ostream &operator<<(ostream &os, const GetVariable &data)
{
    return os << data.name;
}

DeclareVariable::DeclareVariable(QualifierExpression *qualifier, TypeExpression variable_type, string name, Node *value) : qualifier(qualifier), variable_type(variable_type), name(name), value(value)
{
    this->type = "DeclareVariable";
}

void DeclareVariable::CheckSemantics(vector<Node*> call_stack)
{
    CheckExpression(value, call_stack);
}

ostream &operator<<(ostream &os, const DeclareVariable &data)
{
    if (data.value == NULL)
    {
        return os << *data.qualifier << data.variable_type << " " << data.name;
    }
    else
    {
        return os << *data.qualifier << data.variable_type << " " << data.name << " = " << *data.value;
    }
}

FunctionCall::FunctionCall(string name, vector<Node*> arguments) : name(name), arguments(arguments)
{
    this->type = "FunctionCall";
}

void FunctionCall::CheckSemantics(vector<Node*> call_stack)
{
    for (Node *expression : this->arguments)
    {
        CheckExpression(expression, call_stack);
    }
}

ostream &operator<<(ostream &os, const FunctionCall &data)
{
    os << data.name << "(";

    for (Node *node : data.arguments)
    {
        os << *node << ", ";
    }

    return os << ")";
}

ClassDefinition::ClassDefinition(string name, string interface, vector<Node*> body) : name(name), interface(interface), body(body)
{
    this->type = "ClassDefinition";
}

void ClassDefinition::CheckSemantics(vector<Node*> call_stack)
{
    vector<Node*> new_call_stack = vector<Node*>(call_stack);
    new_call_stack.push_back(this);

    for (Node *statement : this->body)
    {
        CheckStatement(statement, new_call_stack);
    }
}

ostream &operator<<(ostream &os, const ClassDefinition &data)
{
    if (data.interface == "")
    {
        os << "class " << data.name << " {\n";
    }
    else
    {
        os << "class " << data.name << " : " << data.interface << " {\n";
    }

    for (Node *node : data.body)
    {
        os << *node << "\n";
    }

    os << "}";

    return os;
}

MemberAccess::MemberAccess(string name, Node *statement) : name(name), statement(statement)
{
    this->type = "MemberAccess";
}

void MemberAccess::CheckSemantics(vector<Node*> call_stack)
{
    CheckStatement(this->statement, call_stack);
}

ostream &operator<<(ostream &os, const MemberAccess &data)
{
    return os << data.name << "." << *data.statement;
}

IfStatement::IfStatement(Node *if_expression, CodeBlock *if_code_block, vector<Node*> else_if_expressions, vector<CodeBlock*> else_if_code_blocks, CodeBlock *else_code_block) : if_expression(if_expression), if_code_block(if_code_block), else_if_expressions(else_if_expressions), else_if_code_blocks(else_if_code_blocks), else_code_block(else_code_block)
{
    this->type = "IfStatement";
}

void IfStatement::CheckSemantics(vector<Node*> call_stack)
{
    vector<Node*> new_call_stack = vector<Node*>(call_stack);
    new_call_stack.push_back(this);

    CheckExpression(this->if_expression, call_stack);

    CheckStatement(this->if_code_block, new_call_stack);

    for (Node *expression : this->else_if_expressions)
    {
        CheckExpression(expression, call_stack);
    }

    for (Node *expression : this->else_if_code_blocks)
    {
        CheckExpression(expression, new_call_stack);
    }

    CheckExpression(this->else_code_block, new_call_stack);
}

ostream &operator<<(ostream &os, const IfStatement &data)
{
    os << "if (" << *data.if_expression << ") " << *data.if_code_block;

    for (int i = 0; i < data.else_if_expressions.size(); i++)
    {
        os << " else if (" << *data.else_if_expressions[i] << ") " << *data.else_if_code_blocks[i];
    }

    if (data.else_code_block != NULL)
    {
        os << " else " << *data.else_code_block;
    }

    return os;
}

SwitchStatement::SwitchStatement(Node *switch_expression, vector<Node*> case_expressions, vector<CodeBlock*> case_code_blocks, CodeBlock *default_code_block) : switch_expression(switch_expression), case_expressions(case_expressions), case_code_blocks(case_code_blocks), default_code_block(default_code_block)
{
    this->type = "SwitchStatement";
}

void SwitchStatement::CheckSemantics(vector<Node*> call_stack)
{
    vector<Node*> new_call_stack = vector<Node*>(call_stack);
    new_call_stack.push_back(this);

    CheckExpression(this->switch_expression, call_stack);

    for (Node *expression : this->case_expressions)
    {
        CheckExpression(expression, new_call_stack);
    }

    for (Node *expression : this->case_code_blocks)
    {
        CheckExpression(expression, new_call_stack);
    }

    CheckExpression(this->default_code_block, new_call_stack);
}

ostream &operator<<(ostream &os, const SwitchStatement &data)
{
    os << "switch (" << *data.switch_expression << ") {";

    for (int i = 0; i < data.case_expressions.size(); i++)
    {
        os << " case (" << *data.case_expressions[i] << ") " << *data.case_code_blocks[i];
    }

    if (data.default_code_block != NULL)
    {
        os << " default " << *data.default_code_block;
    }

    return os;
}

ForLoop::ForLoop(Node *declaration_expression, Node*condition_expression, Node *iteration_expression, CodeBlock *for_code_block) : declaration_expression(declaration_expression), condition_expression(condition_expression), iteration_expression(iteration_expression), for_code_block(for_code_block)
{
    this->type = "ForLoop";
}

void ForLoop::CheckSemantics(vector<Node*> call_stack)
{
    vector<Node*> new_call_stack = vector<Node*>(call_stack);
    new_call_stack.push_back(this);

    CheckStatement(this->declaration_expression, call_stack);
    CheckStatement(this->condition_expression, call_stack);
    CheckStatement(this->iteration_expression, call_stack);

    CheckExpression(this->for_code_block, new_call_stack);
}

ostream &operator<<(ostream &os, const ForLoop &data)
{
    os << "for (";

    if (data.declaration_expression != NULL)
    {
        os << *data.declaration_expression;
    }

    os << "; ";

    if (data.condition_expression != NULL)
    {
        os << *data.condition_expression;
    }

    os << "; ";

    if (data.iteration_expression != NULL)
    {
        os << *data.iteration_expression;
    }

    return os << ") " << *data.for_code_block;
}

ForEachLoop::ForEachLoop(Node *declaration_expression, Node *iteration_expression, CodeBlock *for_code_block) : declaration_expression(declaration_expression), iteration_expression(iteration_expression), for_code_block(for_code_block)
{
    this->type = "ForEachLoop";
}

void ForEachLoop::CheckSemantics(vector<Node*> call_stack)
{
    vector<Node*> new_call_stack = vector<Node*>(call_stack);
    new_call_stack.push_back(this);

    CheckExpression(this->declaration_expression, call_stack);
    CheckExpression(this->iteration_expression, call_stack);

    CheckExpression(this->for_code_block, new_call_stack);
}

ostream &operator<<(ostream &os, const ForEachLoop &data)
{
    return os << "for (" << *data.declaration_expression << " : " << *data.iteration_expression << ") " << *data.for_code_block;
}

WhileLoop::WhileLoop(Node *condition, CodeBlock *while_code_block) : condition(condition), while_code_block(while_code_block)
{
    this->type = "WhileLoop";
}

void WhileLoop::CheckSemantics(vector<Node*> call_stack)
{
    vector<Node*> new_call_stack = vector<Node*>(call_stack);
    new_call_stack.push_back(this);

    CheckExpression(this->condition, call_stack);

    CheckExpression(this->while_code_block, new_call_stack);
}

ostream &operator<<(ostream &os, const WhileLoop &data)
{
    return os << "while (" << *data.condition << ") " << *data.while_code_block;
}

Return::Return(Node *expression) : expression(expression)
{
    this->type = "Return";
}

void Return::CheckSemantics(vector<Node*> call_stack)
{
    CheckExpression(this->expression, call_stack);
}

ostream &operator<<(ostream &os, const Return &data)
{
    if (data.expression == NULL)
    {
        return os << "return";
    }
    else
    {
        return os << "return " << *data.expression;
    }
}

Break::Break()
{
    this->type = "Break";
}

ostream &operator<<(ostream &os, const Break &data)
{
    return os << "break";
}

Continue::Continue()
{
    this->type = "Continue";
}

ostream &operator<<(ostream &os, const Continue &data)
{
    return os << "continue";
}

StatementEnd::StatementEnd()
{
    this->type = "StatementEnd";
}

ostream &operator<<(ostream &os, const StatementEnd &data)
{
    return os;
}