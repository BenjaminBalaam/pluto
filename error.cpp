#include <iostream>
#include <string>
#include <vector>

#include "error.hpp"

using namespace std;

ostream& operator<<(ostream& os, const ERROR_TYPE& e)
{
    switch (e)
    {
        case SyntaxError:
            return os << "SyntaxError";
    }

    return os;
}

int ThrowError(Error error, int start, int end, vector<int> line_numbers, vector<string> lines)
{
    pair<pair<int, int>, pair<int, int>> positions = GetPositions(line_numbers, start, end);

    cout << "\033[1;31m" << error.type << ": " << error.text << "\033[m\n";

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
            << "\033[4;31m" << lines[positions.first.first - 1].substr(positions.first.second, positions.second.second - positions.first.second)
            << "\n";

        for (int i = positions.first.first; i < positions.second.first - 1; i++)
        {
            cout << "\033[0;31m" << i << ": \033[4;31m" << lines[i] << "\n";
        }

        cout << "\033[0;31m" << positions.second.first << ": \033[4;31m"
                << lines[positions.second.first - 1].substr(0, positions.second.second)
                << "\033[0;31m" << lines[positions.second.first - 1].substr(positions.second.second, lines[positions.second.first - 1].size() - 1 - positions.second.second)
                << "\n";
    }

    for (int i = positions.first.first; i < positions.second.first - 1; i++)
    {
        cout << lines[i] << "\n";
    }

    if (positions.second.first != lines.size()) {
        cout << positions.second.first + 1 << ": " << lines[positions.second.first] << "\n";
    }

    cout << "\n";

    cout << "\033[30mCompilation Terminated.\033[m\n";

    return 1;
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