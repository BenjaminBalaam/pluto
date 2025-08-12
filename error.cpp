#include <iostream>
#include <string>

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

int TerminateCompilation()
{
    cout << "\033[30mCompilation Terminated.\033[m\n";

    return 1;
}