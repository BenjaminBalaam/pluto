#ifndef object_hpp
#define object_hpp

#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <tuple>
#include <optional>
#include <memory>
#include <variant>
#include <functional>

#include "node.hpp"
#include "error.hpp"

class Environment;
class Object;

enum RETURN_REASON
{
    ReturnStatement,
    BreakStatement,
    ContinueStatement,
    EndOfAST,
};

class Type
{
    public:
        std::shared_ptr<Object> type_definition;

        Type(std::shared_ptr<Object> type_definition);

        bool operator==(Type other);

        bool operator!=(Type other);

        friend std::ostream &operator<<(std::ostream &os, const Type &type);
};

class Qualifier
{
    public:
        bool is_public;
        bool is_static;
        bool is_const;

        Qualifier(bool is_public = false, bool is_static = false, bool is_const = false);

        Qualifier(std::vector<std::string> qualifiers);

        friend std::ostream &operator<<(std::ostream &os, const Qualifier &qualifier);
};

class Object
{
    public:
        Type type;

        Object();

        virtual std::string to_string() = 0;

        virtual ~Object();

        friend std::ostream &operator<<(std::ostream &os, Object &object);
};

class Member
{
    public:
        std::shared_ptr<Object> object;
        Qualifier qualifiers;

        Member(std::shared_ptr<Object> object, Qualifier qualifiers);

        friend std::ostream &operator<<(std::ostream &os, const Member &member);
};

class Parameter
{
    public:
        Type type;
        std::string name;
        std::optional<Node*> default_argument;
        ARGUMENT_EXPANSION argument_expansion;

        Parameter(Type type, std::string name, std::optional<Node*> default_argument, ARGUMENT_EXPANSION argument_expansion);

        friend std::ostream &operator<<(std::ostream &os, const Parameter &parameter);
};

class Variable
{
    public:
        std::shared_ptr<Object> object;
        Qualifier qualifiers;

        Variable(std::shared_ptr<Object> object, Qualifier qualifiers);

        friend std::ostream &operator<<(std::ostream &os, const Variable &variable);
};

class Environment
{
    public:
        std::shared_ptr<Environment> parent_environment;
        std::map<std::string, Variable> variables;

        Environment(std::shared_ptr<Environment> parent_environment, std::map<std::string, Variable> variables);

        std::optional<Variable> Get(std::string name);

        void Add(std::string name, Variable variable);

        friend std::ostream &operator<<(std::ostream &os, const Environment &environment);
};

class Call
{
    public:
        std::string node_type;
        std::optional<Type> return_type;

        Call(std::string node_type, std::optional<Type> return_type);

        friend std::ostream &operator<<(std::ostream &os, const Call &call);
};

class IntObject : public Object
{
    public:
        int value;

        IntObject(int value);

        std::string to_string();

        ~IntObject();
};

class FloatObject : public Object
{
    public:
        float value;

        FloatObject(float value);

        std::string to_string();

        ~FloatObject();
};

class BoolObject : public Object
{
    public:
        bool value;

        BoolObject(bool value);

        std::string to_string();

        ~BoolObject();
};

class StringObject : public Object
{
    public:
        std::string value;

        StringObject(std::string value);

        std::string to_string();

        ~StringObject();
};

class TypeObject : public Object
{
    public:
        Type value;

        TypeObject(Type value);

        std::string to_string();

        ~TypeObject();
};

class TypeDefinitionObject : public Object
{
    public:
        std::string name;
        std::map<std::string, Member> members;

        TypeDefinitionObject(std::string name, std::map<std::string, Member> members);

        std::optional<Member> GetMember(std::string name);

        std::string to_string();

        ~TypeDefinitionObject();
};

class ClassInstanceObject : public Object
{
    public:
        Type class_type;

        ClassInstanceObject(Type class_type);

        std::string to_string();

        ~ClassInstanceObject();
};

class FunctionObject : public Object
{
    public:
        Type return_type;
        std::vector<Parameter> parameters;
        std::variant<std::vector<Node*>, std::function<std::shared_ptr<Object>(std::shared_ptr<Environment>)>> body;

        FunctionObject(Type return_type, std::vector<Parameter> parameters, std::variant<std::vector<Node*>, std::function<std::shared_ptr<Object>(std::shared_ptr<Environment>)>> body);

        std::string to_string();

        ~FunctionObject();
};

class VoidObject : public Object
{
    public:
        VoidObject();

        std::string to_string();

        ~VoidObject();
};

class ErrorObject : public Object
{
    public:
        int start;
        int end;
        Error error;

        ErrorObject(int start, int end, Error error);

        std::string to_string();

        ~ErrorObject();
};

void InitialiseInterpreterData();

extern std::shared_ptr<Environment> Types;

Member CreateMethod(std::shared_ptr<Object> definition, std::vector<Parameter> parameters, std::function<std::shared_ptr<Object>(std::shared_ptr<Environment>)> function);

void CreateIntMethods();
void CreateFloatMethods();
void CreateBoolMethods();
void CreateStringMethods();
void CreateTypeMethods();
void CreateFunctionMethods();

#endif