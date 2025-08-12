#include <string>

#include "token.hpp"

using namespace std;

Integer::Integer(int number) : number(number) {}

Float::Float(float number) : number(number) {}

String::String(string content) : content(content) {}

Identifier::Identifier(string name) : name(name) {}

Keyword::Keyword(string name) : name(name) {}