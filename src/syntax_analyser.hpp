#ifndef syntax_analyser_hpp
#define syntax_analyser_hpp

#include <string>
#include <vector>

#include "token.hpp"
#include "node.hpp"

std::pair<std::vector<Node*>, std::vector<Token*>> AnalyseSyntax(std::vector<Token*> tokens, std::pair<std::vector<Token*>, bool> return_flags = { {}, false });

Node* GetASTEnd(std::vector<Node*> AST);

bool ShouldReturn(Token* current_token, std::vector<Token*> return_tokens);

bool EraseFront(std::vector<Token*> *tokens, int length);

void ThrowError(int start, int end, Error error);

#endif