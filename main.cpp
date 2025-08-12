#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <regex>

#include "main.hpp"

using namespace std;

int main(int argc, char *argv[])
{
    fstream f;
    f.open(argv[1], ios::in);

    string raw_text;

    while (getline(f, raw_text)) {
        raw_text += "\n";
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
            cout << "\033[1;31m" << token->error->type << ": " << token->error->text << "\033[m\n";

            return 0;
        }

        delete token;
    }

    return 0;
}