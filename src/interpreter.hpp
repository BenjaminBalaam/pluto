#ifndef interpreter_hpp
#define interpreter_hpp

#include <string>
#include <vector>
#include <memory>

#include "node.hpp"
#include "object.hpp"

std::shared_ptr<Object> Interpret(std::vector<Node*> AST, std::shared_ptr<Environment> env, std::vector<Node*> call_stack);

Type InterpretType(TypeExpression type_expression, std::shared_ptr<Environment> env);

#endif