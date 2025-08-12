#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <regex>
#include <memory>

#include "main.hpp"
#include "../lexer.hpp"
#include "../syntax_analyser.hpp"
#include "../semantics_analyser.hpp"
#include "../interpreter.hpp"
#include "../token.hpp"
#include "../node.hpp"
#include "../object.hpp"
#include "../error.hpp"

using namespace std;

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        cout << "\033[1;31mError:\033[0;31m No input file provided\033[m\n";

        cout << "\033[30mCompilation Terminated.\033[m\n";

        return 1;
    }

    fstream f;
    f.open(argv[1], ios::in);

    vector<int> line_numbers = vector<int>();

    vector<string> lines = vector<string>();

    string line;

    string raw_text;

    while (getline(f, line)) {
        raw_text += line + "\n";

        lines.push_back(line);
        line_numbers.push_back(raw_text.length());
    }

    vector<Token*> tokens = Tokenise(raw_text);

    if (tokens[0]->error)
    {
        return ThrowError(*tokens[0]->error, tokens[0]->start, tokens[0]->end, line_numbers, lines);
    }

    vector<Node*> AST;

    try
    {
        AST = get<0>(AnalyseSyntax(tokens));
    }
    catch (Node *node)
    {
        return ThrowError(*node->error, node->start, node->end, line_numbers, lines);
    }

    try
    {
        AST = AnalyseSemantics(AST, {});
    }
    catch (Node *node)
    {
        return ThrowError(*node->error, node->start, node->end, line_numbers, lines);
    }
    
    shared_ptr<Environment> env = InitialiseInterpreterData();

    shared_ptr<Object> result;

    try
    {
        result = get<0>(Interpret(AST, env, {}));
    }
    catch (ErrorObject error)
    {
        return ThrowError(error.error, error.start, error.end, line_numbers, lines);
    }


    cout << "\nEnvironment:\n" << *env << "\n\n";
    
    if (result)
    {
        cout << "Return Value: " << *result;
    }

    return 0;
}