#ifndef syntax_analyser_hpp
#define syntax_analyser_hpp

#include <string>
#include <vector>

#include "token.hpp"
#include "node.hpp"

std::pair<std::vector<Node*>, std::vector<Token*>> AnalyseSyntax(std::vector<Token*> tokens, std::pair<std::vector<std::pair<Token*, bool>>, bool> return_flags = { {}, false });

Node* GetASTEnd(std::vector<Node*> AST);

bool ShouldReturn(Token *current_token, std::vector<std::pair<Token*, bool>> return_tokens);

bool StatementStarted(std::vector<Node*> AST);

std::vector<std::pair<Token*, bool>> GetReturnTokens(std::vector<Token*> new_return_tokens, std::vector<std::pair<Token*, bool>> existing_return_tokens);

std::pair<Operation*, std::vector<Token*>> ProcessOperations(std::pair<std::vector<std::pair<Token*, bool>>, bool> return_flags, int start, std::vector<Token*> tokens, Node *left);

bool CompareOperator(std::string operator1, std::string operator2);

void AddStartAndEnd(Operation *operation);

bool InMultiExpression(Node *node);

bool EraseFront(std::vector<Token*> *tokens, int length);

#endif