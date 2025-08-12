#include <string>
#include <vector>
#include <regex>

#include "syntax_analyser.hpp"
#include "token.hpp"
#include "node.hpp"

using namespace std;

pair<vector<Node*>, vector<Token*>> AnalyseSyntax(vector<Token*> tokens, pair<Token*, bool> return_flags)
{
    vector<Node*> AST = vector<Node*>();

    while (tokens.size() != 0)
    {
        if (get<0>(return_flags) == tokens[0] || (get<1>(return_flags) && AST.size() > 0))
        {
            return {AST, tokens};
        }

        if (tokens[0]->type == "Integer" || tokens[0]->type == "Float" || tokens[0]->type == "String")
        {
            Literal *l = new Literal();

            tuple<string, int, float> vals = GetTokenValue(tokens[0]);

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
            if (tokens[1]->type == "Identifier")
            {
                if (tokens[2]->type == "Bracket" && get<0>(GetTokenValue(tokens[2])) == "(")
                {
                    int start = tokens[0]->start;

                    string return_type = get<0>(GetTokenValue(tokens[0]));
                    string name = get<0>(GetTokenValue(tokens[1]));

                    EraseFront(&tokens, 3);

                    vector<tuple<string, string, optional<Node*>>> parameters = vector<tuple<string, string, optional<Node*>>>();

                    while (!(tokens[0]->type == "Bracket" && get<0>(GetTokenValue(tokens[0])) == ")"))
                    {
                        if (tokens[0]->type == "Identifier" && tokens[1]->type == "Identifier")
                        {   
                            string param_type = ((Identifier*)tokens[0])->name;
                            string param_name = ((Identifier*)tokens[1])->name;
                            optional<Node*> param_def_value = optional<Node*>();

                            EraseFront(&tokens, 2);

                            if (tokens[0]->type == "Operator" && get<0>(GetTokenValue(tokens[0])) == "=")
                            {
                                EraseFront(&tokens, 1);

                                vector<Node*> data;

                                tie(data, tokens) = AnalyseSyntax(tokens, { NULL, true });

                                param_def_value = optional<Node*>(data[0]);
                            }

                            parameters.push_back({ param_type, param_name, param_def_value });
                        }
                        else if (tokens[0]->type == "Control" && get<0>(GetTokenValue(tokens[0])) == ",")
                        {
                            EraseFront(&tokens, 1);
                        }
                        else
                        {
                            Node *node = new Node();
                            node->start = start;
                            node->end = tokens[0]->end;

                            if (tokens[0]->type == "Identifier")
                            {
                                node->error = Error {SyntaxError, "Missing type for parameter"};
                            }
                            else
                            {
                                node->error = Error {SyntaxError, "Invalid character in function definition"};
                            }

                            throw node;
                        }
                    }

                    vector<Node*> content;

                    if (tokens[1]->type == "Bracket" && get<0>(GetTokenValue(tokens[1])) == "{")
                    {
                        EraseFront(&tokens, 2);

                        tie(content, tokens) = AnalyseSyntax(tokens, { new Bracket("}"), false });

                        EraseFront(&tokens, 1);
                    }
                    else if (tokens[1]->type == "Control" && get<0>(GetTokenValue(tokens[1])) == ";")
                    {
                        EraseFront(&tokens, 2);

                        content = vector<Node*>();
                    }
                    else
                    {
                        Node *node = new Node();
                        node->start = start;
                        node->end = tokens[0]->end;

                        node->error = Error { SyntaxError, "Invalid character in function definition" };

                        throw node;
                    }

                    AST.push_back(new AssignVariable(name, new CodeBlock(return_type, parameters, content)));
                }
            }
        }
        else
        {
            
        }
    }

    return {AST, tokens};
}

void EraseFront(vector<Token*> *tokens, int length)
{
    for (int i = 0; i < length; i++)
    {
        tokens->erase(tokens->begin());
    }
}