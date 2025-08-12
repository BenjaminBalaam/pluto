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
        bool last = tokens.size() == 1;

        int start = tokens[0]->start;

        if (get<0>(return_flags).size() != 0 && ShouldReturn(tokens[0], get<0>(return_flags)) || (AST.size() != 0 && get<1>(return_flags)))
        {
            return {AST, tokens};
        }

        if (tokens[0]->type == "Integer" || tokens[0]->type == "Float" || tokens[0]->type == "String")
        {
            Literal *l = new Literal();

            l->start = start;
            l->end = tokens[0]->end;

            string type = tokens[0]->type;

            tuple<string, int, double> vals = GetTokenValue(tokens[0]);

            if (type == "Integer")
            {
                l->l_integer = get<1>(vals);
            }
            else if (type == "Float")
            {
                l->l_float = get<2>(vals);
            }
            else if (type == "String")
            {
                l->l_string = get<0>(vals);
            }

            AST.push_back(l);

            EraseFront(&tokens, 1);
        }
        else if (tokens[0]->type == "Identifier")
        {
            if (!last && tokens[1]->type == "Operator" && get<0>(GetTokenValue(tokens[1])) == "<")
            {
                vector<Token*> old_tokens(tokens);

                string name = ((Identifier*)tokens[0])->name;

                if (EraseFront(&tokens, 2))
                {
                    ThrowError(start, file_end, Error {SyntaxError, "Missing end of type"});
                }

                vector<Type> content = vector<Type>();

                bool is_type = true;

                bool got_arg = false;

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

                        got_arg = true;
                    }
                    else if (data[0]->type == "GetVariable")
                    {
                        content.push_back(Type(((GetVariable*)data[0])->name, vector<Type>()));

                        got_arg = true;
                    }
                    else
                    {
                        is_type = false;

                        break;
                    }

                    if (tokens.size() == 0)
                    {
                        ThrowError(start, file_end, Error {SyntaxError, "Missing ending >"});
                    }

                    if (tokens[0]->type == "Control" && get<0>(GetTokenValue(tokens[0])) == ",")
                    {
                        if (!got_arg)
                        {
                            ThrowError(start,tokens[0]->end, Error {SyntaxError, "Missing type argument"});
                        }

                        if (EraseFront(&tokens, 1))
                        {
                            ThrowError(start, file_end, Error {SyntaxError, "Missing end of type"});
                        }

                        got_arg = false;

                        continue;
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
                    Type *type = new Type(name, content);

                    type->start = start;

                    type->end = tokens[0]->end;

                    AST.push_back(type);

                    EraseFront(&tokens, 1);
                }
            }
            else if (!last && AST.size() != 0 && (GetASTEnd(AST)->type == "Type" || GetASTEnd(AST)->type == "GetVariable") && tokens[1]->type == "Bracket" && get<0>(GetTokenValue(tokens[1])) == "(")
            {
                Type return_type = Type("", vector<Type>());

                Node *n = GetASTEnd(AST);

                AST.pop_back();

                if (n->type == "Type")
                {
                    return_type = *(Type*)n;
                }
                else if (n->type == "GetVariable")
                {
                    return_type = Type(((GetVariable*)n)->name, vector<Type>());
                }

                Qualifier *qualifier = new Qualifier({});

                if (AST.size() != 0 && GetASTEnd(AST)->type == "Qualifier")
                {
                    qualifier = (Qualifier*)GetASTEnd(AST);

                    AST.pop_back();
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

                Node *node = new AssignVariable(qualifier, return_type, name, code_block);

                node->start = start;
                node->end = code_block->end;

                AST.push_back(node);
            }
            else if (!last && AST.size() != 0 && (GetASTEnd(AST)->type == "Type" || GetASTEnd(AST)->type == "GetVariable") && tokens[1]->type == "Operator" && get<0>(GetTokenValue(tokens[1])) == "=")
            {
                Type variable_type = Type("", vector<Type>());

                Node *n = GetASTEnd(AST);

                AST.pop_back();

                if (n->type == "Type")
                {
                    variable_type = *(Type*)n;
                }
                else if (n->type == "GetVariable")
                {
                    variable_type = Type(((GetVariable*)n)->name, vector<Type>());
                }

                Qualifier *qualifier = new Qualifier({});

                if (AST.size() != 0 && GetASTEnd(AST)->type == "Qualifier")
                {
                    qualifier = (Qualifier*)GetASTEnd(AST);

                    AST.pop_back();
                }

                string name = get<0>(GetTokenValue(tokens[0]));

                if (EraseFront(&tokens, 2))
                {
                    ThrowError(start, file_end, Error {SyntaxError, "Missing end of assignment"});
                }

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

                Node *node = new AssignVariable(qualifier, variable_type, name, content);

                node->start = start;
                node->end = tokens[0]->end;

                AST.push_back(node);

                EraseFront(&tokens, 1);
            }
            else if (!last && tokens[1]->type == "Bracket" && get<0>(GetTokenValue(tokens[1])) == "(")
            {
                string name = ((Identifier*)tokens[0])->name;

                if (EraseFront(&tokens, 2))
                {
                    ThrowError(start, file_end, Error {SyntaxError, "Missing end of function call"});
                }

                vector<Node*> arguments = vector<Node*>();

                bool got_arg = false;

                while (!(tokens[0]->type == "Bracket" && get<0>(GetTokenValue(tokens[0])) == ")"))
                {
                    if (tokens[0]->type == "Control" && get<0>(GetTokenValue(tokens[0])) == ",")
                    {
                        if (!got_arg)
                        {
                            ThrowError(start,tokens[0]->end, Error {SyntaxError, "Missing argument"});
                        }

                        if (EraseFront(&tokens, 1))
                        {
                            ThrowError(start, file_end, Error {SyntaxError, "Missing end of function call"});
                        }

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
                        ThrowError(start, tokens[0]->end, Error {SyntaxError, "Invalid character in function call"});
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
                Node *node = new GetVariable(((Identifier*)tokens[0])->name);

                node->start = start;
                node->end = tokens[0]->end;

                AST.push_back(node);

                EraseFront(&tokens, 1);
            }
        }
        else if (tokens[0]->type == "Bracket" && get<0>(GetTokenValue(tokens[0])) == "(")
        {
            int start = tokens[0]->start;

            if (EraseFront(&tokens, 1))
            {
                ThrowError(start, file_end, Error {SyntaxError, "Missing ending )"});
            }

            vector<Parameter> parameters = vector<Parameter>();

            bool got_arg = false;

            while (!(tokens[0]->type == "Bracket" && get<0>(GetTokenValue(tokens[0])) == ")"))
            {
                if (tokens[0]->type == "Control" && get<0>(GetTokenValue(tokens[0])) == ",")
                {
                    if (!got_arg)
                    {
                        ThrowError(start,tokens[0]->end, Error {SyntaxError, "Missing argument"});
                    }

                    if (EraseFront(&tokens, 1))
                    {
                        ThrowError(start, file_end, Error {SyntaxError, "Missing end of code block"});
                    }

                    got_arg = false;

                    continue;
                }

                vector<Node*> data;
                
                tie(data, tokens) = AnalyseSyntax(tokens, { {}, true });

                if (tokens.size() == 0)
                {
                    ThrowError(start, file_end, Error {SyntaxError, "Missing end of code block"});
                }

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

                        if (EraseFront(&tokens, 1))
                        {
                            ThrowError(start, file_end, Error {SyntaxError, "Missing end of code block"});
                        }

                        if (tokens[0]->type == "Operator" && get<0>(GetTokenValue(tokens[0])) == "*")
                        {
                            param_expansion = Dictionary;

                            if (EraseFront(&tokens, 1))
                            {
                                ThrowError(start, file_end, Error {SyntaxError, "Missing end of code block"});
                            }
                        }
                    }

                    if (tokens[0]->type == "Identifier")
                    {
                        string param_name = ((Identifier*)tokens[0])->name;
                        optional<Node*> param_def_value = optional<Node*>();

                        if (EraseFront(&tokens, 1))
                        {
                            ThrowError(start, file_end, Error {SyntaxError, "Missing end of code block"});
                        }

                        if (tokens[0]->type == "Operator" && get<0>(GetTokenValue(tokens[0])) == "=")
                        {
                            if (EraseFront(&tokens, 1))
                            {
                                ThrowError(start, file_end, Error {SyntaxError, "Missing end of default argument"});
                            }

                            vector<Node*> data;

                            tie(data, tokens) = AnalyseSyntax(tokens, { { new Control(","), new Bracket(")") }, false });

                            if (tokens.size() == 0)
                            {
                                ThrowError(start, file_end, Error {SyntaxError, "Missing ending )"});
                            }

                            if (data.size() == 0)
                            {
                                ThrowError(start, tokens[0]->end, Error {SyntaxError, "Invalid character in code block"});
                            }

                            param_def_value = optional<Node*>(data[0]);
                        }

                        parameters.push_back(Parameter(param_type, param_name, param_def_value, param_expansion));
                    }
                    else
                    {
                        ThrowError(start, tokens[0]->end, Error {SyntaxError, "Invalid character in code block"});
                    }
                }
                else
                {
                    ThrowError(start, tokens[0]->end, Error {SyntaxError, "Invalid character in code block"});
                }
            }

            int end;

            if (EraseFront(&tokens, 1))
            {
                ThrowError(start, file_end, Error {SyntaxError, "Missing end of code block"});
            }

            optional<Type> return_type = optional<Type>();

            if (tokens.size() > 1 && tokens[0]->type == "Operator" && get<0>(GetTokenValue(tokens[0])) == "-" && tokens[1]->type == "Operator" && get<0>(GetTokenValue(tokens[1])) == ">") {
                if (EraseFront(&tokens, 2))
                {
                    ThrowError(start, file_end, Error {SyntaxError, "Missing return type"});
                }

                vector<Node*> data;
                
                tie(data, tokens) = AnalyseSyntax(tokens, { {}, true });

                if (tokens.size() == 0)
                {
                    ThrowError(start, file_end, Error {SyntaxError, "Missing end of code block"});
                }

                if (data.size() == 0)
                {
                    ThrowError(start, file_end, Error {SyntaxError, "Invalid character in code block"});
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
                    ThrowError(start, data[0]->end, Error {SyntaxError, "Invalid character in code block"});
                }
            }

            vector<Node*> content;

            if (tokens[0]->type == "Bracket" && get<0>(GetTokenValue(tokens[0])) == "{")
            {
                if (EraseFront(&tokens, 1))
                {
                    ThrowError(start, file_end, Error {SyntaxError, "Missing ending }"});
                }

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
                end = tokens[0]->end;

                EraseFront(&tokens, 1);

                content = vector<Node*>();
            }
            else
            {
                ThrowError(start, tokens[0]->end, Error { SyntaxError, "Invalid character in code block" });
            }

            Node *node = new CodeBlock(return_type, parameters, content);

            node->start = start;
            node->end = end;

            AST.push_back(node);
        }
        else if (tokens[0]->type == "Bracket" && get<0>(GetTokenValue(tokens[0])) == "{")
        {
            int start = tokens[0]->start;

            if (EraseFront(&tokens, 1))
            {
                ThrowError(start, file_end, Error {SyntaxError, "Missing ending }"});
            }

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
        else if (tokens[0]->type == "Keyword")
        {
            if (get<0>(GetTokenValue(tokens[0])) == "public" || get<0>(GetTokenValue(tokens[0])) == "static" || get<0>(GetTokenValue(tokens[0])) == "const")
            {
                vector<string> qualifiers = {};

                int end;

                if (get<0>(GetTokenValue(tokens[0])) == "public")
                {
                    qualifiers.push_back("public");

                    end = tokens[0]->end;

                    if (EraseFront(&tokens, 1))
                    {
                        ThrowError(start, file_end, Error {SyntaxError, "Missing ending of statement"});
                    }
                }

                if (get<0>(GetTokenValue(tokens[0])) == "static")
                {
                    qualifiers.push_back("static");

                    end = tokens[0]->end;

                    if (EraseFront(&tokens, 1))
                    {
                        ThrowError(start, file_end, Error {SyntaxError, "Missing ending of statement"});
                    }
                }

                if (get<0>(GetTokenValue(tokens[0])) == "const")
                {
                    qualifiers.push_back("const");

                    end = tokens[0]->end;

                    if (EraseFront(&tokens, 1))
                    {
                        ThrowError(start, file_end, Error {SyntaxError, "Missing ending of statement"});
                    }
                }

                Node *node = new Qualifier(qualifiers);

                node->start = start;
                node->end = end;

                AST.push_back(node);
            }
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

bool ShouldReturn(Token *current_token, vector<Token*> return_tokens)
{
    for (Token *ret_t : return_tokens)
    {
        if (*ret_t == *current_token)
        {
            return true;
        }
    }

    return false;
}

bool EraseFront(vector<Token*> *tokens, int length)
{
    for (int i = 0; i < length; i++)
    {
        tokens->erase(tokens->begin());
    }

    return tokens->size() == 0;
}

void ThrowError(int start, int end, Error error)
{
    Node *node = new Node();

    node->start = start;
    node->end = end;
    node->error = error;

    throw node;
}