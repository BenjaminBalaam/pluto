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

        return TerminateCompilation();
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
            pair<pair<int, int>, pair<int, int>> positions = GetPositions(line_numbers, token->start, token->end);

            cout << "\033[1;31m" << token->error->type << ": " << token->error->text << "\033[m\n";

            cout << "\033[0;31mline [" << positions.first.first << ", " << positions.first.second << "]:\n\n";

            if (positions.first.first != 1)
            {
                cout << positions.first.first - 1 << ": " << lines[positions.first.first - 2] << "\n";
            }

            if (positions.first.first == positions.second.first)
            {
                cout << positions.first.first << ": "
                    << lines[positions.first.first - 1].substr(0, positions.first.second)
                    << "\033[4;31m" << lines[positions.first.first - 1].substr(positions.first.second, positions.second.second - positions.first.second)
                    << "\033[0;31m" << lines[positions.second.first - 1].substr(positions.second.second, lines[positions.second.first - 1].size() - 1 - positions.second.second) << "\n";
            }
            else
            {
                cout << positions.first.first << ": "
                    << lines[positions.first.first - 1].substr(0, positions.first.second)
                    << "\033[4;31m" << lines[positions.first.first - 1].substr(positions.first.second, positions.second.second - positions.first.second);

                for (int i = positions.first.first; i < positions.second.first - 1; i++)
                {
                    cout << "\033[0;31m" << i << ": \033[4;31m" << lines[i] << "\n";
                }

                cout << "\033[0;31m" << positions.second.first << ": \033[4;31m"
                     << lines[positions.second.first - 1].substr(0, positions.second.second)
                     << "\033[0;31m" << lines[positions.second.first - 1].substr(positions.second.second, lines[positions.second.first - 1].size() - 1 - positions.second.second) << "\n";
            }

            for (int i = positions.first.first; i < positions.second.first - 1; i++)
            {
                cout << lines[i] << "\n";
            }

            if (positions.second.first != lines.size()) {
                cout << positions.second.first + 1 << ": " << lines[positions.second.first] << "\n";
            }

            cout << "\n";

            return TerminateCompilation();
        }

        delete token;
    }

    return 0;
}

pair<pair<int, int>, pair<int, int>> GetPositions(vector<int> lines, int start, int end)
{
    lines.insert(lines.begin(), 0);

    int start_line = 0;
    int end_line = 0;

    for (int i = 1; i < lines.size(); i++)
    {
        if (lines[i] > start && start_line == 0)
        {
            start_line = i;
            start -= lines[i - 1];
        }

        if (lines[i] > end && end_line == 0)
        {
            end_line = i;
            end -= lines[i - 1];
        }
    }

    return pair<pair<int, int>, pair<int, int>> { pair<int, int> { start_line, start }, pair<int, int> { end_line, end } };
}