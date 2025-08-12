#include "catch.hpp"

#include "../src/node.hpp"
#include "../src/lexer.hpp"
#include "../src/syntax_analyser.hpp"

using namespace std;

TEST_CASE("Test Syntax Analyser Type Expressions")
{
    string text = "foo<bar> test<a, b, c> foo[] this<is<a, good[], test<>>>";

    vector<Node*> AST = get<0>(AnalyseSyntax(Tokenise(text)));

    REQUIRE( AST[0]->type == "TypeExpression" );
    REQUIRE( ((TypeExpression*)AST[0])->name == "foo" );
    REQUIRE( ((TypeExpression*)AST[0])->content[0].name == "bar" );

    REQUIRE( AST[1]->type == "TypeExpression" );
    REQUIRE( ((TypeExpression*)AST[1])->name == "test" );
    REQUIRE( ((TypeExpression*)AST[1])->content[0].name == "a" );
    REQUIRE( ((TypeExpression*)AST[1])->content[1].name == "b" );
    REQUIRE( ((TypeExpression*)AST[1])->content[2].name == "c" );

    REQUIRE( AST[2]->type == "TypeExpression" );
    REQUIRE( ((TypeExpression*)AST[2])->name == "foo" );
    REQUIRE( ((TypeExpression*)AST[2])->is_array );

    REQUIRE( AST[3]->type == "TypeExpression" );
    REQUIRE( ((TypeExpression*)AST[3])->name == "this" );
    REQUIRE( ((TypeExpression*)AST[3])->content[0].name == "is" );
    REQUIRE( ((TypeExpression*)AST[3])->content[0].content[0].name == "a" );
    REQUIRE( ((TypeExpression*)AST[3])->content[0].content[1].name == "good" );
    REQUIRE( ((TypeExpression*)AST[3])->content[0].content[1].is_array );
    REQUIRE( ((TypeExpression*)AST[3])->content[0].content[2].name == "test" );
    REQUIRE( ((TypeExpression*)AST[3])->content[0].content[2].content.size() == 0 );
}

TEST_CASE("Test Syntax Analyser Invalid Type Expressions")
{
    string text = "list<";

    try
    {
        AnalyseSyntax(Tokenise(text));

        FAIL();
    }
    catch (Node *node)
    {
        REQUIRE( node->error->type == SyntaxError );
        REQUIRE( node->error->text == "Missing end of type" );
    }

    text = "list<int";

    try
    {
        AnalyseSyntax(Tokenise(text));

        FAIL();
    }
    catch (Node *node)
    {
        REQUIRE( node->error->type == SyntaxError );
        REQUIRE( node->error->text == "Missing ending ;" ); // Syntax Analyser believes this is a less than operation
    }

    text = "list<int,";

    try
    {
        AnalyseSyntax(Tokenise(text));

        FAIL();
    }
    catch (Node *node)
    {
        REQUIRE( node->error->type == SyntaxError );
        REQUIRE( node->error->text == "Missing end of type" );
    }
}

TEST_CASE("Test Syntax Analyser Qualifier Expressions")
{
    string text = "public public static public static const;";
    vector<Node*> AST = get<0>(AnalyseSyntax(Tokenise(text)));

    REQUIRE( AST[0]->type == "QualifierExpression" );
    REQUIRE( ((QualifierExpression*)AST[0])->qualifiers == vector<string> { "public" } );

    REQUIRE( AST[1]->type == "QualifierExpression" );
    REQUIRE( ((QualifierExpression*)AST[1])->qualifiers == vector<string> { "public", "static" } );

    REQUIRE( AST[2]->type == "QualifierExpression" );
    REQUIRE( ((QualifierExpression*)AST[2])->qualifiers == vector<string> { "public", "static", "const" } );

    text = "const static public;";
    AST = get<0>(AnalyseSyntax(Tokenise(text)));

    REQUIRE( AST[0]->type == "QualifierExpression" );
    REQUIRE_FALSE( ((QualifierExpression*)AST[0])->qualifiers == vector<string> { "const", "static", "public" } ); // Qualifiers must be written in the correct order

    text = "public";

    try
    {
        AST = get<0>(AnalyseSyntax(Tokenise(text)));
    }
    catch (Node *node)
    {
        REQUIRE( node->error->type == SyntaxError );
        REQUIRE( node->error->text == "Missing ending of statement" );
    }
}

TEST_CASE("Test Syntax Analyser Literals")
{
    string text = "23 5.2 'foo'";

    vector<Node*> AST = get<0>(AnalyseSyntax(Tokenise(text)));

    REQUIRE( AST[0]->type == "Literal" );
    REQUIRE( ((Literal*)AST[0])->l_integer.value() == 23 );

    REQUIRE( AST[1]->type == "Literal" );
    REQUIRE( ((Literal*)AST[1])->l_float.value() == 5.2 );

    REQUIRE( AST[2]->type == "Literal" );
    REQUIRE( ((Literal*)AST[2])->l_string.value() == "foo" );
}

TEST_CASE("Test Syntax Analyser Code Block")
{
    string text = "{'foo'}; () {'foo'}; (int foo) {'bar'}; (int *foo = 0, float **bar) -> string {'foobar'};";

    vector<Node*> AST = get<0>(AnalyseSyntax(Tokenise(text)));

    REQUIRE( AST[0]->type == "CodeBlock" );
    REQUIRE( ((CodeBlock*)AST[0])->return_type.name == "void" );
    REQUIRE( ((CodeBlock*)AST[0])->parameters.size() == 0 );
    REQUIRE( ((CodeBlock*)AST[0])->content[0]->type == "Literal" );

    REQUIRE( AST[2]->type == "CodeBlock" );
    REQUIRE( ((CodeBlock*)AST[2])->return_type.name == "void" );
    REQUIRE( ((CodeBlock*)AST[2])->parameters.size() == 0 );
    REQUIRE( ((CodeBlock*)AST[2])->content[0]->type == "Literal" );
    REQUIRE( ((Literal*)((CodeBlock*)AST[2])->content[0])->l_string == "foo" );

    REQUIRE( AST[4]->type == "CodeBlock" );
    REQUIRE( ((CodeBlock*)AST[4])->return_type.name == "void" );
    REQUIRE( ((CodeBlock*)AST[4])->parameters[0].type_data.name == "int" );
    REQUIRE( ((CodeBlock*)AST[4])->parameters[0].name == "foo" );
    REQUIRE( ((CodeBlock*)AST[4])->content[0]->type == "Literal" );
    REQUIRE( ((Literal*)((CodeBlock*)AST[4])->content[0])->l_string == "bar" );

    REQUIRE( AST[6]->type == "CodeBlock" );
    REQUIRE( ((CodeBlock*)AST[6])->return_type.name == "string" );
    REQUIRE( ((CodeBlock*)AST[6])->parameters[0].type_data.name == "int" );
    REQUIRE( ((CodeBlock*)AST[6])->parameters[0].name == "foo" );
    REQUIRE( ((CodeBlock*)AST[6])->parameters[0].default_argument.value()->type == "Literal" );
    REQUIRE( ((Literal*)((CodeBlock*)AST[6])->parameters[0].default_argument.value())->l_integer == 0 );
    REQUIRE( ((CodeBlock*)AST[6])->parameters[0].argument_expansion == Array );
    REQUIRE( ((CodeBlock*)AST[6])->parameters[1].type_data.name == "float" );
    REQUIRE( ((CodeBlock*)AST[6])->parameters[1].name == "bar" );
    REQUIRE( ((CodeBlock*)AST[6])->parameters[1].argument_expansion == Dictionary );
    REQUIRE( ((CodeBlock*)AST[6])->content[0]->type == "Literal" );
    REQUIRE( ((Literal*)((CodeBlock*)AST[6])->content[0])->l_string == "foobar" );
}

TEST_CASE("Test Syntax Analyser Invalid Code Block")
{
    string text = "(";

    try
    {
        AnalyseSyntax(Tokenise(text));

        FAIL();
    }
    catch (Node *node)
    {
        REQUIRE( node->error->type == SyntaxError );
        REQUIRE( node->error->text == "Missing ending )" );
    }

    text = "(,";

    try
    {
        AnalyseSyntax(Tokenise(text));

        FAIL();
    }
    catch (Node *node)
    {
        REQUIRE( node->error->type == SyntaxError );
        REQUIRE( node->error->text == "Missing argument" );
    }

    text = "(int foo,";

    try
    {
        AnalyseSyntax(Tokenise(text));

        FAIL();
    }
    catch (Node *node)
    {
        REQUIRE( node->error->type == SyntaxError );
        REQUIRE( node->error->text == "Missing end of code block" );
    }

    text = "(int";

    try
    {
        AnalyseSyntax(Tokenise(text));

        FAIL();
    }
    catch (Node *node)
    {
        REQUIRE( node->error->type == SyntaxError );
        REQUIRE( node->error->text == "Missing end of code block" );
    }

    text = "(int *";

    try
    {
        AnalyseSyntax(Tokenise(text));

        FAIL();
    }
    catch (Node *node)
    {
        REQUIRE( node->error->type == SyntaxError );
        REQUIRE( node->error->text == "Missing end of code block" );
    }

    text = "(int **";

    try
    {
        AnalyseSyntax(Tokenise(text));

        FAIL();
    }
    catch (Node *node)
    {
        REQUIRE( node->error->type == SyntaxError );
        REQUIRE( node->error->text == "Missing end of code block" );
    }

    text = "(int foo";

    try
    {
        AnalyseSyntax(Tokenise(text));

        FAIL();
    }
    catch (Node *node)
    {
        REQUIRE( node->error->type == SyntaxError );
        REQUIRE( node->error->text == "Missing end of code block" );
    }

    text = "(int foo =";

    try
    {
        AnalyseSyntax(Tokenise(text));

        FAIL();
    }
    catch (Node *node)
    {
        REQUIRE( node->error->type == SyntaxError );
        REQUIRE( node->error->text == "Missing end of default argument" );
    }

    text = "(int foo = 0";

    try
    {
        AnalyseSyntax(Tokenise(text));

        FAIL();
    }
    catch (Node *node)
    {
        REQUIRE( node->error->type == SyntaxError );
        REQUIRE( node->error->text == "Missing ending )" );
    }

    text = "(0)"; // Will have to change when brackets implemented

    try
    {
        AnalyseSyntax(Tokenise(text));

        FAIL();
    }
    catch (Node *node)
    {
        REQUIRE( node->error->type == SyntaxError );
        REQUIRE( node->error->text == "Invalid character in code block" );
    }

    text = "(int 0)";

    try
    {
        AnalyseSyntax(Tokenise(text));

        FAIL();
    }
    catch (Node *node)
    {
        REQUIRE( node->error->type == SyntaxError );
        REQUIRE( node->error->text == "Invalid character in code block" );
    }

    text = "()";

    try
    {
        AnalyseSyntax(Tokenise(text));

        FAIL();
    }
    catch (Node *node)
    {
        REQUIRE( node->error->type == SyntaxError );
        REQUIRE( node->error->text == "Missing end of code block" );
    }

    text = "() ->";

    try
    {
        AnalyseSyntax(Tokenise(text));

        FAIL();
    }
    catch (Node *node)
    {
        REQUIRE( node->error->type == SyntaxError );
        REQUIRE( node->error->text == "Missing return type" );
    }

    text = "() -> int";

    try
    {
        AnalyseSyntax(Tokenise(text));

        FAIL();
    }
    catch (Node *node)
    {
        REQUIRE( node->error->type == SyntaxError );
        REQUIRE( node->error->text == "Missing end of code block" );
    }

    text = "() -> 0 {}";

    try
    {
        AnalyseSyntax(Tokenise(text));

        FAIL();
    }
    catch (Node *node)
    {
        REQUIRE( node->error->type == SyntaxError );
        REQUIRE( node->error->text == "Invalid character in code block" );
    }

    text = "() -> string {";

    try
    {
        AnalyseSyntax(Tokenise(text));

        FAIL();
    }
    catch (Node *node)
    {
        REQUIRE( node->error->type == SyntaxError );
        REQUIRE( node->error->text == "Missing ending }" );
    }

    text = "() -> string 0";

    try
    {
        AnalyseSyntax(Tokenise(text));

        FAIL();
    }
    catch (Node *node)
    {
        REQUIRE( node->error->type == SyntaxError );
        REQUIRE( node->error->text == "Invalid character in code block" );
    }

    text = "() -> string {0";

    try
    {
        AnalyseSyntax(Tokenise(text));

        FAIL();
    }
    catch (Node *node)
    {
        REQUIRE( node->error->type == SyntaxError );
        REQUIRE( node->error->text == "Missing ending }" );
    }

    text = "{";

    try
    {
        AnalyseSyntax(Tokenise(text));

        FAIL();
    }
    catch (Node *node)
    {
        REQUIRE( node->error->type == SyntaxError );
        REQUIRE( node->error->text == "Missing ending }" );
    }

    text = "{0";

    try
    {
        AnalyseSyntax(Tokenise(text));

        FAIL();
    }
    catch (Node *node)
    {
        REQUIRE( node->error->type == SyntaxError );
        REQUIRE( node->error->text == "Missing ending }" );
    }
}

TEST_CASE("Test Syntax Analyser Operation") // All needs to change for order of operations -a ^ b * c + d == e & f | g = h;
{

    string text = "a ^ b; -a; a * b; a + b; a == b; a & b; a | b; a = b; a * b + c; a + b * c; a + b * c == d; a == b * c + d; func(a < b);";
    vector<Node*> AST = get<0>(AnalyseSyntax(Tokenise(text)));

    REQUIRE( AST[0]->type == "Operation" );
    REQUIRE( ((Operation*)AST[0])->operator_string == "^" );
    REQUIRE( ((Operation*)AST[0])->left->type == "GetVariable" );
    REQUIRE( ((GetVariable*)((Operation*)AST[0])->left)->name == "a" );
    REQUIRE( ((Operation*)AST[0])->right->type == "GetVariable" );
    REQUIRE( ((GetVariable*)((Operation*)AST[0])->right)->name == "b" );

    REQUIRE( AST[2]->type == "Operation" );
    REQUIRE( ((Operation*)AST[2])->operator_string == "-" );
    REQUIRE( ((Operation*)AST[2])->left == NULL );
    REQUIRE( ((Operation*)AST[2])->right->type == "GetVariable" );
    REQUIRE( ((GetVariable*)((Operation*)AST[2])->right)->name == "a" );

    REQUIRE( AST[4]->type == "Operation" );
    REQUIRE( ((Operation*)AST[4])->operator_string == "*" );
    REQUIRE( ((Operation*)AST[4])->left->type == "GetVariable" );
    REQUIRE( ((GetVariable*)((Operation*)AST[4])->left)->name == "a" );
    REQUIRE( ((Operation*)AST[4])->right->type == "GetVariable" );
    REQUIRE( ((GetVariable*)((Operation*)AST[4])->right)->name == "b" );

    REQUIRE( AST[6]->type == "Operation" );
    REQUIRE( ((Operation*)AST[6])->operator_string == "+" );
    REQUIRE( ((Operation*)AST[6])->left->type == "GetVariable" );
    REQUIRE( ((GetVariable*)((Operation*)AST[6])->left)->name == "a" );
    REQUIRE( ((Operation*)AST[6])->right->type == "GetVariable" );
    REQUIRE( ((GetVariable*)((Operation*)AST[6])->right)->name == "b" );

    REQUIRE( AST[8]->type == "Operation" );
    REQUIRE( ((Operation*)AST[8])->operator_string == "==" );
    REQUIRE( ((Operation*)AST[8])->left->type == "GetVariable" );
    REQUIRE( ((GetVariable*)((Operation*)AST[8])->left)->name == "a" );
    REQUIRE( ((Operation*)AST[8])->right->type == "GetVariable" );
    REQUIRE( ((GetVariable*)((Operation*)AST[8])->right)->name == "b" );

    REQUIRE( AST[10]->type == "Operation" );
    REQUIRE( ((Operation*)AST[10])->operator_string == "&" );
    REQUIRE( ((Operation*)AST[10])->left->type == "GetVariable" );
    REQUIRE( ((GetVariable*)((Operation*)AST[10])->left)->name == "a" );
    REQUIRE( ((Operation*)AST[10])->right->type == "GetVariable" );
    REQUIRE( ((GetVariable*)((Operation*)AST[10])->right)->name == "b" );

    REQUIRE( AST[12]->type == "Operation" );
    REQUIRE( ((Operation*)AST[12])->operator_string == "|" );
    REQUIRE( ((Operation*)AST[12])->left->type == "GetVariable" );
    REQUIRE( ((GetVariable*)((Operation*)AST[12])->left)->name == "a" );
    REQUIRE( ((Operation*)AST[12])->right->type == "GetVariable" );
    REQUIRE( ((GetVariable*)((Operation*)AST[12])->right)->name == "b" );

    REQUIRE( AST[14]->type == "Operation" );
    REQUIRE( ((Operation*)AST[14])->operator_string == "=" );
    REQUIRE( ((Operation*)AST[14])->left->type == "GetVariable" );
    REQUIRE( ((GetVariable*)((Operation*)AST[14])->left)->name == "a" );
    REQUIRE( ((Operation*)AST[14])->right->type == "GetVariable" );
    REQUIRE( ((GetVariable*)((Operation*)AST[14])->right)->name == "b" );

    REQUIRE( AST[16]->type == "Operation" );
    REQUIRE( ((Operation*)AST[16])->operator_string == "+" );
    REQUIRE( ((Operation*)AST[16])->left->type == "Operation" );
    REQUIRE( ((Operation*)((Operation*)AST[16])->left)->operator_string == "*" );
    REQUIRE( ((Operation*)((Operation*)AST[16])->left)->left->type == "GetVariable" );
    REQUIRE( ((GetVariable*)((Operation*)((Operation*)AST[16])->left)->left)->name == "a" );
    REQUIRE( ((Operation*)((Operation*)AST[16])->left)->right->type == "GetVariable" );
    REQUIRE( ((GetVariable*)((Operation*)((Operation*)AST[16])->left)->right)->name == "b" );
    REQUIRE( ((Operation*)AST[16])->right->type == "GetVariable" );
    REQUIRE( ((GetVariable*)((Operation*)AST[16])->right)->name == "c" );

    REQUIRE( AST[18]->type == "Operation" );
    REQUIRE( ((Operation*)AST[18])->operator_string == "+" );
    REQUIRE( ((Operation*)AST[18])->left->type == "GetVariable" );
    REQUIRE( ((GetVariable*)((Operation*)AST[18])->left)->name == "a" );
    REQUIRE( ((Operation*)AST[18])->right->type == "Operation" );
    REQUIRE( ((Operation*)((Operation*)AST[18])->right)->operator_string == "*" );
    REQUIRE( ((Operation*)((Operation*)AST[18])->right)->right->type == "GetVariable" );
    REQUIRE( ((GetVariable*)((Operation*)((Operation*)AST[18])->right)->left)->name == "b" );
    REQUIRE( ((Operation*)((Operation*)AST[18])->right)->right->type == "GetVariable" );
    REQUIRE( ((GetVariable*)((Operation*)((Operation*)AST[18])->right)->right)->name == "c" );

    REQUIRE( AST[20]->type == "Operation" );
    REQUIRE( ((Operation*)AST[20])->operator_string == "==" );
    REQUIRE( ((Operation*)AST[20])->left->type == "Operation" );
    REQUIRE( ((Operation*)((Operation*)AST[20])->left)->operator_string == "+" );
    REQUIRE( ((Operation*)((Operation*)AST[20])->left)->left->type == "GetVariable" );
    REQUIRE( ((GetVariable*)((Operation*)((Operation*)AST[20])->left)->left)->name == "a" );
    REQUIRE( ((Operation*)((Operation*)AST[20])->left)->right->type == "Operation" );
    REQUIRE( ((Operation*)((Operation*)((Operation*)AST[20])->left)->right)->operator_string == "*" );
    REQUIRE( ((Operation*)((Operation*)((Operation*)AST[20])->left)->right)->left->type == "GetVariable" );
    REQUIRE( ((GetVariable*)((Operation*)((Operation*)((Operation*)AST[20])->left)->right)->left)->name == "b" );
    REQUIRE( ((Operation*)((Operation*)((Operation*)AST[20])->left)->right)->right->type == "GetVariable" );
    REQUIRE( ((GetVariable*)((Operation*)((Operation*)((Operation*)AST[20])->left)->right)->right)->name == "c" );
    REQUIRE( ((Operation*)AST[20])->right->type == "GetVariable" );
    REQUIRE( ((GetVariable*)((Operation*)AST[20])->right)->name == "d" );

    REQUIRE( AST[22]->type == "Operation" );
    REQUIRE( ((Operation*)AST[22])->operator_string == "==" );
    REQUIRE( ((Operation*)AST[22])->left->type == "GetVariable" );
    REQUIRE( ((GetVariable*)((Operation*)AST[22])->left)->name == "a" );
    REQUIRE( ((Operation*)AST[22])->right->type == "Operation" );
    REQUIRE( ((Operation*)((Operation*)AST[22])->right)->operator_string == "+" );
    REQUIRE( ((Operation*)((Operation*)AST[22])->right)->left->type == "Operation" );
    REQUIRE( ((Operation*)((Operation*)((Operation*)AST[22])->right)->left)->operator_string == "*" );
    REQUIRE( ((Operation*)((Operation*)((Operation*)AST[22])->right)->left)->left->type == "GetVariable" );
    REQUIRE( ((GetVariable*)((Operation*)((Operation*)((Operation*)AST[22])->right)->left)->left)->name == "b" );
    REQUIRE( ((Operation*)((Operation*)((Operation*)AST[22])->right)->left)->right->type == "GetVariable" );
    REQUIRE( ((GetVariable*)((Operation*)((Operation*)((Operation*)AST[22])->right)->left)->right)->name == "c" );
    REQUIRE( ((Operation*)((Operation*)AST[22])->right)->right->type == "GetVariable" );
    REQUIRE( ((GetVariable*)((Operation*)((Operation*)AST[22])->right)->right)->name == "d" );

    REQUIRE( AST[24]->type == "FunctionCall" );
    REQUIRE( ((FunctionCall*)AST[24])->arguments[0]->type == "Operation" );
    REQUIRE( ((Operation*)((FunctionCall*)AST[24])->arguments[0])->operator_string == "<" );
    REQUIRE( (((Operation*)((FunctionCall*)AST[24])->arguments[0])->left)->type == "GetVariable" );
    REQUIRE( ((GetVariable*)(((Operation*)((FunctionCall*)AST[24])->arguments[0])->left))->name == "a" );
    REQUIRE( (((Operation*)((FunctionCall*)AST[24])->arguments[0])->right)->type == "GetVariable" );
    REQUIRE( ((GetVariable*)(((Operation*)((FunctionCall*)AST[24])->arguments[0])->right))->name == "b" );

    text = "> a";

    try
    {
        AnalyseSyntax(Tokenise(text));

        FAIL();
    }
    catch (Node *node)
    {
        REQUIRE( node->error->type == SyntaxError );
        REQUIRE( node->error->text == "Invalid statement start" );
    }

    text = "!";

    try
    {
        AnalyseSyntax(Tokenise(text));

        FAIL();
    }
    catch (Node *node)
    {
        REQUIRE( node->error->type == SyntaxError );
        REQUIRE( node->error->text == "Missing right expression for operation" );
    }

    text = "!a";

    try
    {
        AnalyseSyntax(Tokenise(text));

        FAIL();
    }
    catch (Node *node)
    {
        REQUIRE( node->error->type == SyntaxError );
        REQUIRE( node->error->text == "Missing ending ;" );
    }

    text = "!;";

    try
    {
        AnalyseSyntax(Tokenise(text));

        FAIL();
    }
    catch (Node *node)
    {
        REQUIRE( node->error->type == SyntaxError );
        REQUIRE( node->error->text == "Missing right expression for operation" );
    }

    text = "a >";

    try
    {
        AnalyseSyntax(Tokenise(text));

        FAIL();
    }
    catch (Node *node)
    {
        REQUIRE( node->error->type == SyntaxError );
        REQUIRE( node->error->text == "Missing right expression for operation" );
    }

    text = "a > b";

    try
    {
        AnalyseSyntax(Tokenise(text));

        FAIL();
    }
    catch (Node *node)
    {
        REQUIRE( node->error->type == SyntaxError );
        REQUIRE( node->error->text == "Missing ending ;" );
    }

    text = "a >;";

    try
    {
        AnalyseSyntax(Tokenise(text));

        FAIL();
    }
    catch (Node *node)
    {
        REQUIRE( node->error->type == SyntaxError );
        REQUIRE( node->error->text == "Missing right expression for operation" );
    }
}

TEST_CASE("Test Syntax Analyser Get Variable")
{
    string text = "foo; bar;";

    vector<Node*> AST = get<0>(AnalyseSyntax(Tokenise(text)));

    REQUIRE( AST[0]->type == "GetVariable" );
    REQUIRE( ((GetVariable*)AST[0])->name == "foo" );

    REQUIRE( AST[2]->type == "GetVariable" );
    REQUIRE( ((GetVariable*)AST[2])->name == "bar" );
}

TEST_CASE("Test Syntax Analyser Assign Variable")
{
    string text = "int foo = 0;";
    vector<Node*> AST = get<0>(AnalyseSyntax(Tokenise(text)));

    REQUIRE( AST[0]->type == "DeclareVariable" );
    REQUIRE( ((DeclareVariable*)AST[0])->variable_type.name == "int" );
    REQUIRE( ((DeclareVariable*)AST[0])->name == "foo" );
    REQUIRE( ((DeclareVariable*)AST[0])->value->type == "Literal" );
    REQUIRE( ((Literal*)((DeclareVariable*)AST[0])->value)->l_integer == 0 );

    text = "static const int foo = 0;";
    AST = get<0>(AnalyseSyntax(Tokenise(text)));

    REQUIRE( AST[0]->type == "DeclareVariable" );
    REQUIRE( ((DeclareVariable*)AST[0])->qualifier->qualifiers == vector<string> { "static", "const" } );
    REQUIRE( ((DeclareVariable*)AST[0])->variable_type.name == "int" );
    REQUIRE( ((DeclareVariable*)AST[0])->name == "foo" );
    REQUIRE( ((DeclareVariable*)AST[0])->value->type == "Literal" );
    REQUIRE( ((Literal*)((DeclareVariable*)AST[0])->value)->l_integer == 0 );

    text = "int foo;";
    AST = get<0>(AnalyseSyntax(Tokenise(text)));

    REQUIRE( AST[0]->type == "DeclareVariable" );
    REQUIRE( ((DeclareVariable*)AST[0])->variable_type.name == "int" );
    REQUIRE( ((DeclareVariable*)AST[0])->name == "foo" );
    REQUIRE( ((DeclareVariable*)AST[0])->value == NULL );

    text = "static const int foo;";
    AST = get<0>(AnalyseSyntax(Tokenise(text)));

    REQUIRE( AST[0]->type == "DeclareVariable" );
    REQUIRE( ((DeclareVariable*)AST[0])->qualifier->qualifiers == vector<string> { "static", "const" } );
    REQUIRE( ((DeclareVariable*)AST[0])->variable_type.name == "int" );
    REQUIRE( ((DeclareVariable*)AST[0])->name == "foo" );
    REQUIRE( ((DeclareVariable*)AST[0])->value == NULL );

    text = "int foo = 0"; // No ending semi-colon

    try
    {
        AnalyseSyntax(Tokenise(text));

        FAIL();
    }
    catch (Node *node)
    {
        REQUIRE( node->error->type == SyntaxError );
        REQUIRE( node->error->text == "Missing ending ;" );
    }

    text = "int foo = ;";

    try
    {
        AnalyseSyntax(Tokenise(text));

        FAIL();
    }
    catch (Node *node)
    {
        REQUIRE( node->error->type == SyntaxError );
        REQUIRE( node->error->text == "Missing declaration value" );
    }

    text = "int foo =";

    try
    {
        AnalyseSyntax(Tokenise(text));

        FAIL();
    }
    catch (Node *node)
    {
        REQUIRE( node->error->type == SyntaxError );
        REQUIRE( node->error->text == "Missing end of declaration" );
    }
}

TEST_CASE("Test Syntax Analyser Function Call")
{
    string text = "foo(); bar(0.); foobar(a + b);";
    vector<Node*> AST = get<0>(AnalyseSyntax(Tokenise(text)));

    REQUIRE( AST[0]->type == "FunctionCall" );
    REQUIRE( ((FunctionCall*)AST[0])->name == "foo" );
    REQUIRE( ((FunctionCall*)AST[0])->arguments.size() == 0 );

    REQUIRE( AST[2]->type == "FunctionCall" );
    REQUIRE( ((FunctionCall*)AST[2])->name == "bar" );
    REQUIRE( ((Literal*)((FunctionCall*)AST[2])->arguments[0])->l_float == 0. );
    
    REQUIRE( AST[4]->type == "FunctionCall" );
    REQUIRE( ((FunctionCall*)AST[4])->name == "foobar" );
    REQUIRE( ((Operation*)((FunctionCall*)AST[4])->arguments[0])->operator_string == "+" );
    REQUIRE( ((Operation*)((FunctionCall*)AST[4])->arguments[0])->left->type == "GetVariable" );
    REQUIRE( ((GetVariable*)((Operation*)((FunctionCall*)AST[4])->arguments[0])->left)->name == "a" );
    REQUIRE( ((Operation*)((FunctionCall*)AST[4])->arguments[0])->right->type == "GetVariable" );
    REQUIRE( ((GetVariable*)((Operation*)((FunctionCall*)AST[4])->arguments[0])->right)->name == "b" );

    text = "foo(";
    
    try
    {
        AnalyseSyntax(Tokenise(text));

        FAIL();
    }
    catch (Node *node)
    {
        REQUIRE( node->error->type == SyntaxError );
        REQUIRE( node->error->text == "Missing end of function call" );
    }

    text = "foo(bar, ";
    
    try
    {
        AnalyseSyntax(Tokenise(text));

        FAIL();
    }
    catch (Node *node)
    {
        REQUIRE( node->error->type == SyntaxError );
        REQUIRE( node->error->text == "Missing end of function call" );
    }

    text = "foo(,";
    
    try
    {
        AnalyseSyntax(Tokenise(text));

        FAIL();
    }
    catch (Node *node)
    {
        REQUIRE( node->error->type == SyntaxError );
        REQUIRE( node->error->text == "Missing argument" );
    }

    text = "foo(bar";

    try
    {
        AnalyseSyntax(Tokenise(text));

        FAIL();
    }
    catch (Node *node)
    {
        REQUIRE( node->error->type == SyntaxError );
        REQUIRE( node->error->text == "Missing ending )" );
    }
}

TEST_CASE("Test Syntax Analyser Define Function")
{
    string text = "int foo() {}";
    vector<Node*> AST = get<0>(AnalyseSyntax(Tokenise(text)));

    REQUIRE( AST[0]->type == "DeclareVariable" );
    REQUIRE( ((DeclareVariable*)AST[0])->variable_type.name == "Function" );
    REQUIRE( ((DeclareVariable*)AST[0])->name == "foo" );
    REQUIRE( ((DeclareVariable*)AST[0])->value->type == "CodeBlock" );
    REQUIRE( ((CodeBlock*)((DeclareVariable*)AST[0])->value)->parameters.size() == 0 );
    REQUIRE( ((CodeBlock*)((DeclareVariable*)AST[0])->value)->return_type.name == "int" );
    REQUIRE( ((CodeBlock*)((DeclareVariable*)AST[0])->value)->content.size() == 0 );

    text = "void foo() {}";
    AST = get<0>(AnalyseSyntax(Tokenise(text)));

    REQUIRE( AST[0]->type == "DeclareVariable" );
    REQUIRE( ((DeclareVariable*)AST[0])->variable_type.name == "Function" );
    REQUIRE( ((DeclareVariable*)AST[0])->name == "foo" );
    REQUIRE( ((DeclareVariable*)AST[0])->value->type == "CodeBlock" );
    REQUIRE( ((CodeBlock*)((DeclareVariable*)AST[0])->value)->parameters.size() == 0 );
    REQUIRE( ((CodeBlock*)((DeclareVariable*)AST[0])->value)->return_type.name == "void" );
    REQUIRE( ((CodeBlock*)((DeclareVariable*)AST[0])->value)->content.size() == 0 );

    text = "public static int foo() {}";
    AST = get<0>(AnalyseSyntax(Tokenise(text)));

    DeclareVariable a = *((DeclareVariable*)AST[0]);

    REQUIRE( AST[0]->type == "DeclareVariable" );
    REQUIRE( ((DeclareVariable*)AST[0])->qualifier->qualifiers == vector<string> { "public", "static" } );
    REQUIRE( ((DeclareVariable*)AST[0])->variable_type.name == "Function" );
    REQUIRE( ((DeclareVariable*)AST[0])->name == "foo" );
    REQUIRE( ((DeclareVariable*)AST[0])->value->type == "CodeBlock" );
    REQUIRE( ((CodeBlock*)((DeclareVariable*)AST[0])->value)->parameters.size() == 0 );
    REQUIRE( ((CodeBlock*)((DeclareVariable*)AST[0])->value)->return_type.name == "int" );
    REQUIRE( ((CodeBlock*)((DeclareVariable*)AST[0])->value)->content.size() == 0 );

    text = "int foo() {bar}";
    AST = get<0>(AnalyseSyntax(Tokenise(text)));

    REQUIRE( AST[0]->type == "DeclareVariable" );
    REQUIRE( ((DeclareVariable*)AST[0])->variable_type.name == "Function" );
    REQUIRE( ((DeclareVariable*)AST[0])->name == "foo" );
    REQUIRE( ((DeclareVariable*)AST[0])->value->type == "CodeBlock" );
    REQUIRE( ((CodeBlock*)((DeclareVariable*)AST[0])->value)->parameters.size() == 0 );
    REQUIRE( ((CodeBlock*)((DeclareVariable*)AST[0])->value)->return_type.name == "int" );
    REQUIRE( ((CodeBlock*)((DeclareVariable*)AST[0])->value)->content[0]->type == "GetVariable" );
    REQUIRE( ((GetVariable*)((CodeBlock*)((DeclareVariable*)AST[0])->value)->content[0])->name == "bar" );

    text = "int foo(string bar) {}";
    AST = get<0>(AnalyseSyntax(Tokenise(text)));

    REQUIRE( AST[0]->type == "DeclareVariable" );
    REQUIRE( ((DeclareVariable*)AST[0])->variable_type.name == "Function" );
    REQUIRE( ((DeclareVariable*)AST[0])->name == "foo" );
    REQUIRE( ((DeclareVariable*)AST[0])->value->type == "CodeBlock" );
    REQUIRE( ((CodeBlock*)((DeclareVariable*)AST[0])->value)->return_type.name == "int" );
    REQUIRE( ((CodeBlock*)((DeclareVariable*)AST[0])->value)->content.size() == 0 );
    REQUIRE( ((CodeBlock*)((DeclareVariable*)AST[0])->value)->parameters[0].type_data.name == "string" );
    REQUIRE( ((CodeBlock*)((DeclareVariable*)AST[0])->value)->parameters[0].name == "bar" );

    text = "int foo(string bar = 'abc') {}";
    AST = get<0>(AnalyseSyntax(Tokenise(text)));

    REQUIRE( AST[0]->type == "DeclareVariable" );
    REQUIRE( ((DeclareVariable*)AST[0])->variable_type.name == "Function" );
    REQUIRE( ((DeclareVariable*)AST[0])->name == "foo" );
    REQUIRE( ((DeclareVariable*)AST[0])->value->type == "CodeBlock" );
    REQUIRE( ((CodeBlock*)((DeclareVariable*)AST[0])->value)->return_type.name == "int" );
    REQUIRE( ((CodeBlock*)((DeclareVariable*)AST[0])->value)->content.size() == 0 );
    REQUIRE( ((CodeBlock*)((DeclareVariable*)AST[0])->value)->parameters[0].type_data.name == "string" );
    REQUIRE( ((CodeBlock*)((DeclareVariable*)AST[0])->value)->parameters[0].name == "bar" );
    REQUIRE( ((CodeBlock*)((DeclareVariable*)AST[0])->value)->parameters[0].default_argument.value()->type == "Literal" );
    REQUIRE( ((Literal*)((CodeBlock*)((DeclareVariable*)AST[0])->value)->parameters[0].default_argument.value())->l_string == "abc" );

    text = "int foo(string **bar) {}";
    AST = get<0>(AnalyseSyntax(Tokenise(text)));

    REQUIRE( AST[0]->type == "DeclareVariable" );
    REQUIRE( ((DeclareVariable*)AST[0])->variable_type.name == "Function" );
    REQUIRE( ((DeclareVariable*)AST[0])->name == "foo" );
    REQUIRE( ((DeclareVariable*)AST[0])->value->type == "CodeBlock" );
    REQUIRE( ((CodeBlock*)((DeclareVariable*)AST[0])->value)->return_type.name == "int" );
    REQUIRE( ((CodeBlock*)((DeclareVariable*)AST[0])->value)->content.size() == 0 );
    REQUIRE( ((CodeBlock*)((DeclareVariable*)AST[0])->value)->parameters[0].type_data.name == "string" );
    REQUIRE( ((CodeBlock*)((DeclareVariable*)AST[0])->value)->parameters[0].name == "bar" );
    REQUIRE( ((CodeBlock*)((DeclareVariable*)AST[0])->value)->parameters[0].argument_expansion == Dictionary );

    text = "int foo(string *bar = 'abc') {}";
    AST = get<0>(AnalyseSyntax(Tokenise(text)));

    REQUIRE( AST[0]->type == "DeclareVariable" );
    REQUIRE( ((DeclareVariable*)AST[0])->variable_type.name == "Function" );
    REQUIRE( ((DeclareVariable*)AST[0])->name == "foo" );
    REQUIRE( ((DeclareVariable*)AST[0])->value->type == "CodeBlock" );
    REQUIRE( ((CodeBlock*)((DeclareVariable*)AST[0])->value)->return_type.name == "int" );
    REQUIRE( ((CodeBlock*)((DeclareVariable*)AST[0])->value)->content.size() == 0 );
    REQUIRE( ((CodeBlock*)((DeclareVariable*)AST[0])->value)->parameters[0].type_data.name == "string" );
    REQUIRE( ((CodeBlock*)((DeclareVariable*)AST[0])->value)->parameters[0].name == "bar" );
    REQUIRE( ((CodeBlock*)((DeclareVariable*)AST[0])->value)->parameters[0].default_argument.value()->type == "Literal" );
    REQUIRE( ((Literal*)((CodeBlock*)((DeclareVariable*)AST[0])->value)->parameters[0].default_argument.value())->l_string == "abc" );
    REQUIRE( ((CodeBlock*)((DeclareVariable*)AST[0])->value)->parameters[0].argument_expansion == Array );
}

TEST_CASE("Test Syntax Analyser Class Definition")
{
    string text = "class foo { int test; } class foo : bar { int test; }";
    vector<Node*> AST = get<0>(AnalyseSyntax(Tokenise(text)));

    REQUIRE( AST[0]->type == "ClassDefinition" );
    REQUIRE( ((ClassDefinition*)AST[0])->name == "foo" );
    REQUIRE( ((ClassDefinition*)AST[0])->interface == "" );
    REQUIRE( ((ClassDefinition*)AST[0])->body.size() == 2 );
    REQUIRE( ((ClassDefinition*)AST[0])->body[0]->type == "DeclareVariable" );
    REQUIRE( ((DeclareVariable*)((ClassDefinition*)AST[0])->body[0])->variable_type.name == "int" );
    REQUIRE( ((DeclareVariable*)((ClassDefinition*)AST[0])->body[0])->name == "test" );

    REQUIRE( AST[1]->type == "ClassDefinition" );
    REQUIRE( ((ClassDefinition*)AST[1])->name == "foo" );
    REQUIRE( ((ClassDefinition*)AST[1])->interface == "bar" );
    REQUIRE( ((ClassDefinition*)AST[1])->body.size() == 2 );
    REQUIRE( ((ClassDefinition*)AST[1])->body[0]->type == "DeclareVariable" );
    REQUIRE( ((DeclareVariable*)((ClassDefinition*)AST[1])->body[0])->variable_type.name == "int" );
    REQUIRE( ((DeclareVariable*)((ClassDefinition*)AST[1])->body[0])->name == "test" );

    text = "class";

    try
    {
        AnalyseSyntax(Tokenise(text));

        FAIL();
    }
    catch (Node *node)
    {
        REQUIRE( node->error->type == SyntaxError );
        REQUIRE( node->error->text == "Missing end of statement" );
    }

    text = "class 0";

    try
    {
        AnalyseSyntax(Tokenise(text));

        FAIL();
    }
    catch (Node *node)
    {
        REQUIRE( node->error->type == SyntaxError );
        REQUIRE( node->error->text == "Invalid character in class definition" );
    }

    text = "class foo";

    try
    {
        AnalyseSyntax(Tokenise(text));

        FAIL();
    }
    catch (Node *node)
    {
        REQUIRE( node->error->type == SyntaxError );
        REQUIRE( node->error->text == "Missing end of statement" );
    }

    text = "class foo :";

    try
    {
        AnalyseSyntax(Tokenise(text));

        FAIL();
    }
    catch (Node *node)
    {
        REQUIRE( node->error->type == SyntaxError );
        REQUIRE( node->error->text == "Missing end of statement" );
    }

    text = "class foo : 0";

    try
    {
        AnalyseSyntax(Tokenise(text));

        FAIL();
    }
    catch (Node *node)
    {
        REQUIRE( node->error->type == SyntaxError );
        REQUIRE( node->error->text == "Invalid character in class definition" );
    }

    text = "class foo : bar";

    try
    {
        AnalyseSyntax(Tokenise(text));

        FAIL();
    }
    catch (Node *node)
    {
        REQUIRE( node->error->type == SyntaxError );
        REQUIRE( node->error->text == "Missing end of statement" );
    }

    text = "class foo : bar 0";

    try
    {
        AnalyseSyntax(Tokenise(text));

        FAIL();
    }
    catch (Node *node)
    {
        REQUIRE( node->error->type == SyntaxError );
        REQUIRE( node->error->text == "Invalid character in class definition" );
    }

    text = "class foo : bar {";

    try
    {
        AnalyseSyntax(Tokenise(text));

        FAIL();
    }
    catch (Node *node)
    {
        REQUIRE( node->error->type == SyntaxError );
        REQUIRE( node->error->text == "Missing end of statement" );
    }

    text = "class foo : bar { test";

    try
    {
        AnalyseSyntax(Tokenise(text));

        FAIL();
    }
    catch (Node *node)
    {
        REQUIRE( node->error->type == SyntaxError );
        REQUIRE( node->error->text == "Missing ending }" );
    }

    text = "class foo : bar { a + b; }";

    try
    {
        AnalyseSyntax(Tokenise(text));

        FAIL();
    }
    catch (Node *node)
    {
        REQUIRE( node->error->type == SyntaxError );
        REQUIRE( node->error->text == "Expected declaration" );
    }
}

TEST_CASE("Test Syntax Analyser Member Access")
{
    string text = "foo.bar; foo.bar(); foo.bar.foobar;";
    vector<Node*> AST = get<0>(AnalyseSyntax(Tokenise(text)));

    REQUIRE( AST[0]->type == "MemberAccess" );
    REQUIRE( ((MemberAccess*)AST[0])->name == "foo" );
    REQUIRE( ((MemberAccess*)AST[0])->statement->type == "GetVariable" );
    REQUIRE( ((GetVariable*)((MemberAccess*)AST[0])->statement)->name == "bar" );

    REQUIRE( AST[2]->type == "MemberAccess" );
    REQUIRE( ((MemberAccess*)AST[2])->name == "foo" );
    REQUIRE( ((MemberAccess*)AST[2])->statement->type == "FunctionCall" );
    REQUIRE( ((FunctionCall*)((MemberAccess*)AST[2])->statement)->name == "bar" );
    REQUIRE( ((FunctionCall*)((MemberAccess*)AST[2])->statement)->arguments.size() == 0 );

    REQUIRE( AST[4]->type == "MemberAccess" );
    REQUIRE( ((MemberAccess*)AST[4])->name == "foo" );
    REQUIRE( ((MemberAccess*)AST[4])->statement->type == "MemberAccess" );
    REQUIRE( ((MemberAccess*)((MemberAccess*)AST[4])->statement)->name == "bar" );
    REQUIRE( ((MemberAccess*)((MemberAccess*)AST[4])->statement)->statement->type == "GetVariable" );
    REQUIRE( ((GetVariable*)((MemberAccess*)((MemberAccess*)AST[4])->statement)->statement)->name == "foobar" );

    text = "foo.";

    try
    {
        AnalyseSyntax(Tokenise(text));

        FAIL();
    }
    catch (Node *node)
    {
        REQUIRE( node->error->type == SyntaxError );
        REQUIRE( node->error->text == "Missing end of statement" );
    }

    text = "foo.";

    try
    {
        AnalyseSyntax(Tokenise(text));

        FAIL();
    }
    catch (Node *node)
    {
        REQUIRE( node->error->type == SyntaxError );
        REQUIRE( node->error->text == "Missing end of statement" );
    }

    text = "foo.test";

    try
    {
        AnalyseSyntax(Tokenise(text));

        FAIL();
    }
    catch (Node *node)
    {
        REQUIRE( node->error->type == SyntaxError );
        REQUIRE( node->error->text == "Missing ending ;" );
    }

    text = "foo.;";

    try
    {
        AnalyseSyntax(Tokenise(text));

        FAIL();
    }
    catch (Node *node)
    {
        REQUIRE( node->error->type == SyntaxError );
        REQUIRE( node->error->text == "Missing member access" );
    }

    text = "foo.test 0;";

    try
    {
        AnalyseSyntax(Tokenise(text));

        FAIL();
    }
    catch (Node *node)
    {
        REQUIRE( node->error->type == SyntaxError );
        REQUIRE( node->error->text == "Too many expressions" );
    }

    text = "foo.if (true) {};";

    try
    {
        AnalyseSyntax(Tokenise(text));

        FAIL();
    }
    catch (Node *node)
    {
        REQUIRE( node->error->type == SyntaxError );
        REQUIRE( node->error->text == "Invalid expression in member access" );
    }
}

TEST_CASE("Test Syntax Analyser If Statements")
{
    string text = "if (exp) {}";
    vector<Node*> AST = get<0>(AnalyseSyntax(Tokenise(text)));

    REQUIRE( AST[0]->type == "IfStatement" );
    REQUIRE( ((IfStatement*)AST[0])->if_expression->type == "GetVariable" );
    REQUIRE( ((GetVariable*)((IfStatement*)AST[0])->if_expression)->name == "exp" );
    REQUIRE( ((IfStatement*)AST[0])->if_code_block->content.size() == 0 );
    REQUIRE( ((IfStatement*)AST[0])->else_if_expressions.size() == 0 );
    REQUIRE( ((IfStatement*)AST[0])->else_if_code_blocks.size() == 0 );
    REQUIRE( ((IfStatement*)AST[0])->else_code_block == NULL );

    text = "if (exp) {} if (exp) {} else {} if (exp1) {} else if (exp2) {} if (exp1) {} else if (exp2) {} else if (exp3) {} else {}";
    AST = get<0>(AnalyseSyntax(Tokenise(text)));

    REQUIRE( AST[0]->type == "IfStatement" );
    REQUIRE( ((IfStatement*)AST[0])->if_expression->type == "GetVariable" );
    REQUIRE( ((GetVariable*)((IfStatement*)AST[0])->if_expression)->name == "exp" );
    REQUIRE( ((IfStatement*)AST[0])->if_code_block->content.size() == 0 );
    REQUIRE( ((IfStatement*)AST[0])->else_if_expressions.size() == 0 );
    REQUIRE( ((IfStatement*)AST[0])->else_if_code_blocks.size() == 0 );
    REQUIRE( ((IfStatement*)AST[0])->else_code_block == NULL );

    REQUIRE( AST[1]->type == "IfStatement" );
    REQUIRE( ((IfStatement*)AST[1])->if_expression->type == "GetVariable" );
    REQUIRE( ((GetVariable*)((IfStatement*)AST[1])->if_expression)->name == "exp" );
    REQUIRE( ((IfStatement*)AST[1])->if_code_block->content.size() == 0 );
    REQUIRE( ((IfStatement*)AST[1])->else_if_expressions.size() == 0 );
    REQUIRE( ((IfStatement*)AST[1])->else_if_code_blocks.size() == 0 );
    REQUIRE( ((IfStatement*)AST[1])->else_code_block->content.size() == 0 );

    REQUIRE( AST[2]->type == "IfStatement" );
    REQUIRE( ((IfStatement*)AST[2])->if_expression->type == "GetVariable" );
    REQUIRE( ((GetVariable*)((IfStatement*)AST[2])->if_expression)->name == "exp1" );
    REQUIRE( ((IfStatement*)AST[2])->if_code_block->content.size() == 0 );
    REQUIRE( ((IfStatement*)AST[2])->else_if_expressions.size() == 1 );
    REQUIRE( ((IfStatement*)AST[2])->else_if_expressions[0]->type == "GetVariable" );
    REQUIRE( ((GetVariable*)((IfStatement*)AST[2])->else_if_expressions[0])->name == "exp2" );
    REQUIRE( ((IfStatement*)AST[2])->else_if_code_blocks.size() == 1 );
    REQUIRE( ((IfStatement*)AST[2])->else_if_code_blocks[0]->content.size() == 0 );
    REQUIRE( ((IfStatement*)AST[0])->else_code_block == NULL );

    REQUIRE( AST[3]->type == "IfStatement" );
    REQUIRE( ((IfStatement*)AST[3])->if_expression->type == "GetVariable" );
    REQUIRE( ((GetVariable*)((IfStatement*)AST[3])->if_expression)->name == "exp1" );
    REQUIRE( ((IfStatement*)AST[3])->if_code_block->content.size() == 0 );
    REQUIRE( ((IfStatement*)AST[3])->else_if_expressions.size() == 2 );
    REQUIRE( ((IfStatement*)AST[3])->else_if_expressions[0]->type == "GetVariable" );
    REQUIRE( ((GetVariable*)((IfStatement*)AST[3])->else_if_expressions[0])->name == "exp2" );
    REQUIRE( ((IfStatement*)AST[3])->else_if_expressions[1]->type == "GetVariable" );
    REQUIRE( ((GetVariable*)((IfStatement*)AST[3])->else_if_expressions[1])->name == "exp3" );
    REQUIRE( ((IfStatement*)AST[3])->else_if_code_blocks.size() == 2 );
    REQUIRE( ((IfStatement*)AST[3])->else_if_code_blocks[0]->content.size() == 0 );
    REQUIRE( ((IfStatement*)AST[3])->else_if_code_blocks[1]->content.size() == 0 );
    REQUIRE( ((IfStatement*)AST[1])->else_code_block->content.size() == 0 );

    text = "if";
    
    try
    {
        AnalyseSyntax(Tokenise(text));

        FAIL();
    }
    catch (Node *node)
    {
        REQUIRE( node->error->type == SyntaxError );
        REQUIRE( node->error->text == "Missing end of statement" );
    }

    text = "if test";
    
    try
    {
        AnalyseSyntax(Tokenise(text));

        FAIL();
    }
    catch (Node *node)
    {
        REQUIRE( node->error->type == SyntaxError );
        REQUIRE( node->error->text == "Missing starting (" );
    }

    text = "if (";
    
    try
    {
        AnalyseSyntax(Tokenise(text));

        FAIL();
    }
    catch (Node *node)
    {
        REQUIRE( node->error->type == SyntaxError );
        REQUIRE( node->error->text == "Missing end of statement" );
    }

    text = "if (test";
    
    try
    {
        AnalyseSyntax(Tokenise(text));

        FAIL();
    }
    catch (Node *node)
    {
        REQUIRE( node->error->type == SyntaxError );
        REQUIRE( node->error->text == "Missing ending )" );
    }

    text = "if ()";
    
    try
    {
        AnalyseSyntax(Tokenise(text));

        FAIL();
    }
    catch (Node *node)
    {
        REQUIRE( node->error->type == SyntaxError );
        REQUIRE( node->error->text == "Missing expression" );
    }

    text = "if (true)";
    
    try
    {
        AnalyseSyntax(Tokenise(text));

        FAIL();
    }
    catch (Node *node)
    {
        REQUIRE( node->error->type == SyntaxError );
        REQUIRE( node->error->text == "Missing end of statement" );
    }

    text = "if (true) test";
    
    try
    {
        AnalyseSyntax(Tokenise(text));

        FAIL();
    }
    catch (Node *node)
    {
        REQUIRE( node->error->type == SyntaxError );
        REQUIRE( node->error->text == "Invalid character in if statement" );
    }

    text = "if (true) {} else if";
    
    try
    {
        AnalyseSyntax(Tokenise(text));

        FAIL();
    }
    catch (Node *node)
    {
        REQUIRE( node->error->type == SyntaxError );
        REQUIRE( node->error->text == "Missing end of statement" );
    }

    text = "if (true) {} else if test";
    
    try
    {
        AnalyseSyntax(Tokenise(text));

        FAIL();
    }
    catch (Node *node)
    {
        REQUIRE( node->error->type == SyntaxError );
        REQUIRE( node->error->text == "Missing starting (" );
    }

    text = "if (true) {} else if (";
    
    try
    {
        AnalyseSyntax(Tokenise(text));

        FAIL();
    }
    catch (Node *node)
    {
        REQUIRE( node->error->type == SyntaxError );
        REQUIRE( node->error->text == "Missing end of statement" );
    }

    text = "if (true) {} else if (test";
    
    try
    {
        AnalyseSyntax(Tokenise(text));

        FAIL();
    }
    catch (Node *node)
    {
        REQUIRE( node->error->type == SyntaxError );
        REQUIRE( node->error->text == "Missing ending )" );
    }

    text = "if (true) {} else if ()";
    
    try
    {
        AnalyseSyntax(Tokenise(text));

        FAIL();
    }
    catch (Node *node)
    {
        REQUIRE( node->error->type == SyntaxError );
        REQUIRE( node->error->text == "Missing expression" );
    }

    text = "if (true) {} else if (true)";
    
    try
    {
        AnalyseSyntax(Tokenise(text));

        FAIL();
    }
    catch (Node *node)
    {
        REQUIRE( node->error->type == SyntaxError );
        REQUIRE( node->error->text == "Missing end of statement" );
    }

    text = "if (true) {} else if (true) test";
    
    try
    {
        AnalyseSyntax(Tokenise(text));

        FAIL();
    }
    catch (Node *node)
    {
        REQUIRE( node->error->type == SyntaxError );
        REQUIRE( node->error->text == "Invalid character in if statement" );
    }

    text = "if (true) {} else if (true) {} else";
    
    try
    {
        AnalyseSyntax(Tokenise(text));

        FAIL();
    }
    catch (Node *node)
    {
        REQUIRE( node->error->type == SyntaxError );
        REQUIRE( node->error->text == "Missing end of statement" );
    }

    text = "if (true) {} else if (true) {} else test";
    
    try
    {
        AnalyseSyntax(Tokenise(text));

        FAIL();
    }
    catch (Node *node)
    {
        REQUIRE( node->error->type == SyntaxError );
        REQUIRE( node->error->text == "Invalid character in if statement" );
    }
}

TEST_CASE("Test Syntax Analyser Switch Statement")
{
    string text = "switch (exp) {} switch (exp1) { case (exp2) {} } switch (exp1) { case (exp2) {} case (exp3) {} default {} }";
    vector<Node*> AST = get<0>(AnalyseSyntax(Tokenise(text)));

    REQUIRE( AST[0]->type == "SwitchStatement" );
    REQUIRE( ((SwitchStatement*)AST[0])->switch_expression->type == "GetVariable" );
    REQUIRE( ((GetVariable*)((SwitchStatement*)AST[0])->switch_expression)->name == "exp" );
    REQUIRE( ((SwitchStatement*)AST[0])->case_expressions.size() == 0 );
    REQUIRE( ((SwitchStatement*)AST[0])->case_code_blocks.size() == 0 );
    REQUIRE( ((SwitchStatement*)AST[0])->default_code_block == NULL );

    REQUIRE( AST[1]->type == "SwitchStatement" );
    REQUIRE( ((SwitchStatement*)AST[1])->switch_expression->type == "GetVariable" );
    REQUIRE( ((GetVariable*)((SwitchStatement*)AST[1])->switch_expression)->name == "exp1" );
    REQUIRE( ((SwitchStatement*)AST[1])->case_expressions.size() == 1 );
    REQUIRE( ((SwitchStatement*)AST[1])->case_expressions[0]->type == "GetVariable" );
    REQUIRE( ((GetVariable*)((SwitchStatement*)AST[1])->case_expressions[0])->name == "exp2" );
    REQUIRE( ((SwitchStatement*)AST[1])->case_code_blocks.size() == 1 );
    REQUIRE( ((SwitchStatement*)AST[1])->case_code_blocks[0]->content.size() == 0 );
    REQUIRE( ((SwitchStatement*)AST[1])->default_code_block == NULL );

    REQUIRE( AST[2]->type == "SwitchStatement" );
    REQUIRE( ((SwitchStatement*)AST[2])->switch_expression->type == "GetVariable" );
    REQUIRE( ((GetVariable*)((SwitchStatement*)AST[2])->switch_expression)->name == "exp1" );
    REQUIRE( ((SwitchStatement*)AST[2])->case_expressions.size() == 2 );
    REQUIRE( ((SwitchStatement*)AST[2])->case_expressions[0]->type == "GetVariable" );
    REQUIRE( ((GetVariable*)((SwitchStatement*)AST[2])->case_expressions[0])->name == "exp2" );
    REQUIRE( ((SwitchStatement*)AST[2])->case_expressions[1]->type == "GetVariable" );
    REQUIRE( ((GetVariable*)((SwitchStatement*)AST[2])->case_expressions[1])->name == "exp3" );
    REQUIRE( ((SwitchStatement*)AST[2])->case_code_blocks.size() == 2 );
    REQUIRE( ((SwitchStatement*)AST[2])->case_code_blocks[0]->content.size() == 0 );
    REQUIRE( ((SwitchStatement*)AST[2])->case_code_blocks[1]->content.size() == 0 );
    REQUIRE( ((SwitchStatement*)AST[2])->default_code_block->content.size() == 0 );

    text = "switch";

    try
    {
        AnalyseSyntax(Tokenise(text));

        FAIL();
    }
    catch (Node *node)
    {
        REQUIRE( node->error->type == SyntaxError );
        REQUIRE( node->error->text == "Missing end of statement" );
    }

    text = "switch test";

    try
    {
        AnalyseSyntax(Tokenise(text));

        FAIL();
    }
    catch (Node *node)
    {
        REQUIRE( node->error->type == SyntaxError );
        REQUIRE( node->error->text == "Missing starting (" );
    }

    text = "switch (";

    try
    {
        AnalyseSyntax(Tokenise(text));

        FAIL();
    }
    catch (Node *node)
    {
        REQUIRE( node->error->type == SyntaxError );
        REQUIRE( node->error->text == "Missing end of statement" );
    }

    text = "switch (test";

    try
    {
        AnalyseSyntax(Tokenise(text));

        FAIL();
    }
    catch (Node *node)
    {
        REQUIRE( node->error->type == SyntaxError );
        REQUIRE( node->error->text == "Missing ending )" );
    }

    text = "switch ()";

    try
    {
        AnalyseSyntax(Tokenise(text));

        FAIL();
    }
    catch (Node *node)
    {
        REQUIRE( node->error->type == SyntaxError );
        REQUIRE( node->error->text == "Missing expression" );
    }

    text = "switch (foo)";

    try
    {
        AnalyseSyntax(Tokenise(text));

        FAIL();
    }
    catch (Node *node)
    {
        REQUIRE( node->error->type == SyntaxError );
        REQUIRE( node->error->text == "Missing end of statement" );
    }

    text = "switch (foo) test";

    try
    {
        AnalyseSyntax(Tokenise(text));

        FAIL();
    }
    catch (Node *node)
    {
        REQUIRE( node->error->type == SyntaxError );
        REQUIRE( node->error->text == "Missing starting {" );
    }

    text = "switch (foo) { test";

    try
    {
        AnalyseSyntax(Tokenise(text));

        FAIL();
    }
    catch (Node *node)
    {
        REQUIRE( node->error->type == SyntaxError );
        REQUIRE( node->error->text == "Invalid character in switch statement" );
    }

    text = "switch (foo) { case";

    try
    {
        AnalyseSyntax(Tokenise(text));

        FAIL();
    }
    catch (Node *node)
    {
        REQUIRE( node->error->type == SyntaxError );
        REQUIRE( node->error->text == "Missing end of statement" );
    }

    text = "switch (foo) { case test";

    try
    {
        AnalyseSyntax(Tokenise(text));

        FAIL();
    }
    catch (Node *node)
    {
        REQUIRE( node->error->type == SyntaxError );
        REQUIRE( node->error->text == "Missing starting (" );
    }

    text = "switch (foo) { case (";

    try
    {
        AnalyseSyntax(Tokenise(text));

        FAIL();
    }
    catch (Node *node)
    {
        REQUIRE( node->error->type == SyntaxError );
        REQUIRE( node->error->text == "Missing end of statement" );
    }

    text = "switch (foo) { case (test";

    try
    {
        AnalyseSyntax(Tokenise(text));

        FAIL();
    }
    catch (Node *node)
    {
        REQUIRE( node->error->type == SyntaxError );
        REQUIRE( node->error->text == "Missing ending )" );
    }

    text = "switch (foo) { case ()";

    try
    {
        AnalyseSyntax(Tokenise(text));

        FAIL();
    }
    catch (Node *node)
    {
        REQUIRE( node->error->type == SyntaxError );
        REQUIRE( node->error->text == "Missing expression" );
    }

    text = "switch (foo) { case (bar)";

    try
    {
        AnalyseSyntax(Tokenise(text));

        FAIL();
    }
    catch (Node *node)
    {
        REQUIRE( node->error->type == SyntaxError );
        REQUIRE( node->error->text == "Missing end of statement" );
    }

    text = "switch (foo) { case (bar) test";

    try
    {
        AnalyseSyntax(Tokenise(text));

        FAIL();
    }
    catch (Node *node)
    {
        REQUIRE( node->error->type == SyntaxError );
        REQUIRE( node->error->text == "Missing ending }" );
    }

    text = "switch (foo) { case (bar) test;";

    try
    {
        AnalyseSyntax(Tokenise(text));

        FAIL();
    }
    catch (Node *node)
    {
        REQUIRE( node->error->type == SyntaxError );
        REQUIRE( node->error->text == "Invalid character in switch statement" );
    }

    text = "switch (foo) { case (bar) {}";

    try
    {
        AnalyseSyntax(Tokenise(text));

        FAIL();
    }
    catch (Node *node)
    {
        REQUIRE( node->error->type == SyntaxError );
        REQUIRE( node->error->text == "Missing ending }" );
    }

    text = "switch (foo) { default";

    try
    {
        AnalyseSyntax(Tokenise(text));

        FAIL();
    }
    catch (Node *node)
    {
        REQUIRE( node->error->type == SyntaxError );
        REQUIRE( node->error->text == "Missing end of statement" );
    }

    text = "switch (foo) { default test";

    try
    {
        AnalyseSyntax(Tokenise(text));

        FAIL();
    }
    catch (Node *node)
    {
        REQUIRE( node->error->type == SyntaxError );
        REQUIRE( node->error->text == "Missing ending }" );
    }

    text = "switch (foo) { default test;";

    try
    {
        AnalyseSyntax(Tokenise(text));

        FAIL();
    }
    catch (Node *node)
    {
        REQUIRE( node->error->type == SyntaxError );
        REQUIRE( node->error->text == "Invalid character in switch statement" );
    }

    text = "switch (foo) { default {}";

    try
    {
        AnalyseSyntax(Tokenise(text));

        FAIL();
    }
    catch (Node *node)
    {
        REQUIRE( node->error->type == SyntaxError );
        REQUIRE( node->error->text == "Missing ending }" );
    }
}

TEST_CASE("Test Syntax Analyser For Loop")
{
    string text = "for (;;) {} for (int i = 0;;) {} for (int i = 0; i < length;) {} for (int i = 0; i < length; i += 1) {}";
    vector<Node*> AST = get<0>(AnalyseSyntax(Tokenise(text)));

    REQUIRE( AST[0]->type == "ForLoop" );
    REQUIRE( ((ForLoop*)AST[0])->declaration_expression == NULL );
    REQUIRE( ((ForLoop*)AST[0])->condition_expression == NULL );
    REQUIRE( ((ForLoop*)AST[0])->iteration_expression == NULL );
    REQUIRE( ((ForLoop*)AST[0])->for_code_block->content.size() == 0 );

    REQUIRE( AST[1]->type == "ForLoop" );
    REQUIRE( ((ForLoop*)AST[1])->declaration_expression->type == "DeclareVariable" );
    REQUIRE( ((DeclareVariable*)((ForLoop*)AST[1])->declaration_expression)->variable_type.name == "int" );
    REQUIRE( ((DeclareVariable*)((ForLoop*)AST[1])->declaration_expression)->name == "i" );
    REQUIRE( ((DeclareVariable*)((ForLoop*)AST[1])->declaration_expression)->value->type == "Literal" );
    REQUIRE( ((Literal*)((DeclareVariable*)((ForLoop*)AST[1])->declaration_expression)->value)->l_integer == 0 );
    REQUIRE( ((ForLoop*)AST[1])->condition_expression == NULL );
    REQUIRE( ((ForLoop*)AST[1])->iteration_expression == NULL );
    REQUIRE( ((ForLoop*)AST[1])->for_code_block->content.size() == 0 );

    REQUIRE( AST[2]->type == "ForLoop" );
    REQUIRE( ((ForLoop*)AST[2])->declaration_expression->type == "DeclareVariable" );
    REQUIRE( ((DeclareVariable*)((ForLoop*)AST[2])->declaration_expression)->variable_type.name == "int" );
    REQUIRE( ((DeclareVariable*)((ForLoop*)AST[2])->declaration_expression)->name == "i" );
    REQUIRE( ((DeclareVariable*)((ForLoop*)AST[2])->declaration_expression)->value->type == "Literal" );
    REQUIRE( ((Literal*)((DeclareVariable*)((ForLoop*)AST[2])->declaration_expression)->value)->l_integer == 0 );
    REQUIRE( ((ForLoop*)AST[2])->condition_expression->type == "Operation" );
    REQUIRE( ((Operation*)((ForLoop*)AST[2])->condition_expression)->operator_string == "<" );
    REQUIRE( ((Operation*)((ForLoop*)AST[2])->condition_expression)->left->type == "GetVariable" );
    REQUIRE( ((GetVariable*)((Operation*)((ForLoop*)AST[2])->condition_expression)->left)->name == "i" );
    REQUIRE( ((Operation*)((ForLoop*)AST[2])->condition_expression)->right->type == "GetVariable" );
    REQUIRE( ((GetVariable*)((Operation*)((ForLoop*)AST[2])->condition_expression)->right)->name == "length" );
    REQUIRE( ((ForLoop*)AST[2])->iteration_expression == NULL );
    REQUIRE( ((ForLoop*)AST[2])->for_code_block->content.size() == 0 );

    REQUIRE( AST[3]->type == "ForLoop" );
    REQUIRE( ((ForLoop*)AST[3])->declaration_expression->type == "DeclareVariable" );
    REQUIRE( ((DeclareVariable*)((ForLoop*)AST[3])->declaration_expression)->variable_type.name == "int" );
    REQUIRE( ((DeclareVariable*)((ForLoop*)AST[3])->declaration_expression)->name == "i" );
    REQUIRE( ((DeclareVariable*)((ForLoop*)AST[3])->declaration_expression)->value->type == "Literal" );
    REQUIRE( ((Literal*)((DeclareVariable*)((ForLoop*)AST[3])->declaration_expression)->value)->l_integer == 0 );
    REQUIRE( ((ForLoop*)AST[3])->condition_expression->type == "Operation" );
    REQUIRE( ((Operation*)((ForLoop*)AST[3])->condition_expression)->operator_string == "<" );
    REQUIRE( ((Operation*)((ForLoop*)AST[3])->condition_expression)->left->type == "GetVariable" );
    REQUIRE( ((GetVariable*)((Operation*)((ForLoop*)AST[3])->condition_expression)->left)->name == "i" );
    REQUIRE( ((Operation*)((ForLoop*)AST[3])->condition_expression)->right->type == "GetVariable" );
    REQUIRE( ((GetVariable*)((Operation*)((ForLoop*)AST[3])->condition_expression)->right)->name == "length" );
    REQUIRE( ((ForLoop*)AST[3])->iteration_expression->type == "Operation" );
    REQUIRE( ((Operation*)((ForLoop*)AST[3])->iteration_expression)->operator_string == "+=" );
    REQUIRE( ((Operation*)((ForLoop*)AST[3])->iteration_expression)->left->type == "GetVariable" );
    REQUIRE( ((GetVariable*)((Operation*)((ForLoop*)AST[3])->iteration_expression)->left)->name == "i" );
    REQUIRE( ((Operation*)((ForLoop*)AST[3])->iteration_expression)->right->type == "Literal" );
    REQUIRE( ((Literal*)((Operation*)((ForLoop*)AST[3])->iteration_expression)->right)->l_integer == 1 );
    REQUIRE( ((ForLoop*)AST[3])->for_code_block->content.size() == 0 );

    text = "for";

    try {
        AnalyseSyntax(Tokenise(text));
    }
    catch (Node *node)
    {
        REQUIRE( node->error->type == SyntaxError );
        REQUIRE( node->error->text == "Missing end of statement" );
    }

    text = "for test";

    try {
        AnalyseSyntax(Tokenise(text));
    }
    catch (Node *node)
    {
        REQUIRE( node->error->type == SyntaxError );
        REQUIRE( node->error->text == "Missing starting (" );
    }

    text = "for (test";

    try {
        AnalyseSyntax(Tokenise(text));
    }
    catch (Node *node)
    {
        REQUIRE( node->error->type == SyntaxError );
        REQUIRE( node->error->text == "Missing ending )" );
    }

    text = "for (1 0)";

    try {
        AnalyseSyntax(Tokenise(text));
    }
    catch (Node *node)
    {
        REQUIRE( node->error->type == SyntaxError );
        REQUIRE( node->error->text == "Missing ;" );
    }

    text = "for (a; b)";

    try {
        AnalyseSyntax(Tokenise(text));
    }
    catch (Node *node)
    {
        REQUIRE( node->error->type == SyntaxError );
        REQUIRE( node->error->text == "Missing expression(s)" );
    }

    text = "for (a; b; c; d)";

    try {
        AnalyseSyntax(Tokenise(text));
    }
    catch (Node *node)
    {
        REQUIRE( node->error->type == SyntaxError );
        REQUIRE( node->error->text == "Too many expressions" );
    }

    text = "for (a; b; c)";

    try {
        AnalyseSyntax(Tokenise(text));
    }
    catch (Node *node)
    {
        REQUIRE( node->error->type == SyntaxError );
        REQUIRE( node->error->text == "Missing end of statement" );
    }

    text = "for (a; b; c) test";

    try {
        AnalyseSyntax(Tokenise(text));
    }
    catch (Node *node)
    {
        REQUIRE( node->error->type == SyntaxError );
        REQUIRE( node->error->text == "Invalid character in for loop" );
    }
}

TEST_CASE("Test Syntax Analyser Foreach Loop")
{
    string text = "for (int item : list) {}";
    vector<Node*> AST = get<0>(AnalyseSyntax(Tokenise(text)));

    REQUIRE( AST[0]->type == "ForEachLoop" );
    REQUIRE( ((ForEachLoop*)AST[0])->declaration_expression->type == "DeclareVariable" );
    REQUIRE( ((DeclareVariable*)((ForEachLoop*)AST[0])->declaration_expression)->variable_type.name == "int" );
    REQUIRE( ((DeclareVariable*)((ForEachLoop*)AST[0])->declaration_expression)->name == "item" );
    REQUIRE( ((ForEachLoop*)AST[0])->iteration_expression->type == "GetVariable" );
    REQUIRE( ((GetVariable*)((ForEachLoop*)AST[0])->iteration_expression)->name == "list" );
    REQUIRE( ((ForEachLoop*)AST[0])->for_code_block->content.size() == 0 );

    text = "for";

    try {
        AnalyseSyntax(Tokenise(text));
    }
    catch (Node *node)
    {
        REQUIRE( node->error->type == SyntaxError );
        REQUIRE( node->error->text == "Missing end of statement" );
    }

    text = "for test";

    try {
        AnalyseSyntax(Tokenise(text));
    }
    catch (Node *node)
    {
        REQUIRE( node->error->type == SyntaxError );
        REQUIRE( node->error->text == "Missing starting (" );
    }

    text = "for (test";

    try {
        AnalyseSyntax(Tokenise(text));
    }
    catch (Node *node)
    {
        REQUIRE( node->error->type == SyntaxError );
        REQUIRE( node->error->text == "Missing ending )" );
    }

    text = "for (test :";

    try {
        AnalyseSyntax(Tokenise(text));
    }
    catch (Node *node)
    {
        REQUIRE( node->error->type == SyntaxError );
        REQUIRE( node->error->text == "Invalid expression in for loop" );
    }

    text = "for (int item :";

    try {
        AnalyseSyntax(Tokenise(text));
    }
    catch (Node *node)
    {
        REQUIRE( node->error->type == SyntaxError );
        REQUIRE( node->error->text == "Missing end of statement" );
    }

    text = "for (int item : list";

    try {
        AnalyseSyntax(Tokenise(text));
    }
    catch (Node *node)
    {
        REQUIRE( node->error->type == SyntaxError );
        REQUIRE( node->error->text == "Missing ending )" );
    }

    text = "for (int item :)";

    try {
        AnalyseSyntax(Tokenise(text));
    }
    catch (Node *node)
    {
        REQUIRE( node->error->type == SyntaxError );
        REQUIRE( node->error->text == "Missing iteration expression" );
    }

    text = "for (int item : list)";

    try {
        AnalyseSyntax(Tokenise(text));
    }
    catch (Node *node)
    {
        REQUIRE( node->error->type == SyntaxError );
        REQUIRE( node->error->text == "Missing end of statement" );
    }

    text = "for (int item : list) test";

    try {
        AnalyseSyntax(Tokenise(text));
    }
    catch (Node *node)
    {
        REQUIRE( node->error->type == SyntaxError );
        REQUIRE( node->error->text == "Invalid character in for loop" );
    }
}

TEST_CASE("Test Syntax Analyser While Loop")
{
    string text = "while (true) {}";
    vector<Node*> AST = get<0>(AnalyseSyntax(Tokenise(text)));

    REQUIRE( AST[0]->type == "WhileLoop" );
    REQUIRE( ((WhileLoop*)AST[0])->condition->type == "Literal" );
    REQUIRE( ((Literal*)((WhileLoop*)AST[0])->condition)->l_boolean == true );
    REQUIRE( ((WhileLoop*)AST[0])->while_code_block->content.size() == 0 );

    text = "while";

    try {
        AnalyseSyntax(Tokenise(text));
    }
    catch (Node *node)
    {
        REQUIRE( node->error->type == SyntaxError );
        REQUIRE( node->error->text == "Missing end of statement" );
    }

    text = "while test";

    try {
        AnalyseSyntax(Tokenise(text));
    }
    catch (Node *node)
    {
        REQUIRE( node->error->type == SyntaxError );
        REQUIRE( node->error->text == "Missing starting (" );
    }

    text = "while (";

    try {
        AnalyseSyntax(Tokenise(text));
    }
    catch (Node *node)
    {
        REQUIRE( node->error->type == SyntaxError );
        REQUIRE( node->error->text == "Missing end of statement" );
    }

    text = "while (test";

    try {
        AnalyseSyntax(Tokenise(text));
    }
    catch (Node *node)
    {
        REQUIRE( node->error->type == SyntaxError );
        REQUIRE( node->error->text == "Missing ending )" );
    }

    text = "while ()";

    try {
        AnalyseSyntax(Tokenise(text));
    }
    catch (Node *node)
    {
        REQUIRE( node->error->type == SyntaxError );
        REQUIRE( node->error->text == "Missing expression" );
    }

    text = "while (test)";

    try {
        AnalyseSyntax(Tokenise(text));
    }
    catch (Node *node)
    {
        REQUIRE( node->error->type == SyntaxError );
        REQUIRE( node->error->text == "Missing end of statement" );
    }

    text = "while (test) foo";

    try {
        AnalyseSyntax(Tokenise(text));
    }
    catch (Node *node)
    {
        REQUIRE( node->error->type == SyntaxError );
        REQUIRE( node->error->text == "Invalid character in if statement" );
    }
}

TEST_CASE("Test Syntax Analyser Return")
{
    string text = "return foo;";
    vector<Node*> AST = get<0>(AnalyseSyntax(Tokenise(text)));

    REQUIRE( AST[0]->type == "Return" );
    REQUIRE( ((Return*)AST[0])->expression->type == "GetVariable" );
    REQUIRE( ((GetVariable*)((Return*)AST[0])->expression)->name == "foo" );
}

TEST_CASE("Test Syntax Analyser Break")
{
    string text = "break;";
    vector<Node*> AST = get<0>(AnalyseSyntax(Tokenise(text)));

    REQUIRE( AST[0]->type == "Break" );
}

TEST_CASE("Test Syntax Analyser Continue")
{
    string text = "continue;";
    vector<Node*> AST = get<0>(AnalyseSyntax(Tokenise(text)));

    REQUIRE( AST[0]->type == "Continue" );
}