#include <string>
#include <vector>
#include <regex>

#include "syntax_analyser.hpp"
#include "token.hpp"
#include "node.hpp"

using namespace std;

pair<vector<Node*>, vector<Token*>> AnalyseSyntax(vector<Token*> tokens, pair<vector<pair<Token*, bool>>, bool> return_flags)
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

        if (get<0>(return_flags).size() != 0 && ShouldReturn(tokens[0], get<0>(return_flags)) || (StatementStarted(AST) && get<1>(return_flags)))
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

                    Type param_type = Type("", false, vector<Type>());

                    if (data[0]->type == "Type")
                    {
                        param_type = *(Type*)data[0];
                    }
                    else if (data[0]->type == "GetVariable")
                    {
                        param_type = Type(((GetVariable*)data[0])->name, false, vector<Type>());
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

                            tie(data, tokens) = AnalyseSyntax(tokens, { GetReturnTokens({ new Control(","), new Bracket(")") }, get<0>(return_flags)), false });

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
                    return_type = Type(((GetVariable*)data[0])->name, false, vector<Type>());
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

                tie(content, tokens) = AnalyseSyntax(tokens, { GetReturnTokens({ new Bracket("}") }, get<0>(return_flags)), false });

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

            tie(content, tokens) = AnalyseSyntax(tokens, { GetReturnTokens({ new Bracket("}") }, get<0>(return_flags)), false });

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
        else if (StatementStarted(AST) && tokens[0]->type == "Operator" && get<0>(GetTokenValue(tokens[0])) == "^")
        {
            Node *left = GetASTEnd(AST);

            AST.pop_back();

            if (EraseFront(&tokens, 1))
            {
                ThrowError(start, file_end, Error {SyntaxError, "Missing right expression for operation"});
            }

            vector<Node*> content;

            tie(content, tokens) = AnalyseSyntax(tokens, { {}, true });

            if (tokens.size() == 0 && StatementStarted(content))
            {
                ThrowError(start, file_end, Error {SyntaxError, "Missing ending ;"});
            }
            else if (tokens.size() == 0)
            {
                ThrowError(start, file_end, Error {SyntaxError, "Missing right expression for operation"});
            }

            if (content.size() == 0)
            {
                ThrowError(start, tokens[0]->end, Error {SyntaxError, "Missing right expression for operation"});
            }
            else if (content[0]->type == "StatementEnd")
            {
                ThrowError(start, tokens[0]->end, Error {SyntaxError, "Missing right expression for operation"});
            }

            Node *right = content[0];

            Operation *operation;

            if (left->type == "Operation" && ((Operation*)left)->left == NULL)
            {
                Operation *unary = (Operation*)left;

                Operation *binary = new Operation("^", unary->right, right);

                binary->start = start;

                binary->end = tokens[0]->end;

                operation = new Operation(unary->operator_string, NULL, binary);

                operation->start = unary->start;

                operation->end = tokens[0]->end;
            }
            else
            {
                operation = new Operation("^", left, right);

                operation->start = start;

                operation->end = tokens[0]->end;
            }

            AST.push_back(operation);
        }
        else if (!StatementStarted(AST) && tokens[0]->type == "Operator" && (get<0>(GetTokenValue(tokens[0])) == "!" || get<0>(GetTokenValue(tokens[0])) == "+" || get<0>(GetTokenValue(tokens[0])) == "-"))
        {
            string operator_string = get<0>(GetTokenValue(tokens[0]));

            if (EraseFront(&tokens, 1))
            {
                ThrowError(start, file_end, Error {SyntaxError, "Missing right expression for operation"});
            }

            vector<Node*> content;

            tie(content, tokens) = AnalyseSyntax(tokens, { {}, true });

            if (tokens.size() == 0 && StatementStarted(content))
            {
                ThrowError(start, file_end, Error {SyntaxError, "Missing ending ;"});
            }
            else if (tokens.size() == 0)
            {
                ThrowError(start, file_end, Error {SyntaxError, "Missing right expression for operation"});
            }

            if (content.size() == 0)
            {
                ThrowError(start, tokens[0]->end, Error {SyntaxError, "Missing right expression for operation"});
            }
            else if (content[0]->type == "StatementEnd")
            {
                ThrowError(start, tokens[0]->end, Error {SyntaxError, "Missing right expression for operation"});
            }

            Node *right = content[0];

            Operation *operation = new Operation(operator_string, NULL, right);

            operation->start = start;

            operation->end = tokens[0]->end;

            AST.push_back(operation);
        }
        else if (StatementStarted(AST) && tokens[0]->type == "Operator")
        {
            OPERATION:
                Node *left = GetASTEnd(AST);

                AST.pop_back();

                Operation *operation;
                
                tie(operation, tokens) = ProcessOperations(return_flags, start, tokens, left);

                // string operator_string = get<0>(GetTokenValue(tokens[0]));

                // if (EraseFront(&tokens, 1))
                // {
                //     ThrowError(start, file_end, Error {SyntaxError, "Missing right expression for operation"});
                // }

                // vector<Node*> content;

                // tie(content, tokens) = AnalyseSyntax(tokens, { GetReturnTokens({ new Control(";") }, get<0>(return_flags)), false });

                // if (tokens.size() == 0)
                // {
                //     ThrowError(start, file_end, Error {SyntaxError, "Missing ending ;"});
                // }

                // if (content.size() == 0)
                // {
                //     ThrowError(start, tokens[0]->end, Error {SyntaxError, "Missing right expression for operation"});
                // }

                // Node *right = content[0];

                // Operation *operation = new Operation(operator_string, left, right);

                // operation->start = start;

                // operation->end = tokens[0]->end;

                AST.push_back(operation);
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

                    tie(data, tokens) = AnalyseSyntax(tokens, { GetReturnTokens({ new Control(","), new Operator(">"), new Control(";") }, get<0>(return_flags)), false });

                    if (tokens.size() == 0 || (tokens.size() != 0 && tokens[0]->type == "Control" && get<0>(GetTokenValue(tokens[0])) == ";"))
                    {
                        tokens = old_tokens;

                        Node *node = new GetVariable(name);

                        node->start = start;

                        node->end = tokens[0]->end;

                        AST.push_back(node);

                        EraseFront(&tokens, 1);

                        goto OPERATION;
                    }

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
                        content.push_back(Type(((GetVariable*)data[0])->name, false, vector<Type>()));

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
                    Type *type = new Type(name, false, content);

                    type->start = start;

                    type->end = tokens[0]->end;

                    AST.push_back(type);

                    EraseFront(&tokens, 1);
                }
            }
            else if (!last && tokens.size() > 2 && tokens[1]->type == "Bracket" && get<0>(GetTokenValue(tokens[1])) == "[" && tokens[2]->type == "Bracket" && get<0>(GetTokenValue(tokens[2])) == "]")
            {
                string name = ((Identifier*)tokens[0])->name;

                Type *type = new Type(name, true, {});

                type->start = start;

                type->end = tokens[2]->end;

                AST.push_back(type);

                EraseFront(&tokens, 3);
            }
            else if (!last && StatementStarted(AST) && (GetASTEnd(AST)->type == "Type" || GetASTEnd(AST)->type == "GetVariable") && tokens[1]->type == "Bracket" && get<0>(GetTokenValue(tokens[1])) == "(")
            {
                Type return_type = Type("", false, vector<Type>());

                Node *n = GetASTEnd(AST);

                AST.pop_back();

                if (n->type == "Type")
                {
                    return_type = *(Type*)n;
                }
                else if (n->type == "GetVariable")
                {
                    return_type = Type(((GetVariable*)n)->name, false, vector<Type>());
                }

                Qualifier *qualifier = new Qualifier({});

                if (StatementStarted(AST) && GetASTEnd(AST)->type == "Qualifier")
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
            else if (!last && StatementStarted(AST) && (GetASTEnd(AST)->type == "Type" || GetASTEnd(AST)->type == "GetVariable") && tokens[1]->type == "Operator" && get<0>(GetTokenValue(tokens[1])) == "=")
            {
                Type variable_type = Type("", false, vector<Type>());

                Node *n = GetASTEnd(AST);

                AST.pop_back();

                if (n->type == "Type")
                {
                    variable_type = *(Type*)n;
                }
                else if (n->type == "GetVariable")
                {
                    variable_type = Type(((GetVariable*)n)->name, false, vector<Type>());
                }

                Qualifier *qualifier = new Qualifier({});

                if (StatementStarted(AST) && GetASTEnd(AST)->type == "Qualifier")
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

                tie(data, tokens) = AnalyseSyntax(tokens, { GetReturnTokens({ new Control(";") }, get<0>(return_flags)), false });

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

                    tie(data, tokens) = AnalyseSyntax(tokens, { GetReturnTokens({ new Control(","), new Bracket(")") }, get<0>(return_flags)), false });

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
                Node *node;

                if (!StatementStarted(AST) || GetASTEnd(AST)->type == "Qualifier")
                {
                    node = new GetVariable(((Identifier*)tokens[0])->name);
                }
                else if (GetASTEnd(AST)->type == "Type" || GetASTEnd(AST)->type == "GetVariable")
                {
                    Type variable_type = Type("", false, vector<Type>());

                    Node *n = GetASTEnd(AST);

                    AST.pop_back();

                    if (n->type == "Type")
                    {
                        variable_type = *(Type*)n;
                    }
                    else if (n->type == "GetVariable")
                    {
                        variable_type = Type(((GetVariable*)n)->name, false, vector<Type>());
                    }

                    Qualifier *qualifier = new Qualifier({});

                    if (StatementStarted(AST) && GetASTEnd(AST)->type == "Qualifier")
                    {
                        qualifier = (Qualifier*)GetASTEnd(AST);

                        AST.pop_back();
                    }

                    node = new AssignVariable(qualifier, variable_type, ((Identifier*)tokens[0])->name, NULL);
                }

                node->start = start;
                node->end = tokens[0]->end;

                AST.push_back(node);

                EraseFront(&tokens, 1);
            }
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

            Node *node = new StatementEnd();

            node->start = start;
            node->end = start + 1;

            AST.push_back(node);
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

bool ShouldReturn(Token *current_token, vector<pair<Token*, bool>> return_tokens)
{
    for (pair<Token*, bool> ret_t : return_tokens)
    {
        if (*get<0>(ret_t) == *current_token)
        {
            return true;
        }
    }

    return false;
}

bool StatementStarted(std::vector<Node*> AST)
{
    if (GetASTEnd(AST) == NULL || GetASTEnd(AST)->type == "StatementEnd")
    {
        return false;
    }

    return true;
}

vector<pair<Token*, bool>> GetReturnTokens(vector<Token*> new_return_tokens, vector<pair<Token*, bool>> existing_return_tokens)
{
    vector<pair<Token*, bool>> return_tokens = {};

    for (Token* new_token : new_return_tokens)
    {
        return_tokens.push_back({new_token, false});
    }

    for (pair<Token*, bool> existing_token : existing_return_tokens) {
        if (find(return_tokens.begin(), return_tokens.end(), pair<Token*, bool> {get<0>(existing_token), false}) != return_tokens.end())
        {
            continue;
        }

        return_tokens.push_back(pair<Token*, bool> {get<0>(existing_token), true});
    }

    return return_tokens;
}

pair<Operation*, vector<Token*>> ProcessOperations(pair<vector<pair<Token*, bool>>, bool> return_flags, int start, vector<Token*> tokens, Node *left)
{
    int file_end = tokens[tokens.size()-1]->end;

    vector<Node*> values = { left };

    vector<string> operators = { };

    bool operation_end = false;

    while (!operation_end)
    {
        if (tokens[0]->type != "Operator")
        {
            operation_end = true;
            break;
        }

        operators.push_back(((Operator*)(tokens[0]))->value);

        if (EraseFront(&tokens, 1))
        {
            ThrowError(start, file_end, Error {SyntaxError, "Missing right expression for operation"});
        }

        vector<Node*> data;

        tie(data, tokens) = AnalyseSyntax(tokens, { {}, true });

        if (tokens.size() == 0 && StatementStarted(data))
        {
            ThrowError(start, file_end, Error {SyntaxError, "Missing ending ;"});
        }
        else if (tokens.size() == 0)
        {
            ThrowError(start, file_end, Error {SyntaxError, "Missing right expression for operation"});
        }

        if (data.size() == 0)
        {
            ThrowError(start, tokens[0]->end, Error {SyntaxError, "Invalid character in operation"});
        }
        else if (GetASTEnd(data)->type == "StatementEnd")
        {
            ThrowError(start, data[0]->end, Error {SyntaxError, "Missing right expression for operation"});
        }

        values.push_back(data[0]);
    }

    Operation *operation = new Operation(operators[0], values[0], NULL);

    for (int i = 1; i < operators.size(); i++)
    {
        Node *ending = NULL;
        
        if (i == operators.size() - 1)
        {
            ending = values[values.size() - 1];
        }

        if (CompareOperator(operation->operator_string, operators[i]))
        {
            Operation *current_op = operation;

            while (current_op->right != NULL)
            {
                current_op = (Operation*)current_op->right;
            }

            current_op->right = values[i];

            //operation->right = values[i];
            operation = new Operation(operators[i], operation, ending);
        }
        else
        {
            Operation *current_op = operation;

            while (true)
            {
                if (current_op->right == NULL)
                {
                    current_op->right = new Operation(operators[i], values[i], ending);
                    break;
                }
                
                if (CompareOperator(((Operation*)current_op->right)->operator_string, operators[i]))
                {
                    Operation *op = (Operation*)current_op->right;
                    op->right = values[i];
                    current_op->right = new Operation(operators[i], op, ending);

                    break;
                }
                else
                {
                    current_op = (Operation*)current_op->right;
                }
            }
        }
    }

    if (operation->right == NULL)
    {
        operation->right = values[values.size() - 1];
    }

    AddStartAndEnd(operation);

    return {operation, tokens};
}

bool CompareOperator(string operator1, string operator2)
{
    smatch m;

    int op1_level;
    int op2_level;

    for (int i = 0; i < Operator_Preference.size(); i++)
    {
        if (regex_search(operator1, m, regex(Operator_Preference[i])))
        {
            op1_level = i;
        }

        if (regex_search(operator2, m, regex(Operator_Preference[i])))
        {
            op2_level = i;
        }
    }

    return op1_level <= op2_level;
}

void AddStartAndEnd(Operation *operation)
{
    if (InMultiExpression(operation->left))
    {
        AddStartAndEnd((Operation*)operation->left);
    }
    
    operation->start = operation->left->start;

    if (InMultiExpression(operation->right))
    {
        AddStartAndEnd((Operation*)operation->right);
    }

    operation->end = operation->right->end;
}

bool InMultiExpression(Node *node)
{
    if (node->type != "Operation")
    {
        return false;
    }

    Operation *operation = (Operation*)node;

    if (operation->operator_string == "^" || operation->left == NULL)
    {
        return false;
    }

    return true;
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