#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <sstream>
#include <tuple>
#include <optional>
#include <memory>

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
    return value;
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

shared_ptr<Environment> Types = std::shared_ptr<Environment>(new Environment(NULL, std::map<std::string, Variable> {
    { "void", Variable(shared_ptr<Object>(new TypeDefinitionObject("void")), Qualifier()) },
    { "int", Variable(shared_ptr<Object>(new TypeDefinitionObject("int")), Qualifier()) },
    { "float", Variable(shared_ptr<Object>(new TypeDefinitionObject("float")), Qualifier()) },
    { "bool", Variable(shared_ptr<Object>(new TypeDefinitionObject("bool")), Qualifier()) },
    { "string", Variable(shared_ptr<Object>(new TypeDefinitionObject("string")), Qualifier()) },
    { "Type", Variable(shared_ptr<Object>(new TypeDefinitionObject("Type")), Qualifier()) },
    { "Function", Variable(shared_ptr<Object>(new TypeDefinitionObject("Function")), Qualifier()) },
}));