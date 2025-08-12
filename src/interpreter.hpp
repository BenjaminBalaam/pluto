#ifndef interpreter_hpp
#define interpreter_hpp

#include <string>
#include <vector>
#include <memory>

#include "object.hpp"
#include "node.hpp"

std::pair<std::shared_ptr<Object>, RETURN_REASON> Interpret(std::vector<Node*> AST, std::shared_ptr<Environment> env, std::vector<Call> call_stack);

Type InterpretType(TypeExpression type_expression, std::shared_ptr<Environment> env);

std::shared_ptr<FunctionObject> InterpretCodeBlock(CodeBlock code_block, std::shared_ptr<Environment> env);

std::shared_ptr<Object> CallFunction(FunctionObject *function, std::map<std::string, Variable> argument_values, std::shared_ptr<Environment> env, std::vector<Call> call_stack);

#endif