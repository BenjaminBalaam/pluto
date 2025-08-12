#include <string>
#include <vector>
#include <regex>

#include "semantics_analyser.hpp"
#include "token.hpp"
#include "node.hpp"

using namespace std;

vector<Node*> AnalyseSemantics(vector<Node*> AST, vector<Node*> call_stack)
{
    vector<Node*> new_AST = {};

    if (AST.size() == 0)
    {
        return new_AST;
    }

    bool semicolon_needed = false;

    if (AST[AST.size() - 1]->type != "ClassDefinition" && AST[AST.size() - 1]->type != "IfStatement" && AST[AST.size() - 1]->type != "SwitchStatement" &&
        AST[AST.size() - 1]->type != "ForLoop" && AST[AST.size() - 1]->type != "ForEachLoop" && AST[AST.size() - 1]->type != "WhileLoop" && AST[AST.size() - 1]->type != "StatementEnd")
    {
        ThrowError(AST[AST.size() - 1]->start, AST[AST.size() - 1]->end, Error {SyntaxError, "Missing ending ;"});
    }

    for (Node *node : AST)
    {
        if (node->type != "StatementEnd" && !semicolon_needed)
        {
            if (node->type == "ClassDefinition" || node->type == "IfStatement" || node->type == "SwitchStatement" ||
                node->type == "ForLoop" || node->type == "ForEachLoop" || node->type == "WhileLoop")
            {
                semicolon_needed = false;
            }
            else
            {
                semicolon_needed = true;
            }
        }
        else if (node->type != "StatementEnd" && semicolon_needed)
        {
            ThrowError(node->start, node->end, Error {SyntaxError, "Missing ending ;"});
        }
        else if (node->type == "StatementEnd" && semicolon_needed)
        {
            semicolon_needed = false;
        }

        if (node->type != "StatementEnd")
        {
            new_AST.push_back(node);
        }
    }

    for (Node *node : new_AST)
    {
        CheckStatement(node, call_stack);
    }

    return new_AST;
}

void CheckStatement(Node *statement, vector<Node*> call_stack)
{
    if (statement == NULL)
    {
        return;
    }

    if (statement->type == "ParameterExpression" || statement->type == "QualifierExpression")
    {
        ThrowError(statement->start, statement->end, Error {SyntaxError, "Invalid expression"});
    }

    if (statement->type == "Return" && !InCallStack(call_stack, "CodeBlock"))
    {
        ThrowError(statement->start, statement->end, Error {SyntaxError, "Return outside function"});
    }
    else if ((statement->type == "Break" || statement->type == "Continue") &&
              !InCallStack(call_stack, "ForLoop") && !InCallStack(call_stack, "ForEachLoop") && !InCallStack(call_stack, "WhileLoop") &&
              (!InCallStack(call_stack, "FunctionCall") || (CallStackPosition(call_stack, "ForLoop") < CallStackPosition(call_stack, "CodeBlock")) ||
              (CallStackPosition(call_stack, "ForEachLoop") < CallStackPosition(call_stack, "CodeBlock")) || (CallStackPosition(call_stack, "WhileLoop") < CallStackPosition(call_stack, "CodeBlock"))))
    {
        if (statement->type == "Break")
        {
            ThrowError(statement->start, statement->end, Error {SyntaxError, "Break outside loop"});
        }
        else if (statement->type == "Continue")
        {
            ThrowError(statement->start, statement->end, Error {SyntaxError, "Continue outside loop"});
        }
    }

    statement->CheckSemantics(call_stack);
}

void CheckExpression(Node *expression, vector<Node*> call_stack)
{
    if (expression == NULL)
    {
        return;
    }

    if (
        expression->type == "ParameterExpression" || expression->type == "QualifierExpression" || expression->type == "DeclareVariable" ||
        expression->type == "ClassDefinition" || expression->type == "IfStatement" || expression->type == "SwitchStatement" ||
        expression->type == "ForLoop" || expression->type == "ForEachLoop" || expression->type == "WhileLoop" ||
        expression->type == "Return" || expression->type == "Break" || expression->type == "Continue" ||
        expression->type == "StatementEnd"
    )
    {
        ThrowError(expression->start, expression->end, Error {SyntaxError, "Invalid expression"});
    }

    expression->CheckSemantics(call_stack);
}

bool InCallStack(vector<Node*> call_stack, string type)
{
    for (Node *node : call_stack)
    {
        if (node->type == type)
        {
            return true;
        }
    }

    return false;
}

int CallStackPosition(vector<Node*> call_stack, string type)
{
    int position = -1;

    for (int i = 0; i < call_stack.size(); i++)
    {
        if (call_stack[i]->type == type)
        {
            position = i;
        }
    }

    return position;
}