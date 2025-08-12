#ifndef semantics_analyser_hpp
#define semantics_analyser_hpp

#include <string>
#include <vector>

#include "node.hpp"

std::vector<Node*> AnalyseSemantics(std::vector<Node*> AST, std::vector<Node*> call_stack = {});

void CheckStatement(Node *statement, std::vector<Node*> call_stack);

void CheckExpression(Node *expression, std::vector<Node*> call_stack);

bool InCallStack(std::vector<Node*> call_stack, std::string type);

int CallStackPosition(std::vector<Node*> call_stack, std::string type);

#endif