#include <string>
#include <vector>
#include <map>
#include <tuple>
#include <sstream>

#include "interpreter.hpp"
#include "node.hpp"
#include "object.hpp"
#include "error.hpp"

using namespace std;

shared_ptr<Object> Interpret(vector<Node*> AST, shared_ptr<Environment> env, vector<Node*> call_stack)
{
    shared_ptr<Object> return_value;

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
        // TODO: CodeBlock, Operation
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
            shared_ptr<Object> value = Interpret({declare_variable->value}, env, call_stack);

            env->Add(declare_variable->name, Variable(value, Qualifier(declare_variable->qualifier->qualifiers)));

            if (value->type != type)
            {
                stringstream s;
                s << "A value of type '" << value->type << "' cannot be used to declare a variable of type '" << type << "'";

                throw ErrorObject(declare_variable->start, declare_variable->end, Error {TypeError, s.str()});
            }
        }
    }

    return return_value;
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