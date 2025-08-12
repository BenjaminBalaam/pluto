#ifndef syntax_analyser_hpp
#define syntax_analyser_hpp

#include <string>
#include <vector>

#include "token.hpp"
#include "node.hpp"

std::pair<std::vector<Node*>, std::vector<Token*>> AnalyseSyntax(std::vector<Token*> tokens, std::pair<Token*, bool> return_flags = std::pair<Token*, bool>(NULL, false));

void EraseFront(std::vector<Token*> *tokens, int length);

#endif