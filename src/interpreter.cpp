#include <string>
#include <vector>
#include <map>
#include <tuple>
#include <sstream>

#include "interpreter.hpp"
#include "object.hpp"
#include "node.hpp"
#include "error.hpp"

using namespace std;

pair<shared_ptr<Object>, RETURN_REASON> Interpret(vector<Node*> AST, shared_ptr<Environment> env, vector<Call> call_stack)
{
    shared_ptr<Object> return_value = shared_ptr<Object>(new VoidObject());

    for (Node* node : AST)
    {
        if (node->type == "TypeExpression")
        {
            TypeExpression *type = (TypeExpression*)node;

            return_value = shared_ptr<TypeObject>(new TypeObject(InterpretType(*type, env)));
        }
        else if (node->type == "Literal")
        {
            Literal *literal = (Literal*)node;

            if (literal->l_integer)
            {
                return_value = shared_ptr<Object>(new IntObject(literal->l_integer.value()));
            }
            else if (literal->l_float)
            {
                return_value = shared_ptr<Object>(new FloatObject(literal->l_float.value()));
            }
            else if (literal->l_boolean)
            {
                return_value = shared_ptr<Object>(new BoolObject(literal->l_boolean.value()));
            }
            else if (literal->l_string)
            {
                return_value = shared_ptr<Object>(new StringObject(literal->l_string.value()));
            }
        }
        else if (node->type == "CodeBlock")
        {
            return_value = InterpretCodeBlock(*(CodeBlock*)node, env);
        }
        else if (node->type == "Operation")
        {
            Operation *operation = (Operation*)node;

            if (operation->left != NULL)
            {
                shared_ptr<Object> left = get<0>(Interpret({operation->left}, env, call_stack));
                shared_ptr<Object> right = get<0>(Interpret({operation->right}, env, call_stack));

                // Allow implicit casting
                if (left->type != right->type)
                {
                    stringstream s;
                    s << "'" << operation->operator_string << "' operation cannot be applied to type '" << left->type << "' and '" << right->type << "'";

                    throw ErrorObject(operation->start, operation->end, Error {TypeError, s.str()});
                }

                string func_name;

                if (operation->operator_string == "^") func_name = "pow";
                else if (operation->operator_string == "*") func_name = "multiply";
                else if (operation->operator_string == "/") func_name = "divide";
                else if (operation->operator_string == "$") func_name = "int_divide";
                else if (operation->operator_string == "%") func_name = "modulo";
                else if (operation->operator_string == "+") func_name = "add";
                else if (operation->operator_string == "-") func_name = "subtract";
                else if (operation->operator_string == "==") func_name = "equality";
                else if (operation->operator_string == "!=") func_name = "inequality";
                else if (operation->operator_string == ">=") func_name = "greater_equal";
                else if (operation->operator_string == "<=") func_name = "less_equal";
                else if (operation->operator_string == ">") func_name = "greater";
                else if (operation->operator_string == "<") func_name = "less";
                else if (operation->operator_string == "&") func_name = "and";
                else if (operation->operator_string == "|") func_name = "or";
                else if (operation->operator_string == "=") func_name = "assignment";
                else if (operation->operator_string == "+=") func_name = "add_assignment";
                else if (operation->operator_string == "-=") func_name = "subtract_assignment";
                else if (operation->operator_string == "*=") func_name = "multiply_assignment";
                else if (operation->operator_string == "/=") func_name = "divide_assignment";

                optional<Member> type_def = ((TypeDefinitionObject*)left->type.type_definition.get())->GetMember(func_name);

                if (!type_def)
                {
                    stringstream s;
                    s << "No method on type '" << left->type << "' for '" << operation->operator_string << "' with type '" << right->type << "'";

                    throw ErrorObject(operation->start, operation->end, Error {OperationError, s.str() });
                }

                FunctionObject *operation_func = dynamic_cast<FunctionObject*>(type_def->object.get());

                if (!operation_func)
                {
                    stringstream s;
                    s << "No method on type '" << left->type << "' for '" << operation->operator_string << "' with type '" << right->type << "'";

                    throw ErrorObject(operation->start, operation->end, Error {OperationError, s.str() });
                }

                return_value = CallFunction(operation_func, { { "this", Variable(left, Qualifier()) }, { "other", Variable(right, Qualifier()) } }, env, call_stack);
            }
            else
            {
                shared_ptr<Object> right = get<0>(Interpret({operation->right}, env, call_stack));

                string func_name;

                if (operation->operator_string == "!") func_name = "not";
                else if (operation->operator_string == "-") func_name = "negative";
                else if (operation->operator_string == "+") func_name = "positive";

                FunctionObject *operation_func = dynamic_cast<FunctionObject*>(((TypeDefinitionObject*)right->type.type_definition.get())->GetMember(func_name)->object.get());

                if (!operation_func)
                {
                    stringstream s;
                    s << "No method on type '" << right->type << "' for '" << operation->operator_string << "'";

                    throw ErrorObject(operation->start, operation->end, Error {OperationError, s.str() });
                }

                return_value = CallFunction(operation_func, { { "this", Variable(right, Qualifier()) } }, env, call_stack);
            }
        }
        else if (node->type == "GetVariable")
        {
            GetVariable *get_variable = (GetVariable*)node;

            optional<Variable> var = env->Get(get_variable->name);

            if (!var)
            {
                throw ErrorObject(get_variable->start, get_variable->end, Error {IdentifierError, "Identifier '" + get_variable->name + "' is not defined"});
            }

            return_value = var.value().object;
        }
        else if (node->type == "DeclareVariable")
        {
            DeclareVariable *declare_variable = (DeclareVariable*)node;

            if (env->Get(declare_variable->name))
            {
                throw ErrorObject(declare_variable->start, declare_variable->end, Error {IdentifierError, "Identifier '" + declare_variable->name + "' is already declared"});
            }

            Type type = InterpretType(declare_variable->variable_type, env);
            
            shared_ptr<Object> value = get<0>(Interpret({declare_variable->value}, env, call_stack));

            if (value->type != type)
            {
                stringstream s;
                s << "A value of type '" << value->type << "' cannot be used to declare a variable of type '" << type << "'";

                throw ErrorObject(declare_variable->start, declare_variable->end, Error {TypeError, s.str()});
            }

            env->Add(declare_variable->name, Variable(value, Qualifier(declare_variable->qualifier->qualifiers)));

            return_value = shared_ptr<Object>(new VoidObject());
        }
        else if (node->type == "FunctionCall")
        {
            FunctionCall *function_call = (FunctionCall*)node;

            optional<Variable> func = env->Get(function_call->name);

            if (!func)
            {
                throw ErrorObject(function_call->start, function_call->end, Error {IdentifierError, "Identifier '" + function_call->name + "' is not defined"});
            }

            FunctionObject *function = dynamic_cast<FunctionObject*>(func->object.get());

            if (!function)
            {
                throw ErrorObject(function_call->start, function_call->end, Error {IdentifierError, "Identifier '" + function_call->name + "' is not a function"});
            }

            map<string, Variable> argument_values = {};

            // TODO: Deal with array and map arguments (* and **)

            for (int i = 0; i < function->parameters.size(); i++)
            {
                if (i >= function_call->arguments.size())
                {
                    if (!function->parameters[i].default_argument)
                    {
                        stringstream error;
                        error << "Expected (";

                        for (int p = 0; p < function->parameters.size(); p++)
                        {
                            error << function->parameters[p];
                            if (p < function->parameters.size() - 1) error << ", ";
                        }

                        error << ") too few arguments";

                        throw ErrorObject(function_call->start, function_call->end, Error {FunctionError, error.str()});
                    }

                    argument_values.insert({function->parameters[i].name, Variable(get<0>(Interpret({function->parameters[i].default_argument.value()}, env, call_stack)), Qualifier())});
                }
                else
                {
                    shared_ptr<Object> value = get<0>(Interpret({function_call->arguments[i]}, env, call_stack));

                    if (function->parameters[i].type != value->type)
                    {
                        stringstream s;
                        s << "A value of type '" << value->type << "' cannot be passed into parameter of type '" << function->parameters[i].type << "'";

                        throw ErrorObject(function_call->arguments[i]->start, function_call->arguments[i]->end, Error {TypeError, s.str()});
                    }

                    argument_values.insert({function->parameters[i].name, Variable(value, Qualifier())});
                }
            }

            return_value = CallFunction(function, argument_values, env, call_stack);
        }
        // TODO: ClassDefinition, MemberAccess
        else if (node->type == "IfStatement")
        {
            IfStatement *if_statement = (IfStatement*)node;

            vector<Call> if_call_stack = vector<Call>(call_stack);
            if_call_stack.push_back(Call("IfStatement", optional<Type>()));

            shared_ptr<Object> if_expression = get<0>(Interpret({if_statement->if_expression}, env, if_call_stack));

            BoolObject *expression_result = dynamic_cast<BoolObject*>(if_expression.get());

            if (!expression_result)
            {
                stringstream s;
                s << "Expected value of type 'bool' instead of type '" << expression_result->type << "'";

                throw ErrorObject(if_statement->if_expression->start, if_statement->if_expression->end, Error {TypeError, s.str()});
            }

            if (expression_result->value)
            {
                RETURN_REASON return_reason = get<1>(Interpret(if_statement->if_code_block->content, env, if_call_stack));

                if (return_reason != EndOfAST)
                {
                    return {shared_ptr<Object>(new VoidObject()), return_reason};
                }

                continue;
            }

            bool completed_else_if_expression = false;

            for (int i = 0; i < if_statement->else_if_expressions.size(); i++)
            {
                shared_ptr<Object> else_if_expression = get<0>(Interpret({if_statement->else_if_expressions[i]}, env, if_call_stack));

                BoolObject *expression_result = dynamic_cast<BoolObject*>(else_if_expression.get());

                if (!expression_result)
                {
                    stringstream s;
                    s << "Expected value of type 'bool' instead of type '" << expression_result->type << "'";

                    throw ErrorObject(if_statement->else_if_expressions[i]->start, if_statement->else_if_expressions[i]->end, Error {TypeError, s.str()});
                }

                if (expression_result->value)
                {
                    RETURN_REASON return_reason = get<1>(Interpret(if_statement->else_if_code_blocks[i]->content, env, if_call_stack));

                    if (return_reason != EndOfAST)
                    {
                        return {shared_ptr<Object>(new VoidObject()), return_reason};
                    }

                    completed_else_if_expression = true;

                    break;
                }
            }

            if (if_statement->else_code_block && !completed_else_if_expression)
            {
                RETURN_REASON return_reason = get<1>(Interpret(if_statement->else_code_block->content, env, if_call_stack));

                if (return_reason != EndOfAST)
                {
                    return {shared_ptr<Object>(new VoidObject()), return_reason};
                }
            }

            return_value = shared_ptr<Object>(new VoidObject());
        }
        else if (node->type == "SwitchStatement")
        {
            SwitchStatement *switch_statement = (SwitchStatement*)node;

            bool completed_case_expression = false;

            vector<Call> switch_call_stack = vector<Call>(call_stack);
            switch_call_stack.push_back(Call("SwitchStatement", optional<Type>()));

            shared_ptr<Object> switch_expression = get<0>(Interpret({switch_statement->switch_expression}, env, switch_call_stack));

            for (int i = 0; i < switch_statement->case_expressions.size(); i++)
            {
                shared_ptr<Object> case_expression = get<0>(Interpret({switch_statement->case_expressions[i]}, env, switch_call_stack));

                if (switch_expression->type != case_expression->type)
                {
                    stringstream s;
                    s << "type '" << switch_expression->type << "' is not the same as '" << case_expression->type << "'";

                    throw ErrorObject(switch_statement->case_expressions[i]->start, switch_statement->case_expressions[i]->end, Error {TypeError, s.str()});
                }

                optional<Member> type_def = ((TypeDefinitionObject*)switch_expression->type.type_definition.get())->GetMember("equality");

                if (!type_def)
                {
                    stringstream s;
                    s << "No method on type '" << switch_expression->type << "' for '==' with type '" << case_expression->type << "'";

                    throw ErrorObject(switch_statement->case_expressions[i]->start, switch_statement->case_expressions[i]->end, Error {OperationError, s.str() });
                }

                FunctionObject *operation_func = dynamic_cast<FunctionObject*>(type_def->object.get());

                if (!operation_func)
                {
                    stringstream s;
                    s << "No method on type '" << switch_expression->type << "' for '==' with type '" << case_expression->type << "'";

                    throw ErrorObject(switch_statement->case_expressions[i]->start, switch_statement->case_expressions[i]->end, Error {OperationError, s.str() });
                }

                BoolObject *is_case_correct = dynamic_cast<BoolObject*>(CallFunction(operation_func, { { "this", Variable(switch_expression, Qualifier()) }, { "other", Variable(case_expression, Qualifier()) } }, env, call_stack).get());

                if (!is_case_correct)
                {
                    stringstream s;
                    s << "Comparison of type '" << switch_expression->type << "' with type '" << case_expression->type << "' does not return a boolean";

                    throw ErrorObject(switch_statement->case_expressions[i]->start, switch_statement->case_expressions[i]->end, Error {OperationError, s.str() });
                }

                if (is_case_correct->value)
                {
                    RETURN_REASON return_reason = get<1>(Interpret(switch_statement->case_code_blocks[i]->content, env, switch_call_stack));

                    if (return_reason != EndOfAST)
                    {
                        return {shared_ptr<Object>(new VoidObject()), return_reason};
                    }

                    completed_case_expression = true;

                    break;
                }
            }

            if (switch_statement->default_code_block && !completed_case_expression)
            {
                RETURN_REASON return_reason = get<1>(Interpret(switch_statement->default_code_block->content, env, switch_call_stack));

                if (return_reason != EndOfAST)
                {
                    return {shared_ptr<Object>(new VoidObject()), return_reason};
                }
            }
        }
        else if (node->type == "ForLoop")
        {
            ForLoop *for_loop = (ForLoop*)node;

            shared_ptr<Environment> for_env = shared_ptr<Environment>(new Environment(env, {}));

            vector<Call> for_call_stack = vector<Call>(call_stack);
            for_call_stack.push_back(Call("ForLoop", optional<Type>()));

            Interpret({for_loop->declaration_expression}, for_env, for_call_stack);

            bool continue_looping = true;

            while (continue_looping)
            {
                shared_ptr<Object> value = get<0>(Interpret({for_loop->condition_expression}, for_env, for_call_stack));
                
                BoolObject *expression_result = dynamic_cast<BoolObject*>(value.get());

                if (!expression_result)
                {
                    stringstream s;
                    s << "Expected value of type 'bool' instead of type '" << expression_result->type << "'";

                    throw ErrorObject(for_loop->condition_expression->start, for_loop->condition_expression->end, Error {TypeError, s.str()});
                }

                if (!expression_result->value)
                {
                    continue_looping = false;
                    break;
                }

                shared_ptr<Environment> this_env = shared_ptr<Environment>(new Environment(for_env, {}));

                RETURN_REASON return_reason = get<1>(Interpret(for_loop->for_code_block->content, this_env, for_call_stack));

                if (return_reason == ReturnStatement)
                {
                    return {shared_ptr<Object>(new VoidObject()), return_reason};
                }
                else if (return_reason == BreakStatement)
                {
                    continue;
                }

                Interpret({for_loop->iteration_expression}, for_env, for_call_stack);
            }
        }
        else if (node->type == "ForEachLoop")
        {
            // TODO: Only possible when iterative types exist
        }
        else if (node->type == "WhileLoop")
        {
            WhileLoop *while_loop = (WhileLoop*)node;

            vector<Call> while_call_stack = vector<Call>(call_stack);
            while_call_stack.push_back(Call("WhileLoop", optional<Type>()));

            bool continue_looping = true;

            while (continue_looping)
            {
                shared_ptr<Object> value = get<0>(Interpret({while_loop->condition}, env, while_call_stack));
                
                BoolObject *expression_result = dynamic_cast<BoolObject*>(value.get());

                if (!expression_result)
                {
                    stringstream s;
                    s << "Expected value of type 'bool' instead of type '" << expression_result->type << "'";

                    throw ErrorObject(while_loop->condition->start, while_loop->condition->end, Error {TypeError, s.str()});
                }

                if (!expression_result->value)
                {
                    continue_looping = false;
                    break;
                }

                shared_ptr<Environment> while_env = shared_ptr<Environment>(new Environment(env, {}));

                RETURN_REASON return_reason = get<1>(Interpret(while_loop->while_code_block->content, while_env, while_call_stack));

                if (return_reason == ReturnStatement)
                {
                    return {shared_ptr<Object>(new VoidObject()), return_reason};
                }
                else if (return_reason == BreakStatement)
                {
                    continue;
                }
            }
        }
        else if (node->type == "Return")
        {
            Return *return_node = (Return*)node;

            shared_ptr<Object> value = get<0>(Interpret({return_node->expression}, env, call_stack));

            return {value, ReturnStatement};
        }
        else if (node->type == "Break")
        {
            return {shared_ptr<Object>(new VoidObject()), BreakStatement};
        }
        else if (node->type == "Continue")
        {
            return {shared_ptr<Object>(new VoidObject()), ContinueStatement};
        }
    }

    return {return_value, EndOfAST};
}

Type InterpretType(TypeExpression type_expression, shared_ptr<Environment> env)
{
    optional<Variable> var = env->Get(type_expression.name);

    if (var && dynamic_cast<TypeDefinitionObject*>(var->object.get()))
    {
        Type type = Type(var->object);

        // TODO: More complex types (with generics)

        return type;
    }
    else
    {
        throw ErrorObject(type_expression.start, type_expression.end, Error {TypeError, "The type '" + type_expression.name + "' is undefined"});
    }
}

shared_ptr<FunctionObject> InterpretCodeBlock(CodeBlock code_block, shared_ptr<Environment> env)
{
    Type return_type = InterpretType(code_block.return_type, env);

    vector<Parameter> parameters = {};

    for (ParameterExpression p : code_block.parameters)
    {
        Type p_type = InterpretType(p.type_data, env);

        parameters.push_back(Parameter(p_type, p.name, p.default_argument, p.argument_expansion));
    }

    vector<Node*> body = code_block.content;

    return shared_ptr<FunctionObject>(new FunctionObject(return_type, parameters, body));
}

shared_ptr<Object> CallFunction(FunctionObject *function, map<string, Variable> argument_values, shared_ptr<Environment> env, vector<Call> call_stack)
{
    shared_ptr<Environment> func_env = shared_ptr<Environment>(new Environment(env, argument_values));

    if (function->body.index() == 0)
    {   
        vector<Call> func_call_stack = vector<Call>(call_stack);
        func_call_stack.push_back(Call("FunctionCall", function->return_type));
        
        return get<0>(Interpret(get<0>(function->body), func_env, func_call_stack));
    }
    else
    {
        return get<1>(function->body)(func_env);
    }
}