#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <regex>

#include "main.hpp"
#include "lexer.hpp"
#include "token.hpp"
#include "error.hpp"

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

    for (Token* token : tokens)
    {
        if (!token->error)
        {
            cout << *token << "\n";
        }
        else
        {
            ThrowError(*token->error, token->start, token->end, line_numbers, lines);
        }

        delete token;
    }

    return 0;
}