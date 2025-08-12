#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <sstream>
#include <tuple>
#include <optional>
#include <memory>
#include <cmath>

#include "object.hpp"
#include "node.hpp"
#include "error.hpp"

using namespace std;

Type::Type(shared_ptr<Object> type_definition) : type_definition(type_definition)
{

}

bool Type::operator==(Type other)
{
    return this->type_definition == other.type_definition;
}

bool Type::operator!=(Type other)
{
    return !(this->operator==(other));
}

ostream &operator<<(ostream &os, const Type &type)
{
    return os << *type.type_definition;
}

Qualifier::Qualifier(bool is_public, bool is_static, bool is_const) : is_public(is_public), is_static(is_static), is_const(is_const)
{

}

Qualifier::Qualifier(vector<string> qualifiers)
{
    is_public, is_static, is_const = false;

    for (std::string q : qualifiers)
    {
        if (q == "public")
        {
            is_public = true;
        }
        else if (q == "static")
        {
            is_static = true;
        }
        else if (q == "const")
        {
            is_const = true;
        }
    }
}

ostream &operator<<(ostream &os, const Qualifier &qualifier)
{
    string s = "";

    if (qualifier.is_public) s + "public ";
    if (qualifier.is_static) s + "static ";
    if (qualifier.is_const) s + "const ";

    if (s != "")
    {
        s.pop_back();
    }

    return os << s;
}

Object::Object() : type(Type(NULL))
{
    
}

Object::~Object()
{

}

ostream &operator<<(ostream &os, Object &object)
{
    return os << object.to_string();
}

Member::Member(shared_ptr<Object> object, Qualifier qualifiers) : qualifiers(qualifiers), object(object)
{
    
}

ostream &operator<<(ostream &os, const Member &member)
{
    return os << member.qualifiers << " " << *member.object;
}

Parameter::Parameter(Type type, string name, optional<Node*> default_argument, ARGUMENT_EXPANSION argument_expansion) : type(type), name(name), default_argument(default_argument), argument_expansion(argument_expansion)
{

}

ostream &operator<<(ostream &os, const Parameter &data)
{
    if (data.argument_expansion == None)
    {
        os << data.type << " " << data.name;
    }
    else if (data.argument_expansion == Array)
    {
        os << data.type << " *" << data.name;
    }
    else if (data.argument_expansion == Dictionary)
    {
        os << data.type << " **" << data.name;
    }

    if (data.default_argument)
    {
        os << " = " << *data.default_argument.value();
    }

    return os;
}

Variable::Variable(shared_ptr<Object> object, Qualifier qualifiers) : qualifiers(qualifiers), object(object)
{
    
}

ostream &operator<<(ostream &os, const Variable &variable)
{
    return os << variable.qualifiers << " " << *variable.object;
}

Environment::Environment(shared_ptr<Environment> parent_environment, map<string, Variable> variables) : parent_environment(parent_environment), variables(variables)
{

}

optional<Variable> Environment::Get(string name)
{
    auto var = variables.find(name);

    if (var != variables.end())
    {
        return optional<Variable>(var->second);
    }

    if (parent_environment)
    {
        return parent_environment->Get(name);
    }

    return optional<Variable>();
}

void Environment::Add(string name, Variable variable)
{
    variables.insert({ name, variable });
}

ostream &operator<<(ostream &os, const Environment &environment)
{
    os << "{";

    for (auto pair : environment.variables)
    {
        os << pair.first << ": " << pair.second << ", ";
    }

    os << "}";

    if (environment.parent_environment)
    {
        os << "\nParent: " << *environment.parent_environment;
    }

    return os;
}

Call::Call(std::string node_type, std::optional<Type> return_type) : node_type(node_type), return_type(return_type)
{

}

ostream &operator<<(ostream &os, const Call &call)
{
    if (call.return_type)
    {
        return os << "Call: " << call.node_type << "(" << call.return_type.value() << ")";
    }
    else
    {
        return os << "Call: " << call.node_type << "()";
    }
}

IntObject::IntObject(int value) : value(value)
{
    this->type = Type(Types->Get("int")->object);
}

string IntObject::to_string()
{
    return std::to_string(value);
}

IntObject::~IntObject()
{

}

FloatObject::FloatObject(float value) : value(value)
{
    this->type = Type(Types->Get("float")->object);
}

string FloatObject::to_string()
{
    return std::to_string(value);
}

FloatObject::~FloatObject()
{

}

BoolObject::BoolObject(bool value) : value(value)
{
    this->type = Type(Types->Get("bool")->object);
}

string BoolObject::to_string()
{
    return value ? "true" : "false";
}

BoolObject::~BoolObject()
{

}

StringObject::StringObject(string value) : value(value)
{
    this->type = Type(Types->Get("string")->object);
}

string StringObject::to_string()
{
    return "\"" + value + "\"";
}

StringObject::~StringObject()
{

}

TypeObject::TypeObject(Type value) : value(value)
{
    this->type = Type(Types->Get("Type")->object);
}

string TypeObject::to_string()
{
    stringstream s;
    s << value;

    return s.str();
}

TypeObject::~TypeObject()
{

}

TypeDefinitionObject::TypeDefinitionObject(string name, map<string, Member> members) : name(name), members(members)
{
    
}

optional<Member> TypeDefinitionObject::GetMember(string name)
{
    auto member = members.find(name);

    if (member != members.end())
    {
        return optional<Member>(member->second);
    }

    return optional<Member>();
}

string TypeDefinitionObject::to_string()
{
    return name + "<>";
}

TypeDefinitionObject::~TypeDefinitionObject()
{

}

ClassInstanceObject::ClassInstanceObject(Type class_type) : class_type(class_type)
{
    this->type = Type(class_type);
}

string ClassInstanceObject::to_string()
{
    stringstream s;
    s << class_type << " {}";

    return s.str();
}

ClassInstanceObject::~ClassInstanceObject()
{

}

FunctionObject::FunctionObject(Type return_type, vector<Parameter> parameters, variant<vector<Node*>, function<shared_ptr<Object>(shared_ptr<Environment>)>> body) : return_type(return_type), parameters(parameters), body(body)
{
    this->type = Type(Types->Get("Function")->object);
}

string FunctionObject::to_string()
{
    stringstream s;
    s << return_type << " function(";

    for (int i = 0; i < parameters.size(); i++)
    {
        s << parameters[i];
        if (i < parameters.size() - 1) s << ", ";
    }

    s << ") {";

    if (body.index() == 0)
    {
        for (Node *node : get<0>(body))
        {
            s << *node << "\n";
        }
    }
    else
    {
        s << "C++ Function";
    }

    s << "}";

    return s.str();
}

FunctionObject::~FunctionObject()
{

}

VoidObject::VoidObject()
{

}

string VoidObject::to_string()
{
    return "void";
}

VoidObject::~VoidObject()
{

}

ErrorObject::ErrorObject(int start, int end, Error error) : start(start), end(end), error(error)
{

}

string ErrorObject::to_string()
{
    stringstream s;
    s << "[" << start << ", " << end << "] " << error.type << ": " << error.text;

    return s.str();
}

ErrorObject::~ErrorObject()
{

}

void InitialiseInterpreterData()
{
    Types->variables = {
        { "void", Variable(shared_ptr<Object>(new TypeDefinitionObject("void", {})), Qualifier()) },
        { "int", Variable(shared_ptr<Object>(new TypeDefinitionObject("int", {})), Qualifier()) },
        { "float", Variable(shared_ptr<Object>(new TypeDefinitionObject("float", {})), Qualifier()) },
        { "bool", Variable(shared_ptr<Object>(new TypeDefinitionObject("bool", {})), Qualifier()) },
        { "string", Variable(shared_ptr<Object>(new TypeDefinitionObject("string", {})), Qualifier()) },
        { "Type", Variable(shared_ptr<Object>(new TypeDefinitionObject("Type", {})), Qualifier()) },
        { "Function", Variable(shared_ptr<Object>(new TypeDefinitionObject("Function", {})), Qualifier()) },
    };

    CreateIntMethods();
    CreateFloatMethods();
    CreateBoolMethods();
    CreateStringMethods();
    CreateTypeMethods();
    CreateFunctionMethods();
}

shared_ptr<Environment> Types = std::shared_ptr<Environment>(new Environment(NULL, std::map<std::string, Variable> {}));

Member CreateMethod(shared_ptr<Object> definition, vector<Parameter> parameters, function<shared_ptr<Object>(shared_ptr<Environment>)> function)
{
    return Member(shared_ptr<Object>(new FunctionObject(Type(definition), parameters, function)), Qualifier());
}

void CreateIntMethods()
{
    shared_ptr<Object> intDefinition = Types->Get("int")->object;
    TypeDefinitionObject *rawIntDefinition = (TypeDefinitionObject*)intDefinition.get();

    vector<Parameter> parameters = {Parameter(Type(intDefinition), "other", optional<Node*>(), None)};

    auto powFunction = [] (shared_ptr<Environment> env) {
        return shared_ptr<Object>( new IntObject(pow(((IntObject*)env->Get("this")->object.get())->value, ((IntObject*)env->Get("other")->object.get())->value)));
    };
    rawIntDefinition->members.insert({ "pow", CreateMethod(intDefinition, parameters, powFunction) });

    auto negativeFunction = [] (shared_ptr<Environment> env) {
        return shared_ptr<Object>( new IntObject(-((IntObject*)env->Get("this")->object.get())->value));
    };
    rawIntDefinition->members.insert({ "negative", CreateMethod(intDefinition, parameters, negativeFunction) });

    auto positiveFunction = [] (shared_ptr<Environment> env) {
        return shared_ptr<Object>( new IntObject(+((IntObject*)env->Get("this")->object.get())->value));
    };
    rawIntDefinition->members.insert({ "positive", CreateMethod(intDefinition, parameters, positiveFunction) });

    auto multiplyFunction = [] (shared_ptr<Environment> env) {
        return shared_ptr<Object>( new IntObject(((IntObject*)env->Get("this")->object.get())->value * ((IntObject*)env->Get("other")->object.get())->value));
    };
    rawIntDefinition->members.insert({ "multiply", CreateMethod(intDefinition, parameters, multiplyFunction) });

    auto divideFunction = [] (shared_ptr<Environment> env) {
        return shared_ptr<Object>( new FloatObject((double)(((IntObject*)env->Get("this")->object.get())->value) / (double)(((IntObject*)env->Get("other")->object.get())->value)));
    };
    rawIntDefinition->members.insert({ "divide", CreateMethod(intDefinition, parameters, divideFunction) });

    auto intDivideFunction = [] (shared_ptr<Environment> env) {
        return shared_ptr<Object>( new IntObject(((IntObject*)env->Get("this")->object.get())->value / ((IntObject*)env->Get("other")->object.get())->value));
    };
    rawIntDefinition->members.insert({ "int_divide", CreateMethod(intDefinition, parameters, intDivideFunction) });

    auto moduloFunction = [] (shared_ptr<Environment> env) {
        return shared_ptr<Object>( new IntObject(((IntObject*)env->Get("this")->object.get())->value % ((IntObject*)env->Get("other")->object.get())->value));
    };
    rawIntDefinition->members.insert({ "modulo", CreateMethod(intDefinition, parameters, moduloFunction) });

    auto addFunction = [] (shared_ptr<Environment> env) {
        return shared_ptr<Object>( new IntObject(((IntObject*)env->Get("this")->object.get())->value + ((IntObject*)env->Get("other")->object.get())->value));
    };
    rawIntDefinition->members.insert({ "add", CreateMethod(intDefinition, parameters, addFunction) });

    auto subtractFunction = [] (shared_ptr<Environment> env) {
        return shared_ptr<Object>( new IntObject(((IntObject*)env->Get("this")->object.get())->value - ((IntObject*)env->Get("other")->object.get())->value));
    };
    rawIntDefinition->members.insert({ "subtract", CreateMethod(intDefinition, parameters, subtractFunction) });

    auto equalityFunction = [] (shared_ptr<Environment> env) {
        return shared_ptr<Object>( new BoolObject(((IntObject*)env->Get("this")->object.get())->value == ((IntObject*)env->Get("other")->object.get())->value));
    };
    rawIntDefinition->members.insert({ "equality", CreateMethod(intDefinition, parameters, equalityFunction) });

    auto inequalityFunction = [] (shared_ptr<Environment> env) {
        return shared_ptr<Object>( new BoolObject(((IntObject*)env->Get("this")->object.get())->value != ((IntObject*)env->Get("other")->object.get())->value));
    };
    rawIntDefinition->members.insert({ "inequality", CreateMethod(intDefinition, parameters, inequalityFunction) });

    auto greaterEqualFunction = [] (shared_ptr<Environment> env) {
        return shared_ptr<Object>( new BoolObject(((IntObject*)env->Get("this")->object.get())->value >= ((IntObject*)env->Get("other")->object.get())->value));
    };
    rawIntDefinition->members.insert({ "greater_equal", CreateMethod(intDefinition, parameters, greaterEqualFunction) });

    auto lessEqualFunction = [] (shared_ptr<Environment> env) {
        return shared_ptr<Object>( new BoolObject(((IntObject*)env->Get("this")->object.get())->value <= ((IntObject*)env->Get("other")->object.get())->value));
    };
    rawIntDefinition->members.insert({ "less_equal", CreateMethod(intDefinition, parameters, lessEqualFunction) });

    auto greaterFunction = [] (shared_ptr<Environment> env) {
        return shared_ptr<Object>( new BoolObject(((IntObject*)env->Get("this")->object.get())->value > ((IntObject*)env->Get("other")->object.get())->value));
    };
    rawIntDefinition->members.insert({ "greater", CreateMethod(intDefinition, parameters, greaterFunction) });

    auto lessFunction = [] (shared_ptr<Environment> env) {
        return shared_ptr<Object>( new BoolObject(((IntObject*)env->Get("this")->object.get())->value < ((IntObject*)env->Get("other")->object.get())->value));
    };
    rawIntDefinition->members.insert({ "less", CreateMethod(intDefinition, parameters, lessFunction) });

    auto assignmentFunction = [] (shared_ptr<Environment> env) {
        ((IntObject*)env->Get("this")->object.get())->value = ((IntObject*)env->Get("other")->object.get())->value;
        return shared_ptr<Object>(new VoidObject());
    };
    rawIntDefinition->members.insert({ "assignment", CreateMethod(intDefinition, parameters, assignmentFunction) });

    auto addAssignmentFunction = [] (shared_ptr<Environment> env) {
        ((IntObject*)env->Get("this")->object.get())->value += ((IntObject*)env->Get("other")->object.get())->value;
        return shared_ptr<Object>(new VoidObject());
    };
    rawIntDefinition->members.insert({ "add_assignment", CreateMethod(intDefinition, parameters, addAssignmentFunction) });

    auto subtractAssignmentFunction = [] (shared_ptr<Environment> env) {
        ((IntObject*)env->Get("this")->object.get())->value -= ((IntObject*)env->Get("other")->object.get())->value;
        return shared_ptr<Object>(new VoidObject());
    };
    rawIntDefinition->members.insert({ "subtract_assignment", CreateMethod(intDefinition, parameters, subtractAssignmentFunction) });

    auto multiplyAssignmentFunction = [] (shared_ptr<Environment> env) {
        ((IntObject*)env->Get("this")->object.get())->value *= ((IntObject*)env->Get("other")->object.get())->value;
        return shared_ptr<Object>(new VoidObject());
    };
    rawIntDefinition->members.insert({ "multiply_assignment", CreateMethod(intDefinition, parameters, multiplyAssignmentFunction) });

    auto divideAssignmentFunction = [] (shared_ptr<Environment> env) {
        ((IntObject*)env->Get("this")->object.get())->value /= ((IntObject*)env->Get("other")->object.get())->value;
        return shared_ptr<Object>(new VoidObject());
    };
    rawIntDefinition->members.insert({ "divide_assignment", CreateMethod(intDefinition, parameters, divideAssignmentFunction) });
}

void CreateFloatMethods()
{
    shared_ptr<Object> floatDefinition = Types->Get("float")->object;
    TypeDefinitionObject *rawFloatDefinition = (TypeDefinitionObject*)floatDefinition.get();

    vector<Parameter> parameters = {Parameter(Type(floatDefinition), "other", optional<Node*>(), None)};

    auto powFunction = [] (shared_ptr<Environment> env) {
        return shared_ptr<Object>( new FloatObject(pow(((FloatObject*)env->Get("this")->object.get())->value, ((FloatObject*)env->Get("other")->object.get())->value)));
    };
    rawFloatDefinition->members.insert({ "pow", CreateMethod(floatDefinition, parameters, powFunction) });

    auto negativeFunction = [] (shared_ptr<Environment> env) {
        return shared_ptr<Object>( new FloatObject(-((FloatObject*)env->Get("this")->object.get())->value));
    };
    rawFloatDefinition->members.insert({ "negative", CreateMethod(floatDefinition, parameters, negativeFunction) });

    auto positiveFunction = [] (shared_ptr<Environment> env) {
        return shared_ptr<Object>( new FloatObject(+((FloatObject*)env->Get("this")->object.get())->value));
    };
    rawFloatDefinition->members.insert({ "positive", CreateMethod(floatDefinition, parameters, positiveFunction) });

    auto multiplyFunction = [] (shared_ptr<Environment> env) {
        return shared_ptr<Object>( new FloatObject(((FloatObject*)env->Get("this")->object.get())->value * ((FloatObject*)env->Get("other")->object.get())->value));
    };
    rawFloatDefinition->members.insert({ "multiply", CreateMethod(floatDefinition, parameters, multiplyFunction) });

    auto divideFunction = [] (shared_ptr<Environment> env) {
        return shared_ptr<Object>( new FloatObject(((FloatObject*)env->Get("this")->object.get())->value / ((FloatObject*)env->Get("other")->object.get())->value));
    };
    rawFloatDefinition->members.insert({ "divide", CreateMethod(floatDefinition, parameters, divideFunction) });

    auto floatDivideFunction = [] (shared_ptr<Environment> env) {
        return shared_ptr<Object>( new FloatObject((int)(((FloatObject*)env->Get("this")->object.get())->value) / (int)(((FloatObject*)env->Get("other")->object.get())->value)));
    };
    rawFloatDefinition->members.insert({ "integer_divide", CreateMethod(floatDefinition, parameters, floatDivideFunction) });

    auto moduloFunction = [] (shared_ptr<Environment> env) {
        return shared_ptr<Object>( new FloatObject((int)(((FloatObject*)env->Get("this")->object.get())->value) % (int)(((FloatObject*)env->Get("other")->object.get())->value)));
    };
    rawFloatDefinition->members.insert({ "modulo", CreateMethod(floatDefinition, parameters, moduloFunction) });

    auto addFunction = [] (shared_ptr<Environment> env) {
        return shared_ptr<Object>( new FloatObject(((FloatObject*)env->Get("this")->object.get())->value + ((FloatObject*)env->Get("other")->object.get())->value));
    };
    rawFloatDefinition->members.insert({ "add", CreateMethod(floatDefinition, parameters, addFunction) });

    auto subtractFunction = [] (shared_ptr<Environment> env) {
        return shared_ptr<Object>( new FloatObject(((FloatObject*)env->Get("this")->object.get())->value - ((FloatObject*)env->Get("other")->object.get())->value));
    };
    rawFloatDefinition->members.insert({ "subtract", CreateMethod(floatDefinition, parameters, subtractFunction) });

    auto equalityFunction = [] (shared_ptr<Environment> env) {
        return shared_ptr<Object>( new BoolObject(((FloatObject*)env->Get("this")->object.get())->value == ((FloatObject*)env->Get("other")->object.get())->value));
    };
    rawFloatDefinition->members.insert({ "equality", CreateMethod(floatDefinition, parameters, equalityFunction) });

    auto inequalityFunction = [] (shared_ptr<Environment> env) {
        return shared_ptr<Object>( new BoolObject(((FloatObject*)env->Get("this")->object.get())->value != ((FloatObject*)env->Get("other")->object.get())->value));
    };
    rawFloatDefinition->members.insert({ "inequality", CreateMethod(floatDefinition, parameters, inequalityFunction) });

    auto greaterEqualFunction = [] (shared_ptr<Environment> env) {
        return shared_ptr<Object>( new BoolObject(((FloatObject*)env->Get("this")->object.get())->value >= ((FloatObject*)env->Get("other")->object.get())->value));
    };
    rawFloatDefinition->members.insert({ "greater_equal", CreateMethod(floatDefinition, parameters, greaterEqualFunction) });

    auto lessEqualFunction = [] (shared_ptr<Environment> env) {
        return shared_ptr<Object>( new BoolObject(((FloatObject*)env->Get("this")->object.get())->value <= ((FloatObject*)env->Get("other")->object.get())->value));
    };
    rawFloatDefinition->members.insert({ "less_equal", CreateMethod(floatDefinition, parameters, lessEqualFunction) });

    auto greaterFunction = [] (shared_ptr<Environment> env) {
        return shared_ptr<Object>( new BoolObject(((FloatObject*)env->Get("this")->object.get())->value > ((FloatObject*)env->Get("other")->object.get())->value));
    };
    rawFloatDefinition->members.insert({ "greater", CreateMethod(floatDefinition, parameters, greaterFunction) });

    auto lessFunction = [] (shared_ptr<Environment> env) {
        return shared_ptr<Object>( new BoolObject(((FloatObject*)env->Get("this")->object.get())->value < ((FloatObject*)env->Get("other")->object.get())->value));
    };
    rawFloatDefinition->members.insert({ "less", CreateMethod(floatDefinition, parameters, lessFunction) });

    auto assignmentFunction = [] (shared_ptr<Environment> env) {
        ((FloatObject*)env->Get("this")->object.get())->value = ((FloatObject*)env->Get("other")->object.get())->value;
        return shared_ptr<Object>(new VoidObject());
    };
    rawFloatDefinition->members.insert({ "assignment", CreateMethod(floatDefinition, parameters, assignmentFunction) });

    auto addAssignmentFunction = [] (shared_ptr<Environment> env) {
        ((FloatObject*)env->Get("this")->object.get())->value += ((FloatObject*)env->Get("other")->object.get())->value;
        return shared_ptr<Object>(new VoidObject());
    };
    rawFloatDefinition->members.insert({ "add_assignment", CreateMethod(floatDefinition, parameters, addAssignmentFunction) });

    auto subtractAssignmentFunction = [] (shared_ptr<Environment> env) {
        ((FloatObject*)env->Get("this")->object.get())->value -= ((FloatObject*)env->Get("other")->object.get())->value;
        return shared_ptr<Object>(new VoidObject());
    };
    rawFloatDefinition->members.insert({ "subtract_assignment", CreateMethod(floatDefinition, parameters, subtractAssignmentFunction) });

    auto multiplyAssignmentFunction = [] (shared_ptr<Environment> env) {
        ((FloatObject*)env->Get("this")->object.get())->value *= ((FloatObject*)env->Get("other")->object.get())->value;
        return shared_ptr<Object>(new VoidObject());
    };
    rawFloatDefinition->members.insert({ "multiply_assignment", CreateMethod(floatDefinition, parameters, multiplyAssignmentFunction) });

    auto divideAssignmentFunction = [] (shared_ptr<Environment> env) {
        ((FloatObject*)env->Get("this")->object.get())->value /= ((FloatObject*)env->Get("other")->object.get())->value;
        return shared_ptr<Object>(new VoidObject());
    };
    rawFloatDefinition->members.insert({ "divide_assignment", CreateMethod(floatDefinition, parameters, divideAssignmentFunction) });
}

void CreateBoolMethods()
{
    shared_ptr<Object> boolDefinition = Types->Get("bool")->object;
    TypeDefinitionObject *rawBoolDefinition = (TypeDefinitionObject*)boolDefinition.get();

    vector<Parameter> parameters = {Parameter(Type(boolDefinition), "other", optional<Node*>(), None)};

    auto notFunction = [] (shared_ptr<Environment> env) {
        return shared_ptr<Object>( new BoolObject(!(((BoolObject*)env->Get("this")->object.get())->value)));
    };
    rawBoolDefinition->members.insert({ "not", CreateMethod(boolDefinition, parameters, notFunction) });

    auto equalityFunction = [] (shared_ptr<Environment> env) {
        return shared_ptr<Object>( new BoolObject(((BoolObject*)env->Get("this")->object.get())->value == ((BoolObject*)env->Get("other")->object.get())->value));
    };
    rawBoolDefinition->members.insert({ "equality", CreateMethod(boolDefinition, parameters, equalityFunction) });

    auto inequalityFunction = [] (shared_ptr<Environment> env) {
        return shared_ptr<Object>( new BoolObject(((BoolObject*)env->Get("this")->object.get())->value != ((BoolObject*)env->Get("other")->object.get())->value));
    };
    rawBoolDefinition->members.insert({ "inequality", CreateMethod(boolDefinition, parameters, inequalityFunction) });

    auto andFunction = [] (shared_ptr<Environment> env) {
        return shared_ptr<Object>( new BoolObject(((BoolObject*)env->Get("this")->object.get())->value && ((BoolObject*)env->Get("other")->object.get())->value));
    };
    rawBoolDefinition->members.insert({ "and", CreateMethod(boolDefinition, parameters, andFunction) });

    auto orFunction = [] (shared_ptr<Environment> env) {
        return shared_ptr<Object>( new BoolObject(((BoolObject*)env->Get("this")->object.get())->value || ((BoolObject*)env->Get("other")->object.get())->value));
    };
    rawBoolDefinition->members.insert({ "or", CreateMethod(boolDefinition, parameters, orFunction) });

    auto assignmentFunction = [] (shared_ptr<Environment> env) {
        ((BoolObject*)env->Get("this")->object.get())->value = ((BoolObject*)env->Get("other")->object.get())->value;
        return shared_ptr<Object>(new VoidObject());
    };
    rawBoolDefinition->members.insert({ "assignment", CreateMethod(boolDefinition, parameters, assignmentFunction) });
}

void CreateStringMethods()
{
    shared_ptr<Object> stringDefinition = Types->Get("string")->object;
    TypeDefinitionObject *rawStringDefinition = (TypeDefinitionObject*)stringDefinition.get();

    vector<Parameter> parameters = {Parameter(Type(stringDefinition), "other", optional<Node*>(), None)};

    auto addFunction = [] (shared_ptr<Environment> env) {
        return shared_ptr<Object>( new StringObject(((StringObject*)env->Get("this")->object.get())->value + ((StringObject*)env->Get("other")->object.get())->value));
    };
    rawStringDefinition->members.insert({ "add", CreateMethod(stringDefinition, parameters, addFunction) });

    auto equalityFunction = [] (shared_ptr<Environment> env) {
        return shared_ptr<Object>( new BoolObject(((StringObject*)env->Get("this")->object.get())->value == ((StringObject*)env->Get("other")->object.get())->value));
    };
    rawStringDefinition->members.insert({ "equality", CreateMethod(stringDefinition, parameters, equalityFunction) });

    auto inequalityFunction = [] (shared_ptr<Environment> env) {
        return shared_ptr<Object>( new BoolObject(((StringObject*)env->Get("this")->object.get())->value != ((StringObject*)env->Get("other")->object.get())->value));
    };
    rawStringDefinition->members.insert({ "inequality", CreateMethod(stringDefinition, parameters, inequalityFunction) });

    auto assignmentFunction = [] (shared_ptr<Environment> env) {
        ((StringObject*)env->Get("this")->object.get())->value = ((StringObject*)env->Get("other")->object.get())->value;
        return shared_ptr<Object>(new VoidObject());
    };
    rawStringDefinition->members.insert({ "assignment", CreateMethod(stringDefinition, parameters, assignmentFunction) });

    auto addAssignmentFunction = [] (shared_ptr<Environment> env) {
        ((StringObject*)env->Get("this")->object.get())->value += ((StringObject*)env->Get("other")->object.get())->value;
        return shared_ptr<Object>(new VoidObject());
    };
    rawStringDefinition->members.insert({ "add_assignment", CreateMethod(stringDefinition, parameters, addAssignmentFunction) });
}

void CreateTypeMethods()
{
    shared_ptr<Object> typeDefinition = Types->Get("Type")->object;
    TypeDefinitionObject *rawTypeDefinition = (TypeDefinitionObject*)typeDefinition.get();

    vector<Parameter> parameters = {Parameter(Type(typeDefinition), "other", optional<Node*>(), None)};

    auto equalityFunction = [] (shared_ptr<Environment> env) {
        return shared_ptr<Object>( new BoolObject(((TypeObject*)env->Get("this")->object.get())->value == ((TypeObject*)env->Get("other")->object.get())->value));
    };
    rawTypeDefinition->members.insert({ "equality", CreateMethod(typeDefinition, parameters, equalityFunction) });

    auto inequalityFunction = [] (shared_ptr<Environment> env) {
        return shared_ptr<Object>( new BoolObject(((TypeObject*)env->Get("this")->object.get())->value != ((TypeObject*)env->Get("other")->object.get())->value));
    };
    rawTypeDefinition->members.insert({ "inequality", CreateMethod(typeDefinition, parameters, inequalityFunction) });

    auto assignmentFunction = [] (shared_ptr<Environment> env) {
        ((TypeObject*)env->Get("this")->object.get())->value = ((TypeObject*)env->Get("other")->object.get())->value;
        return shared_ptr<Object>(new VoidObject());
    };
    rawTypeDefinition->members.insert({ "assignment", CreateMethod(typeDefinition, parameters, assignmentFunction) });
}

void CreateFunctionMethods()
{
    shared_ptr<Object> functionDefinition = Types->Get("Function")->object;
    TypeDefinitionObject *rawFunctionDefinition = (TypeDefinitionObject*)functionDefinition.get();

    vector<Parameter> parameters = {Parameter(Type(functionDefinition), "other", optional<Node*>(), None)};

    auto assignmentFunction = [] (shared_ptr<Environment> env) {
        ((FunctionObject*)env->Get("this")->object.get())->return_type = ((FunctionObject*)env->Get("other")->object.get())->return_type;
        ((FunctionObject*)env->Get("this")->object.get())->parameters = ((FunctionObject*)env->Get("other")->object.get())->parameters;
        ((FunctionObject*)env->Get("this")->object.get())->body = ((FunctionObject*)env->Get("other")->object.get())->body;
        return shared_ptr<Object>(new VoidObject());
    };
    rawFunctionDefinition->members.insert({ "assignment", CreateMethod(functionDefinition, parameters, assignmentFunction) });
}