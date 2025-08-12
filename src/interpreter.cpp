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