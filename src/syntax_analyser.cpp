#include <string>
#include <vector>
#include <regex>

#include "syntax_analyser.hpp"
#include "token.hpp"
#include "node.hpp"

using namespace std;

pair<vector<Node*>, vector<Token*>> AnalyseSyntax(vector<Token*> tokens, pair<vector<Token*>, bool> return_flags)
{
    vector<Node*> AST = vector<Node*>();

    int file_end = 0;

    if (tokens.size() != 0)
    {
        file_end = tokens[tokens.size() - 1]->end;
    }

    while (tokens.size() != 0)
    {
        if (get<0>(return_flags).size() != 0 && ShouldReturn(tokens[0], get<0>(return_flags)) || (AST.size() != 0 && get<1>(return_flags)))
        {
            return {AST, tokens};
        }

        if (tokens[0]->type == "Integer" || tokens[0]->type == "Float" || tokens[0]->type == "String")
        {
            Literal *l = new Literal();

            l->start = tokens[0]->start;
            l->end = tokens[0]->end;

            tuple<string, int, double> vals = GetTokenValue(tokens[0]);

            if (tokens[0]->type == "Integer")
            {
                l->l_integer = get<1>(vals);
            }
            else if (tokens[0]->type == "Float")
            {
                l->l_float = get<2>(vals);
            }
            else if (tokens[0]->type == "String")
            {
                l->l_string = get<0>(vals);
            }

            AST.push_back(l);

            EraseFront(&tokens, 1);
        }
        else if (tokens[0]->type == "Identifier")
        {
            int start = tokens[0]->start;

            if (tokens[1]->type == "Operator" && get<0>(GetTokenValue(tokens[1])) == "<")
            {
                vector<Token*> old_tokens(tokens);

                string name = ((Identifier*)tokens[0])->name;

                EraseFront(&tokens, 2);

                vector<Type> content = vector<Type>();

                bool is_type = true;

                while (!(tokens[0]->type == "Operator" && get<0>(GetTokenValue(tokens[0])) == ">"))
                {
                    vector<Node*> data;

                    tie(data, tokens) = AnalyseSyntax(tokens, { { new Control(","), new Operator(">") }, false });

                    if (data.size() == 0)
                    {
                        is_type = false;

                        break;
                    }

                    if (data[0]->type == "Type")
                    {
                        content.push_back(*(Type*)data[0]);
                    }
                    else if (data[0]->type == "GetVariable")
                    {
                        content.push_back(Type(((GetVariable*)data[0])->name, vector<Type>()));
                    }
                    else
                    {
                        is_type = false;

                        break;
                    }

                    if (tokens[0]->type == "Control" && get<0>(GetTokenValue(tokens[0])) == ",")
                    {
                        EraseFront(&tokens, 1);
                    }
                    else if (tokens[0]->type == "Operator" && get<0>(GetTokenValue(tokens[0])) == ">")
                    {
                        continue;
                    }
                    else
                    {
                        is_type = false;

                        break;
                    }
                }

                if (!is_type)
                {
                    tokens = old_tokens;
                }
                else
                {
                    Type* type = new Type(name, content);

                    type->start = start;

                    type->end = tokens[0]->end;

                    AST.push_back(type);

                    EraseFront(&tokens, 1);
                }
            }
            else if (AST.size() != 0 && (GetASTEnd(AST)->type == "Type" || GetASTEnd(AST)->type == "GetVariable") && tokens[1]->type == "Bracket" && get<0>(GetTokenValue(tokens[1])) == "(")
            {
                Type return_type = Type("", vector<Type>());

                Node* n = GetASTEnd(AST);

                AST.pop_back();

                if (n->type == "Type")
                {
                    return_type = *(Type*)n;
                }
                else if (n->type == "GetVariable")
                {
                    return_type = Type(((GetVariable*)n)->name, vector<Type>());
                }

                string name = get<0>(GetTokenValue(tokens[0]));

                EraseFront(&tokens, 1);

                vector<Node*> data;

                tie(data, tokens) = AnalyseSyntax(tokens, { {}, true });

                if (data.size() == 0)
                {
                    ThrowError(start, file_end, Error {SyntaxError, "Missing end of statement"});
                }

                if (data[0]->type != "CodeBlock")
                {
                    ThrowError(start, data[0]->end, Error {SyntaxError, "Invalid character in function definition"});
                }

                CodeBlock *code_block = (CodeBlock*)data[0];

                Node* node = new AssignVariable(return_type, name, code_block);

                node->start = start;
                node->end = code_block->end;

                AST.push_back(node);
            }
            else if (AST.size() != 0 && (GetASTEnd(AST)->type == "Type" || GetASTEnd(AST)->type == "GetVariable") && tokens[1]->type == "Operator" && get<0>(GetTokenValue(tokens[1])) == "=")
            {
                Type variable_type = Type("", vector<Type>());

                Node* n = GetASTEnd(AST);

                AST.pop_back();

                if (n->type == "Type")
                {
                    variable_type = *(Type*)n;
                }
                else if (n->type == "GetVariable")
                {
                    variable_type = Type(((GetVariable*)n)->name, vector<Type>());
                }

                string name = get<0>(GetTokenValue(tokens[0]));

                EraseFront(&tokens, 2);

                vector<Node*> data;

                tie(data, tokens) = AnalyseSyntax(tokens, { { new Control(";") }, false });

                if (tokens.size() == 0)
                {
                    ThrowError(start, file_end, Error {SyntaxError, "Missing ending ;"});
                }

                if (data.size() == 0)
                {
                    ThrowError(start, tokens[0]->end, Error {SyntaxError, "Missing assignment value"});
                }

                Node *content = data[0];

                Node* node = new AssignVariable(variable_type, name, content);

                node->start = start;
                node->end = tokens[0]->end;

                AST.push_back(node);

                EraseFront(&tokens, 1);
            }
            else if (tokens[1]->type == "Bracket" && get<0>(GetTokenValue(tokens[1])) == "(")
            {
                string name = ((Identifier*)tokens[0])->name;

                EraseFront(&tokens, 2);

                vector<Node*> arguments = vector<Node*>();

                bool got_arg = false;

                while (!(tokens[0]->type == "Bracket" && get<0>(GetTokenValue(tokens[0])) == ")"))
                {
                    if (tokens.size() == 0)
                    {
                        ThrowError(start, file_end, Error {SyntaxError, "Missing ending )"});
                    }

                    if (tokens[0]->type == "Control" && get<0>(GetTokenValue(tokens[0])) == ",")
                    {
                        if (!got_arg)
                        {
                            ThrowError(start,tokens[0]->end, Error {SyntaxError, "Missing argument"});
                        }

                        EraseFront(&tokens, 1);

                        got_arg = false;

                        continue;
                    }

                    vector<Node*> data;

                    tie(data, tokens) = AnalyseSyntax(tokens, { { new Control(","), new Bracket(")") }, false });

                    if (tokens.size() == 0)
                    {
                        ThrowError(start, file_end, Error {SyntaxError, "Missing ending )"});
                    }

                    if (data.size() == 0)
                    {
                        if (tokens.size() != 0)
                        {
                            ThrowError(start, tokens[0]->end, Error {SyntaxError, "Invalid character in code block definition"});
                        }
                        else
                        {
                            ThrowError(start, file_end, Error {SyntaxError, "Invalid character in code block definition"});
                        }
                    }

                    got_arg = true;

                    arguments.push_back(data[0]);
                }

                FunctionCall *function_call = new FunctionCall(name, arguments);

                function_call->start = start;
                function_call->end = tokens[0]->end;

                AST.push_back(function_call);

                EraseFront(&tokens, 1);
            }
            else
            {
                Node* node = new GetVariable(((Identifier*)tokens[0])->name);

                node->start = start;
                node->end = tokens[0]->end;

                AST.push_back(node);

                EraseFront(&tokens, 1);
            }
        }
        else if (tokens[0]->type == "Bracket" && get<0>(GetTokenValue(tokens[0])) == "(")
        {
            int start = tokens[0]->start;

            EraseFront(&tokens, 1);

            vector<Parameter> parameters = vector<Parameter>();

            bool got_arg = false;

            while (!(tokens[0]->type == "Bracket" && get<0>(GetTokenValue(tokens[0])) == ")"))
            {
                if (tokens.size() == 0)
                {
                    ThrowError(start, file_end, Error {SyntaxError, "Missing ending )"});
                }

                if (tokens[0]->type == "Control" && get<0>(GetTokenValue(tokens[0])) == ",")
                {
                    if (!got_arg)
                    {
                        ThrowError(start,tokens[0]->end, Error {SyntaxError, "Missing argument"});
                    }

                    EraseFront(&tokens, 1);

                    got_arg = false;

                    continue;
                }

                vector<Node*> data;
                
                tie(data, tokens) = AnalyseSyntax(tokens, { {}, true });

                if (data.size() != 0 && (data[0]->type == "Type" || data[0]->type == "GetVariable"))
                {
                    got_arg = true;

                    Type param_type = Type("", vector<Type>());

                    if (data[0]->type == "Type")
                    {
                        param_type = *(Type*)data[0];
                    }
                    else if (data[0]->type == "GetVariable")
                    {
                        param_type = Type(((GetVariable*)data[0])->name, vector<Type>());
                    }

                    ARGUMENT_EXPANSION param_expansion = None;

                    if (tokens[0]->type == "Operator" && get<0>(GetTokenValue(tokens[0])) == "*")
                    {
                        param_expansion = Array;

                        EraseFront(&tokens, 1);

                        if (tokens[0]->type == "Operator" && get<0>(GetTokenValue(tokens[0])) == "*")
                        {
                            param_expansion = Dictionary;

                            EraseFront(&tokens, 1);
                        }
                    }

                    if (tokens[0]->type == "Identifier")
                    {
                        string param_name = ((Identifier*)tokens[0])->name;
                        optional<Node*> param_def_value = optional<Node*>();

                        EraseFront(&tokens, 1);

                        if (tokens[0]->type == "Operator" && get<0>(GetTokenValue(tokens[0])) == "=")
                        {
                            EraseFront(&tokens, 1);

                            vector<Node*> data;

                            tie(data, tokens) = AnalyseSyntax(tokens, { { new Control(","), new Bracket(")") }, false });

                            if (tokens.size() == 0)
                            {
                                ThrowError(start, file_end, Error {SyntaxError, "Missing ending )"});
                            }

                            if (data.size() == 0)
                            {
                                ThrowError(start, tokens[0]->end, Error {SyntaxError, "Invalid character in code block definition"});
                            }

                            param_def_value = optional<Node*>(data[0]);
                        }

                        parameters.push_back(Parameter(param_type, param_name, param_def_value, param_expansion));
                    }
                    else
                    {
                        ThrowError(start, tokens[0]->end, Error {SyntaxError, "Invalid character in code block definition"});
                    }
                }
                else
                {
                    if (tokens.size() != 0)
                    {
                        ThrowError(start, tokens[0]->end, Error {SyntaxError, "Invalid character in code block definition"});
                    }
                    else
                    {
                        ThrowError(start, file_end, Error {SyntaxError, "Invalid character in code block definition"});
                    }
                }
            }

            int end;

            EraseFront(&tokens, 1);

            optional<Type> return_type = optional<Type>();

            if (tokens[0]->type == "Operator" && get<0>(GetTokenValue(tokens[0])) == "-" && tokens[1]->type == "Operator" && get<0>(GetTokenValue(tokens[1])) == ">") {
                EraseFront(&tokens, 2);

                vector<Node*> data;
                
                tie(data, tokens) = AnalyseSyntax(tokens, { {}, true });

                if (tokens.size() == 0)
                {
                    ThrowError(start, file_end, Error {SyntaxError, "Missing code block body"});
                }

                if (data[0]->type == "Type")
                {
                    return_type = *(Type*)data[0];
                }
                else if (data[0]->type == "GetVariable")
                {
                    return_type = Type(((GetVariable*)data[0])->name, vector<Type>());
                }
                else
                {
                    ThrowError(start, data[0]->end, Error {SyntaxError, "Invalid character in code block definition"});
                }
            }

            vector<Node*> content;

            if (tokens[0]->type == "Bracket" && get<0>(GetTokenValue(tokens[0])) == "{")
            {
                EraseFront(&tokens, 1);

                tie(content, tokens) = AnalyseSyntax(tokens, { { new Bracket("}") }, false });

                if (tokens.size() == 0)
                {
                    ThrowError(start, file_end, Error {SyntaxError, "Missing ending }"});
                }

                end = tokens[0]->end;

                EraseFront(&tokens, 1);
            }
            else if (tokens[0]->type == "Control" && get<0>(GetTokenValue(tokens[0])) == ";")
            {
                end = tokens[1]->end;

                EraseFront(&tokens, 2);

                content = vector<Node*>();
            }
            else
            {
                ThrowError(start, tokens[0]->end, Error { SyntaxError, "Invalid character in code block definition" });
            }

            Node* node = new CodeBlock(return_type, parameters, content);

            node->start = start;
            node->end = end;

            AST.push_back(node);
        }
        else if (tokens[0]->type == "Bracket" && get<0>(GetTokenValue(tokens[0])) == "{")
        {
            int start = tokens[0]->start;

            EraseFront(&tokens, 1);

            vector<Node*> content;

            tie(content, tokens) = AnalyseSyntax(tokens, { { new Bracket("}") }, false });

            if (tokens.size() == 0)
            {
                ThrowError(start, file_end, Error {SyntaxError, "Missing ending }"});
            }

            CodeBlock *code_block = new CodeBlock(optional<Type>(), {}, content);

            code_block->start = start;
            code_block->end = tokens[0]->end;

            AST.push_back(code_block);

            EraseFront(&tokens, 1);
        }
        else if (tokens[0]->type == "Control" && get<0>(GetTokenValue(tokens[0])) == ";")
        {
            EraseFront(&tokens, 1);
        }
        else
        {
            ThrowError(tokens[0]->start, tokens[0]->end, Error {SyntaxError, "Invalid statement start"});
        }
    }

    return {AST, tokens};
}

Node* GetASTEnd(vector<Node*> AST)
{
    if (AST.size() != 0)
    {
        return AST[AST.size() - 1];
    }
    else
    {
        return NULL;
    }
}

bool ShouldReturn(Token* current_token, vector<Token*> return_tokens)
{
    for (Token* ret_t : return_tokens)
    {
        if (*ret_t == *current_token)
        {
            return true;
        }
    }

    return false;
}

void EraseFront(vector<Token*> *tokens, int length)
{
    for (int i = 0; i < length; i++)
    {
        tokens->erase(tokens->begin());
    }
}

void ThrowError(int start, int end, Error error)
{
    Node *node = new Node();

    node->start = start;
    node->end = end;
    node->error = error;

    throw node;
}