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

        if (tokens[0]->type == "Integer" || tokens[0]->type == "Float" || tokens[0]->type == "String" || tokens[0]->type == "Boolean")
        {
            Literal *l = new Literal();

            l->start = start;
            l->end = tokens[0]->end;

            string type = tokens[0]->type;

            tuple<string, int, double, bool> vals = GetTokenValue(tokens[0]);

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
            else if (type == "Boolean")
            {
                l->l_boolean = get<3>(vals);
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

            vector<ParameterExpression> parameters = vector<ParameterExpression>();

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

                if (data.size() != 0 && (data[0]->type == "TypeExpression" || data[0]->type == "GetVariable"))
                {
                    got_arg = true;

                    TypeExpression param_type = TypeExpression("void", false, vector<TypeExpression>());

                    if (data[0]->type == "TypeExpression")
                    {
                        param_type = *(TypeExpression*)data[0];
                    }
                    else if (data[0]->type == "GetVariable")
                    {
                        param_type = TypeExpression(((GetVariable*)data[0])->name, false, vector<TypeExpression>());
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

                            tie(data, tokens) = AnalyseSyntax(tokens, { { { new Control(","), false }, { new Bracket(")"), false } }, false });

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

                        parameters.push_back(ParameterExpression(param_type, param_name, param_def_value, param_expansion));
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

            TypeExpression return_type = TypeExpression("void", false, {});

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

                if (data[0]->type == "TypeExpression")
                {
                    return_type = *(TypeExpression*)data[0];
                }
                else if (data[0]->type == "GetVariable")
                {
                    return_type = TypeExpression(((GetVariable*)data[0])->name, false, vector<TypeExpression>());
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

                tie(content, tokens) = AnalyseSyntax(tokens, { { { new Bracket("}"), false } }, false });

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

            tie(content, tokens) = AnalyseSyntax(tokens, { { { new Bracket("}"), false } }, false });

            if (tokens.size() == 0)
            {
                ThrowError(start, file_end, Error {SyntaxError, "Missing ending }"});
            }

            CodeBlock *code_block = new CodeBlock(TypeExpression("void", false, {}), {}, content);

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

                vector<TypeExpression> content = vector<TypeExpression>();

                bool is_type = true;

                bool got_arg = false;

                while (!(tokens[0]->type == "Operator" && get<0>(GetTokenValue(tokens[0])) == ">"))
                {
                    vector<Node*> data;

                    tie(data, tokens) = AnalyseSyntax(tokens, { GetReturnTokens({ new Control(","), new Operator(">") }, get<0>(return_flags)), false });

                    if (tokens.size() == 0 || (tokens.size() != 0 && get<0>(GetTokenValue(tokens[0])) != "," && get<0>(GetTokenValue(tokens[0])) != ">"))
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

                    if (data[0]->type == "TypeExpression")
                    {
                        content.push_back(*(TypeExpression*)data[0]);

                        got_arg = true;
                    }
                    else if (data[0]->type == "GetVariable")
                    {
                        content.push_back(TypeExpression(((GetVariable*)data[0])->name, false, vector<TypeExpression>()));

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
                    TypeExpression *type = new TypeExpression(name, false, content);

                    type->start = start;

                    type->end = tokens[0]->end;

                    AST.push_back(type);

                    EraseFront(&tokens, 1);
                }
            }
            else if (!last && tokens.size() > 2 && tokens[1]->type == "Bracket" && get<0>(GetTokenValue(tokens[1])) == "[" && tokens[2]->type == "Bracket" && get<0>(GetTokenValue(tokens[2])) == "]")
            {
                string name = ((Identifier*)tokens[0])->name;

                TypeExpression *type = new TypeExpression(name, true, {});

                type->start = start;

                type->end = tokens[2]->end;

                AST.push_back(type);

                EraseFront(&tokens, 3);
            }
            else if (!last && StatementStarted(AST) && (GetASTEnd(AST)->type == "TypeExpression" || GetASTEnd(AST)->type == "GetVariable") && tokens[1]->type == "Bracket" && get<0>(GetTokenValue(tokens[1])) == "(")
            {
                TypeExpression *return_type = NULL;

                Node *n = GetASTEnd(AST);

                AST.pop_back();

                if (n->type == "TypeExpression")
                {
                    return_type = (TypeExpression*)n;
                }
                else if (n->type == "GetVariable")
                {
                    return_type = new TypeExpression(((GetVariable*)n)->name, false, vector<TypeExpression>());
                }

                QualifierExpression *qualifier = new QualifierExpression({});

                if (StatementStarted(AST) && GetASTEnd(AST)->type == "QualifierExpression")
                {
                    qualifier = (QualifierExpression*)GetASTEnd(AST);

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

                code_block->return_type = *return_type;

                vector<TypeExpression> parameter_types = {};

                for (ParameterExpression p : code_block->parameters)
                {
                    parameter_types.push_back(p.type_data);
                }

                TypeExpression func_type = TypeExpression("Function", false, parameter_types);
                func_type.content.push_back(*return_type);

                func_type.start = start;
                func_type.end = code_block->end;

                Node *node = new DeclareVariable(qualifier, func_type, name, code_block);

                node->start = start;
                node->end = code_block->end;

                AST.push_back(node);

                StatementEnd *statement_end = new StatementEnd();

                statement_end->end = code_block->end - 1;
                statement_end->start = code_block->end;

                AST.push_back(statement_end);
            }
            else if (!last && StatementStarted(AST) && (GetASTEnd(AST)->type == "TypeExpression" || GetASTEnd(AST)->type == "GetVariable") && tokens[1]->type == "Operator" && get<0>(GetTokenValue(tokens[1])) == "=")
            {
                TypeExpression variable_type = TypeExpression("void", false, vector<TypeExpression>());

                Node *n = GetASTEnd(AST);

                AST.pop_back();

                if (n->type == "TypeExpression")
                {
                    variable_type = *(TypeExpression*)n;
                }
                else if (n->type == "GetVariable")
                {
                    variable_type = TypeExpression(((GetVariable*)n)->name, false, vector<TypeExpression>());
                }

                QualifierExpression *qualifier = new QualifierExpression({});

                if (StatementStarted(AST) && GetASTEnd(AST)->type == "QualifierExpression")
                {
                    qualifier = (QualifierExpression*)GetASTEnd(AST);

                    AST.pop_back();
                }

                string name = get<0>(GetTokenValue(tokens[0]));

                if (EraseFront(&tokens, 2))
                {
                    ThrowError(start, file_end, Error {SyntaxError, "Missing end of declaration"});
                }

                vector<Node*> data;

                tie(data, tokens) = AnalyseSyntax(tokens, { GetReturnTokens({ new Control(";") }, get<0>(return_flags)), false });

                if (tokens.size() == 0)
                {
                    ThrowError(start, file_end, Error {SyntaxError, "Missing ending ;"});
                }

                if (data.size() == 0)
                {
                    ThrowError(start, tokens[0]->end, Error {SyntaxError, "Missing declaration value"});
                }

                Node *content = data[0];

                Node *node = new DeclareVariable(qualifier, variable_type, name, content);

                node->start = start;
                node->end = tokens[0]->end;

                AST.push_back(node);
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

                    tie(data, tokens) = AnalyseSyntax(tokens, { { { new Control(","), false }, { new Bracket(")"), false } }, false });

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
            else if (!last && tokens[1]->type == "Control" && get<0>(GetTokenValue(tokens[1])) == ".")
            {
                string name = ((Identifier*)tokens[0])->name;

                if (EraseFront(&tokens, 2))
                {
                    ThrowError(start, file_end, Error {SyntaxError, "Missing end of statement"});
                }

                vector<Node*> data;

                tie(data, tokens) = AnalyseSyntax(tokens, { GetReturnTokens({ new Control(";") }, get<0>(return_flags)), false });

                if (tokens.size() == 0)
                {
                    ThrowError(start, file_end, Error {SyntaxError, "Missing ending ;"});
                }

                if (data.size() == 0)
                {
                    ThrowError(start, tokens[0]->end, Error {SyntaxError, "Missing member access"});
                }

                if (data.size() > 1)
                {
                    ThrowError(start, tokens[0]->end, Error {SyntaxError, "Too many expressions"});
                }

                if (data[0]->type != "GetVariable" && data[0]->type != "FunctionCall" && data[0]->type != "MemberAccess")
                {
                    ThrowError(start, tokens[0]->end, Error {SyntaxError, "Invalid expression in member access"});
                }

                MemberAccess *member_access = new MemberAccess(name, data[0]);

                member_access->start = start;
                member_access->end = tokens[0]->end;

                AST.push_back(member_access);
            }
            else
            {
                Node *node;

                if (!StatementStarted(AST) || GetASTEnd(AST)->type == "QualifierExpression")
                {
                    node = new GetVariable(((Identifier*)tokens[0])->name);
                }
                else if (GetASTEnd(AST)->type == "TypeExpression" || GetASTEnd(AST)->type == "GetVariable")
                {
                    TypeExpression variable_type = TypeExpression("void", false, vector<TypeExpression>());

                    Node *n = GetASTEnd(AST);

                    AST.pop_back();

                    if (n->type == "TypeExpression")
                    {
                        variable_type = *(TypeExpression*)n;
                    }
                    else if (n->type == "GetVariable")
                    {
                        variable_type = TypeExpression(((GetVariable*)n)->name, false, vector<TypeExpression>());
                    }

                    QualifierExpression *qualifier = new QualifierExpression({});

                    if (StatementStarted(AST) && GetASTEnd(AST)->type == "QualifierExpression")
                    {
                        qualifier = (QualifierExpression*)GetASTEnd(AST);

                        AST.pop_back();
                    }

                    node = new DeclareVariable(qualifier, variable_type, ((Identifier*)tokens[0])->name, NULL);
                }
                else
                {
                    ThrowError(start, tokens[0]->end, Error {SyntaxError, "Missing ending ;"});
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

                Node *node = new QualifierExpression(qualifiers);

                node->start = start;
                node->end = end;

                AST.push_back(node);
            }
            else if (get<0>(GetTokenValue(tokens[0])) == "class")
            {
                if (EraseFront(&tokens, 1))
                {
                    ThrowError(start, file_end, Error {SyntaxError, "Missing end of statement"});
                }

                if (tokens[0]->type != "Identifier")
                {
                    ThrowError(start, tokens[0]->end, Error {SyntaxError, "Invalid character in class definition"});
                }

                string name = ((Identifier*)tokens[0])->name;

                if (EraseFront(&tokens, 1))
                {
                    ThrowError(start, file_end, Error {SyntaxError, "Missing end of statement"});
                }

                string interface = "";

                if (tokens[0]->type == "Control" && ((Control*)tokens[0])->value == ":")
                {
                    if (EraseFront(&tokens, 1))
                    {
                        ThrowError(start, file_end, Error {SyntaxError, "Missing end of statement"});
                    }

                    if (tokens[0]->type != "Identifier")
                    {
                        ThrowError(start, tokens[0]->end, Error {SyntaxError, "Invalid character in class definition"});
                    }

                    interface = ((Identifier*)tokens[0])->name;

                    if (EraseFront(&tokens, 1))
                    {
                        ThrowError(start, file_end, Error {SyntaxError, "Missing end of statement"});
                    }
                }

                if (tokens[0]->type != "Bracket" || ((Bracket*)tokens[0])->value != "{")
                {
                    ThrowError(start, tokens[0]->end, Error {SyntaxError, "Invalid character in class definition"});
                }

                if (EraseFront(&tokens, 1))
                {
                    ThrowError(start, file_end, Error {SyntaxError, "Missing end of statement"});
                }

                vector<Node*> data;

                tie(data, tokens) = AnalyseSyntax(tokens, { { { new Bracket("}"), false } }, false });

                if (tokens.size() == 0)
                {
                    ThrowError(start, file_end, Error {SyntaxError, "Missing ending }"});
                }

                for (Node *node : data)
                {
                    if (node->type != "DeclareVariable" && node->type != "StatementEnd")
                    {
                        ThrowError(node->start, node->end, Error {SyntaxError, "Expected declaration"});
                    }
                }

                ClassDefinition *class_definition = new ClassDefinition(name, interface, data);

                class_definition->start = start;
                class_definition->end = tokens[0]->end;

                AST.push_back(class_definition);

                EraseFront(&tokens, 1);
            }
            else if (get<0>(GetTokenValue(tokens[0])) == "new")
            {
                if (EraseFront(&tokens, 1))
                {
                    ThrowError(start, file_end, Error {SyntaxError, "Missing end of statement"});
                }

                vector<Node*> data;

                tie(data, tokens) = AnalyseSyntax(tokens, { {}, true });

                if (data.size() == 0)
                {
                    ThrowError(tokens[0]->start, file_end, Error {SyntaxError, "Missing end of statement"});
                }

                if (data[0]->type != "FunctionCall")
                {
                    ThrowError(tokens[0]->start, data[0]->end, Error {SyntaxError, "Invalid character in class instance"});
                }

                FunctionCall *function_call = (FunctionCall*)data[0];

                InstanceClass *instance_class = new InstanceClass(function_call->name, function_call->arguments);

                instance_class->start = start;
                instance_class->end = function_call->end;

                AST.push_back(instance_class);
            }
            else if (get<0>(GetTokenValue(tokens[0])) == "if")
            {
                if (EraseFront(&tokens, 1))
                {
                    ThrowError(start, file_end, Error {SyntaxError, "Missing end of statement"});
                }

                if (tokens[0]->type != "Bracket" || ((Bracket*)tokens[0])->value != "(")
                {
                    ThrowError(start, file_end, Error {SyntaxError, "Missing starting ("});
                }

                if (EraseFront(&tokens, 1))
                {
                    ThrowError(start, file_end, Error {SyntaxError, "Missing end of statement"});
                }

                vector<Node*> data;

                tie(data, tokens) = AnalyseSyntax(tokens, { { { new Bracket(")"), false } }, false });

                if (tokens.size() == 0)
                {
                    ThrowError(start, file_end, Error {SyntaxError, "Missing ending )"});
                }

                if (data.size() == 0)
                {
                    ThrowError(tokens[0]->start, tokens[0]->end, Error {SyntaxError, "Missing expression"});
                }

                Node *if_expression = data[0];

                if (EraseFront(&tokens, 1))
                {
                    ThrowError(start, file_end, Error {SyntaxError, "Missing end of statement"});
                }

                tie(data, tokens) = AnalyseSyntax(tokens, { {}, true });

                if (data.size() == 0)
                {
                    ThrowError(start, file_end, Error {SyntaxError, "Missing end of statement"});
                }

                if (data[0]->type != "CodeBlock")
                {
                    ThrowError(start, data[0]->end, Error {SyntaxError, "Invalid character in if statement"});
                }

                CodeBlock *if_code_block = (CodeBlock*)data[0];

                IfStatement *if_statement = new IfStatement(if_expression, if_code_block, {}, {}, NULL);

                int end = if_code_block->end;

                if (tokens.size() != 0)
                {

                    while (true)
                    {
                        if (tokens.size() < 2 || tokens[0]->type != "Keyword" || ((Keyword*)tokens[0])->name != "else" || tokens[1]->type != "Keyword" || ((Keyword*)tokens[1])->name != "if")
                        {
                            break;
                        }

                        int else_if_start = tokens[0]->start;

                        if (EraseFront(&tokens, 2))
                        {
                            ThrowError(else_if_start, file_end, Error {SyntaxError, "Missing end of statement"});
                        }

                        if (tokens[0]->type != "Bracket" || ((Bracket*)tokens[0])->value != "(")
                        {
                            ThrowError(else_if_start, file_end, Error {SyntaxError, "Missing starting ("});
                        }

                        if (EraseFront(&tokens, 1))
                        {
                            ThrowError(else_if_start, file_end, Error {SyntaxError, "Missing end of statement"});
                        }

                        tie(data, tokens) = AnalyseSyntax(tokens, { { { new Bracket(")"), false } }, false });

                        if (tokens.size() == 0)
                        {
                            ThrowError(else_if_start, file_end, Error {SyntaxError, "Missing ending )"});
                        }

                        if (data.size() == 0)
                        {
                            ThrowError(tokens[0]->start, tokens[0]->end, Error {SyntaxError, "Missing expression"});
                        }

                        Node *else_if_expression = data[0];

                        if (EraseFront(&tokens, 1))
                        {
                            ThrowError(else_if_start, file_end, Error {SyntaxError, "Missing end of statement"});
                        }

                        tie(data, tokens) = AnalyseSyntax(tokens, { {}, true });

                        if (data.size() == 0)
                        {
                            ThrowError(else_if_start, file_end, Error {SyntaxError, "Missing end of statement"});
                        }

                        if (data[0]->type != "CodeBlock")
                        {
                            ThrowError(else_if_start, data[0]->end, Error {SyntaxError, "Invalid character in if statement"});
                        }

                        CodeBlock *else_if_code_block = (CodeBlock*)data[0];

                        if_statement->else_if_expressions.push_back(else_if_expression);
                        if_statement->else_if_code_blocks.push_back(else_if_code_block);

                        end = else_if_code_block->end;
                    }

                    if (tokens[0]->type != "Keyword" || ((Keyword*)tokens[0])->name != "else")
                    {
                        if_statement->start = start;
                        if_statement->end = end;

                        AST.push_back(if_statement);

                        continue;
                    }

                    int else_start = tokens[0]->start;

                    if (EraseFront(&tokens, 1))
                    {
                        ThrowError(else_start, file_end, Error {SyntaxError, "Missing end of statement"});
                    }

                    tie(data, tokens) = AnalyseSyntax(tokens, { {}, true });

                    if (data.size() == 0)
                    {
                        ThrowError(else_start, file_end, Error {SyntaxError, "Missing end of statement"});
                    }

                    if (data[0]->type != "CodeBlock")
                    {
                        ThrowError(else_start, data[0]->end, Error {SyntaxError, "Invalid character in if statement"});
                    }

                    CodeBlock *else_code_block = (CodeBlock*)data[0];

                    if_statement->else_code_block = else_code_block;

                    end = else_code_block->end;
                }

                if_statement->start = start;
                if_statement->end = end;

                AST.push_back(if_statement);
            }
            else if (get<0>(GetTokenValue(tokens[0])) == "switch")
            {
                if (EraseFront(&tokens, 1))
                {
                    ThrowError(start, file_end, Error {SyntaxError, "Missing end of statement"});
                }

                if (tokens[0]->type != "Bracket" || ((Bracket*)tokens[0])->value != "(")
                {
                    ThrowError(start, file_end, Error {SyntaxError, "Missing starting ("});
                }

                if (EraseFront(&tokens, 1))
                {
                    ThrowError(start, file_end, Error {SyntaxError, "Missing end of statement"});
                }

                vector<Node*> data;

                tie(data, tokens) = AnalyseSyntax(tokens, { { { new Bracket(")"), false } }, false });

                if (tokens.size() == 0)
                {
                    ThrowError(start, file_end, Error {SyntaxError, "Missing ending )"});
                }

                if (data.size() == 0)
                {
                    ThrowError(tokens[0]->start, tokens[0]->end, Error {SyntaxError, "Missing expression"});
                }

                Node *switch_expression = data[0];

                if (EraseFront(&tokens, 1))
                {
                    ThrowError(start, file_end, Error {SyntaxError, "Missing end of statement"});
                }

                if (tokens[0]->type != "Bracket" || ((Bracket*)tokens[0])->value != "{")
                {
                    ThrowError(start, file_end, Error {SyntaxError, "Missing starting {"});
                }

                if (EraseFront(&tokens, 1))
                {
                    ThrowError(start, file_end, Error {SyntaxError, "Missing end of statement"});
                }

                SwitchStatement *switch_statement = new SwitchStatement(switch_expression, {}, {}, NULL);

                while (tokens[0]->type != "Bracket" || ((Bracket*)tokens[0])->value != "}")
                {
                    if (tokens[0]->type == "Keyword" && ((Keyword*)tokens[0])->name == "default")
                    {
                        if (EraseFront(&tokens, 1))
                        {
                            ThrowError(start, file_end, Error {SyntaxError, "Missing end of statement"});
                        }

                        tie(data, tokens) = AnalyseSyntax(tokens, { {}, true });

                        if (tokens.size() == 0)
                        {
                            ThrowError(start, file_end, Error {SyntaxError, "Missing ending }"});
                        }

                        if (data.size() == 0)
                        {
                            ThrowError(start, file_end, Error {SyntaxError, "Missing end of statement"});
                        }

                        if (data[0]->type != "CodeBlock")
                        {
                            ThrowError(start, data[0]->end, Error {SyntaxError, "Invalid character in switch statement"});
                        }

                        CodeBlock *default_code_block = (CodeBlock*)data[0];

                        switch_statement->default_code_block = default_code_block;

                        if (tokens[0]->type != "Bracket" || ((Bracket*)tokens[0])->value != "}")
                        {
                            ThrowError(start, tokens[0]->end, Error {SyntaxError, "Missing ending }"});
                        }

                        break;
                    }

                    if (tokens[0]->type != "Keyword" || ((Keyword*)tokens[0])->name != "case")
                    {
                        ThrowError(start, file_end, Error {SyntaxError, "Invalid character in switch statement"});
                    }

                    int case_start = tokens[0]->start;

                    if (EraseFront(&tokens, 1))
                    {
                        ThrowError(case_start, file_end, Error {SyntaxError, "Missing end of statement"});
                    }

                    if (tokens[0]->type != "Bracket" || ((Bracket*)tokens[0])->value != "(")
                    {
                        ThrowError(case_start, file_end, Error {SyntaxError, "Missing starting ("});
                    }

                    if (EraseFront(&tokens, 1))
                    {
                        ThrowError(case_start, file_end, Error {SyntaxError, "Missing end of statement"});
                    }

                    vector<Node*> data;

                    tie(data, tokens) = AnalyseSyntax(tokens, { { { new Bracket(")"), false } }, false });

                    if (tokens.size() == 0)
                    {
                        ThrowError(case_start, file_end, Error {SyntaxError, "Missing ending )"});
                    }

                    if (data.size() == 0)
                    {
                        ThrowError(tokens[0]->start, tokens[0]->end, Error {SyntaxError, "Missing expression"});
                    }

                    Node *case_expression = data[0];

                    if (EraseFront(&tokens, 1))
                    {
                        ThrowError(case_start, file_end, Error {SyntaxError, "Missing end of statement"});
                    }

                    tie(data, tokens) = AnalyseSyntax(tokens, { {}, true });

                    if (tokens.size() == 0)
                    {
                        ThrowError(start, file_end, Error {SyntaxError, "Missing ending }"});
                    }

                    if (data.size() == 0)
                    {
                        ThrowError(case_start, file_end, Error {SyntaxError, "Missing end of statement"});
                    }

                    if (data[0]->type != "CodeBlock")
                    {
                        ThrowError(case_start, data[0]->end, Error {SyntaxError, "Invalid character in switch statement"});
                    }

                    CodeBlock *case_code_block = (CodeBlock*)data[0];

                    switch_statement->case_expressions.push_back(case_expression);
                    switch_statement->case_code_blocks.push_back(case_code_block);
                }

                switch_statement->start = start;
                switch_statement->end = tokens[0]->end;

                AST.push_back(switch_statement);

                EraseFront(&tokens, 1);
            }
            else if (get<0>(GetTokenValue(tokens[0])) == "for")
            {
                if (EraseFront(&tokens, 1))
                {
                    ThrowError(start, file_end, Error {SyntaxError, "Missing end of statement"});
                }

                if (tokens[0]->type != "Bracket" || ((Bracket*)tokens[0])->value != "(")
                {
                    ThrowError(start, file_end, Error {SyntaxError, "Missing starting ("});
                }

                if (EraseFront(&tokens, 1))
                {
                    ThrowError(start, file_end, Error {SyntaxError, "Missing end of statement"});
                }

                vector<Node*> data;

                tie(data, tokens) = AnalyseSyntax(tokens, { { { new Bracket(")"), false }, { new Control(":"), false } }, false });

                if (tokens.size() == 0)
                {
                    ThrowError(start, file_end, Error {SyntaxError, "Missing ending )"});
                }

                if (tokens[0]->type == "Control" && ((Control*)tokens[0])->value == ":")
                {
                    if (data.size() != 1 || data[0]->type != "DeclareVariable" || ((DeclareVariable*)data[0])->value != NULL)
                    {
                        ThrowError(start, tokens[0]->end, Error {SyntaxError, "Invalid expression in for loop"});
                    }

                    DeclareVariable *declaration = (DeclareVariable*)data[0];

                    if (EraseFront(&tokens, 1))
                    {
                        ThrowError(start, file_end, Error {SyntaxError, "Missing end of statement"});
                    }

                    vector<Node*> data;

                    tie(data, tokens) = AnalyseSyntax(tokens, { { { new Bracket(")"), false } }, false });

                    if (tokens.size() == 0)
                    {
                        ThrowError(start, file_end, Error {SyntaxError, "Missing ending )"});
                    }

                    if (data.size() == 0)
                    {
                        ThrowError(declaration->end, tokens[0]->end, Error {SyntaxError, "Missing iteration expression"});
                    }

                    Node *iteration_expression = data[0];

                    if (EraseFront(&tokens, 1))
                    {
                        ThrowError(start, file_end, Error {SyntaxError, "Missing end of statement"});
                    }

                    tie(data, tokens) = AnalyseSyntax(tokens, { {}, true });

                    if (data.size() == 0)
                    {
                        ThrowError(start, file_end, Error {SyntaxError, "Missing end of statement"});
                    }

                    if (data[0]->type != "CodeBlock")
                    {
                        ThrowError(start, data[0]->end, Error {SyntaxError, "Invalid character in for loop"});
                    }

                    CodeBlock *for_code_block = (CodeBlock*)data[0];

                    ForEachLoop *for_each_loop = new ForEachLoop(declaration, iteration_expression, for_code_block);

                    for_each_loop->start = start;
                    for_each_loop->end = for_code_block->end;

                    AST.push_back(for_each_loop);
                }
                else
                {
                    vector<Node*> expressions = {};

                    Node *current_expression = NULL;

                    for (int i = 0; i < data.size(); i++)
                    {
                        if (data[i]->type != "StatementEnd")
                        {
                            if (current_expression != NULL)
                            {
                                ThrowError(start, data[i]->end, Error {SyntaxError, "Missing ;"});
                            }

                            current_expression = data[i];
                        }
                        else
                        {
                            expressions.push_back(current_expression);

                            current_expression = NULL;
                        }
                    }

                    if (current_expression != NULL || (data[data.size() - 1]->type == "StatementEnd" && expressions.size() == 2))
                    {
                        expressions.push_back(current_expression);
                    }

                    if (expressions.size() < 3)
                    {
                        ThrowError(tokens[0]->start, tokens[0]->end, Error {SyntaxError, "Missing expression(s)"});
                    }

                    if (expressions.size() > 3)
                    {
                        ThrowError(tokens[0]->start, tokens[0]->end, Error {SyntaxError, "Too many expressions"});
                    }

                    if (EraseFront(&tokens, 1))
                    {
                        ThrowError(start, file_end, Error {SyntaxError, "Missing end of statement"});
                    }

                    tie(data, tokens) = AnalyseSyntax(tokens, { {}, true });

                    if (data.size() == 0)
                    {
                        ThrowError(start, file_end, Error {SyntaxError, "Missing end of statement"});
                    }

                    if (data[0]->type != "CodeBlock")
                    {
                        ThrowError(start, data[0]->end, Error {SyntaxError, "Invalid character in for loop"});
                    }

                    CodeBlock *for_code_block = (CodeBlock*)data[0];

                    ForLoop *for_loop = new ForLoop(expressions[0], expressions[1], expressions[2], for_code_block);

                    for_loop->start = start;
                    for_loop->end = for_code_block->end;

                    AST.push_back(for_loop);
                }
            }
            else if (get<0>(GetTokenValue(tokens[0])) == "while")
            {
                if (EraseFront(&tokens, 1))
                {
                    ThrowError(start, file_end, Error {SyntaxError, "Missing end of statement"});
                }

                if (tokens[0]->type != "Bracket" || ((Bracket*)tokens[0])->value != "(")
                {
                    ThrowError(start, file_end, Error {SyntaxError, "Missing starting ("});
                }

                if (EraseFront(&tokens, 1))
                {
                    ThrowError(start, file_end, Error {SyntaxError, "Missing end of statement"});
                }

                vector<Node*> data;

                tie(data, tokens) = AnalyseSyntax(tokens, { { { new Bracket(")"), false } }, false });

                if (tokens.size() == 0)
                {
                    ThrowError(start, file_end, Error {SyntaxError, "Missing ending )"});
                }

                if (data.size() == 0)
                {
                    ThrowError(tokens[0]->start, tokens[0]->end, Error {SyntaxError, "Missing expression"});
                }

                Node *while_condition = data[0];

                if (EraseFront(&tokens, 1))
                {
                    ThrowError(start, file_end, Error {SyntaxError, "Missing end of statement"});
                }

                tie(data, tokens) = AnalyseSyntax(tokens, { {}, true });

                if (data.size() == 0)
                {
                    ThrowError(start, file_end, Error {SyntaxError, "Missing end of statement"});
                }

                if (data[0]->type != "CodeBlock")
                {
                    ThrowError(start, data[0]->end, Error {SyntaxError, "Invalid character in if statement"});
                }

                CodeBlock *while_code_block = (CodeBlock*)data[0];

                WhileLoop *while_loop = new WhileLoop(while_condition, while_code_block);

                while_loop->start = start;
                while_loop->end = while_code_block->end;

                AST.push_back(while_loop);
            }
            else if (get<0>(GetTokenValue(tokens[0])) == "return")
            {
                int end = tokens[0]->end;

                EraseFront(&tokens, 1);

                vector<Node*> data;

                tie(data, tokens) = AnalyseSyntax(tokens, { GetReturnTokens({ new Control(";") }, get<0>(return_flags)), false });

                if (tokens.size() == 0)
                {
                    ThrowError(start, file_end, Error {SyntaxError, "Missing ending ;"});
                }

                Return *return_statement = new Return(NULL);

                if (data.size() != 0)
                {
                    return_statement->expression = data[0];
                }

                return_statement->start = start;
                return_statement->end = end;

                AST.push_back(return_statement);
            }
            else if (get<0>(GetTokenValue(tokens[0])) == "break")
            {
                int end = tokens[0]->end;

                EraseFront(&tokens, 1);

                Break *break_statement = new Break();

                break_statement->start = start;
                break_statement->end = end;

                AST.push_back(break_statement);
            }
            else if (get<0>(GetTokenValue(tokens[0])) == "continue")
            {
                int end = tokens[0]->end;

                EraseFront(&tokens, 1);

                Continue *continue_statement = new Continue();

                continue_statement->start = start;
                continue_statement->end = end;

                AST.push_back(continue_statement);
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
    Node *n = GetASTEnd(AST);

    if (GetASTEnd(AST) == NULL || GetASTEnd(AST)->type == "StatementEnd" || GetASTEnd(AST)->type == "ClassDefinition" ||
        GetASTEnd(AST)->type == "IfStatement" || GetASTEnd(AST)->type == "SwitchStatememnt" || GetASTEnd(AST)->type == "ForLoop" ||
        GetASTEnd(AST)->type == "ForEachLoop" || GetASTEnd(AST)->type == "WhileLoop")
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

        if (tokens->size() == 0)
        {
            return true;
        }
    }

    return tokens->size() == 0;
}