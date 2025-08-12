#include "catch.hpp"

#include "../src/node.hpp"
#include "../src/lexer.hpp"
#include "../src/syntax_analyser.hpp"

using namespace std;

TEST_CASE("Test Syntax Analyser Types")
{
    string text = "foo<bar> test<a, b, c> foo[] this<is<a, good[], test<>>>";

    vector<Node*> AST = get<0>(AnalyseSyntax(Tokenise(text)));

    REQUIRE( AST[0]->type == "Type" );
    REQUIRE( ((Type*)AST[0])->name == "foo" );
    REQUIRE( ((Type*)AST[0])->content[0].name == "bar" );

    REQUIRE( AST[1]->type == "Type" );
    REQUIRE( ((Type*)AST[1])->name == "test" );
    REQUIRE( ((Type*)AST[1])->content[0].name == "a" );
    REQUIRE( ((Type*)AST[1])->content[1].name == "b" );
    REQUIRE( ((Type*)AST[1])->content[2].name == "c" );

    REQUIRE( AST[2]->type == "Type" );
    REQUIRE( ((Type*)AST[2])->name == "foo" );
    REQUIRE( ((Type*)AST[2])->is_array );

    REQUIRE( AST[3]->type == "Type" );
    REQUIRE( ((Type*)AST[3])->name == "this" );
    REQUIRE( ((Type*)AST[3])->content[0].name == "is" );
    REQUIRE( ((Type*)AST[3])->content[0].content[0].name == "a" );
    REQUIRE( ((Type*)AST[3])->content[0].content[1].name == "good" );
    REQUIRE( ((Type*)AST[3])->content[0].content[1].is_array );
    REQUIRE( ((Type*)AST[3])->content[0].content[2].name == "test" );
    REQUIRE( ((Type*)AST[3])->content[0].content[2].content.size() == 0 );
}

TEST_CASE("Test Syntax Analyser Invalid Types")
{
    string text = "list<";

    try
    {
        AnalyseSyntax(Tokenise(text));
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
    }
    catch (Node *node)
    {
        REQUIRE( node->error->type == SyntaxError );
        REQUIRE( node->error->text == "Missing end of type" );
    }
}

TEST_CASE("Test Syntax Analyser Qualifiers")
{
    string text = "public public static public static const;";
    vector<Node*> AST = get<0>(AnalyseSyntax(Tokenise(text)));

    REQUIRE( AST[0]->type == "Qualifier" );
    REQUIRE( ((Qualifier*)AST[0])->qualifiers == vector<string> { "public" } );

    REQUIRE( AST[1]->type == "Qualifier" );
    REQUIRE( ((Qualifier*)AST[1])->qualifiers == vector<string> { "public", "static" } );

    REQUIRE( AST[2]->type == "Qualifier" );
    REQUIRE( ((Qualifier*)AST[2])->qualifiers == vector<string> { "public", "static", "const" } );

    text = "const static public;";
    AST = get<0>(AnalyseSyntax(Tokenise(text)));

    REQUIRE( AST[0]->type == "Qualifier" );
    REQUIRE_FALSE( ((Qualifier*)AST[0])->qualifiers == vector<string> { "const", "static", "public" } ); // Qualifiers must be written in the correct order

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
    REQUIRE( !((CodeBlock*)AST[0])->return_type );
    REQUIRE( ((CodeBlock*)AST[0])->parameters.size() == 0 );
    REQUIRE( ((CodeBlock*)AST[0])->content[0]->type == "Literal" );

    REQUIRE( AST[2]->type == "CodeBlock" );
    REQUIRE( !((CodeBlock*)AST[2])->return_type );
    REQUIRE( ((CodeBlock*)AST[2])->parameters.size() == 0 );
    REQUIRE( ((CodeBlock*)AST[2])->content[0]->type == "Literal" );
    REQUIRE( ((Literal*)((CodeBlock*)AST[2])->content[0])->l_string == "foo" );

    REQUIRE( AST[4]->type == "CodeBlock" );
    REQUIRE( !((CodeBlock*)AST[4])->return_type );
    REQUIRE( ((CodeBlock*)AST[4])->parameters[0].type_data.name == "int" );
    REQUIRE( ((CodeBlock*)AST[4])->parameters[0].name == "foo" );
    REQUIRE( ((CodeBlock*)AST[4])->content[0]->type == "Literal" );
    REQUIRE( ((Literal*)((CodeBlock*)AST[4])->content[0])->l_string == "bar" );

    REQUIRE( AST[6]->type == "CodeBlock" );
    REQUIRE( ((CodeBlock*)AST[6])->return_type.value().name == "string" );
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
    }
    catch (Node *node)
    {
        REQUIRE( node->error->type == SyntaxError );
        REQUIRE( node->error->text == "Missing ending }" );
    }
}

TEST_CASE("Test Syntax Analyser Operation") // All needs to change for order of operations -a ^ b * c + d == e & f | g = h;
{

    string text = "a ^ b; -a; a * b; a + b; a == b; a & b; a | b; a = b; a * b + c; a + b * c; a + b * c == d; a == b * c + d; func(a / b);";
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
    REQUIRE( ((Operation*)((FunctionCall*)AST[24])->arguments[0])->operator_string == "/" );
    REQUIRE( (((Operation*)((FunctionCall*)AST[24])->arguments[0])->left)->type == "GetVariable" );
    REQUIRE( ((GetVariable*)(((Operation*)((FunctionCall*)AST[24])->arguments[0])->left))->name == "a" );
    REQUIRE( (((Operation*)((FunctionCall*)AST[24])->arguments[0])->right)->type == "GetVariable" );
    REQUIRE( ((GetVariable*)(((Operation*)((FunctionCall*)AST[24])->arguments[0])->right))->name == "b" );

    text = "> a";

    try
    {
        AnalyseSyntax(Tokenise(text));
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
    }
    catch (Node *node)
    {
        REQUIRE( node->error->type == SyntaxError );
        REQUIRE( node->error->text == "Missing right expression for operation" );
    }
}

TEST_CASE("Test Syntax Analyser Get Variable")
{
    string text = "foo bar";

    vector<Node*> AST = get<0>(AnalyseSyntax(Tokenise(text)));

    REQUIRE( AST[0]->type == "GetVariable" );
    REQUIRE( ((GetVariable*)AST[0])->name == "foo" );

    REQUIRE( AST[1]->type == "GetVariable" );
    REQUIRE( ((GetVariable*)AST[1])->name == "bar" );
}

TEST_CASE("Test Syntax Analyser Assign Variable")
{
    string text = "int foo = 0;";
    vector<Node*> AST = get<0>(AnalyseSyntax(Tokenise(text)));

    REQUIRE( AST[0]->type == "AssignVariable" );
    REQUIRE( ((AssignVariable*)AST[0])->variable_type.name == "int" );
    REQUIRE( ((AssignVariable*)AST[0])->name == "foo" );
    REQUIRE( ((AssignVariable*)AST[0])->value->type == "Literal" );
    REQUIRE( ((Literal*)((AssignVariable*)AST[0])->value)->l_integer == 0 );

    text = "static const int foo = 0;";
    AST = get<0>(AnalyseSyntax(Tokenise(text)));

    REQUIRE( AST[0]->type == "AssignVariable" );
    REQUIRE( ((AssignVariable*)AST[0])->qualifier->qualifiers == vector<string> { "static", "const" } );
    REQUIRE( ((AssignVariable*)AST[0])->variable_type.name == "int" );
    REQUIRE( ((AssignVariable*)AST[0])->name == "foo" );
    REQUIRE( ((AssignVariable*)AST[0])->value->type == "Literal" );
    REQUIRE( ((Literal*)((AssignVariable*)AST[0])->value)->l_integer == 0 );

    text = "int foo = 0"; // No ending semi-colon

    try
    {
        AnalyseSyntax(Tokenise(text));
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
    }
    catch (Node *node)
    {
        REQUIRE( node->error->type == SyntaxError );
        REQUIRE( node->error->text == "Missing assignment value" );
    }

    text = "int foo =";

    try
    {
        AnalyseSyntax(Tokenise(text));
    }
    catch (Node *node)
    {
        REQUIRE( node->error->type == SyntaxError );
        REQUIRE( node->error->text == "Missing end of assignment" );
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

    REQUIRE( AST[0]->type == "AssignVariable" );
    REQUIRE( ((AssignVariable*)AST[0])->variable_type.name == "int" );
    REQUIRE( ((AssignVariable*)AST[0])->name == "foo" );
    REQUIRE( ((AssignVariable*)AST[0])->value->type == "CodeBlock" );
    REQUIRE( ((CodeBlock*)((AssignVariable*)AST[0])->value)->parameters.size() == 0 );
    REQUIRE( ((CodeBlock*)((AssignVariable*)AST[0])->value)->content.size() == 0 );

    text = "public static int foo() {}";
    AST = get<0>(AnalyseSyntax(Tokenise(text)));

    AssignVariable a = *((AssignVariable*)AST[0]);

    REQUIRE( AST[0]->type == "AssignVariable" );
    REQUIRE( ((AssignVariable*)AST[0])->qualifier->qualifiers == vector<string> { "public", "static" } );
    REQUIRE( ((AssignVariable*)AST[0])->variable_type.name == "int" );
    REQUIRE( ((AssignVariable*)AST[0])->name == "foo" );
    REQUIRE( ((AssignVariable*)AST[0])->value->type == "CodeBlock" );
    REQUIRE( ((CodeBlock*)((AssignVariable*)AST[0])->value)->parameters.size() == 0 );
    REQUIRE( ((CodeBlock*)((AssignVariable*)AST[0])->value)->content.size() == 0 );

    text = "int foo() {bar}";
    AST = get<0>(AnalyseSyntax(Tokenise(text)));

    REQUIRE( AST[0]->type == "AssignVariable" );
    REQUIRE( ((AssignVariable*)AST[0])->variable_type.name == "int" );
    REQUIRE( ((AssignVariable*)AST[0])->name == "foo" );
    REQUIRE( ((AssignVariable*)AST[0])->value->type == "CodeBlock" );
    REQUIRE( ((CodeBlock*)((AssignVariable*)AST[0])->value)->parameters.size() == 0 );
    REQUIRE( ((CodeBlock*)((AssignVariable*)AST[0])->value)->content[0]->type == "GetVariable" );
    REQUIRE( ((GetVariable*)((CodeBlock*)((AssignVariable*)AST[0])->value)->content[0])->name == "bar" );

    text = "int foo(string bar) {}";
    AST = get<0>(AnalyseSyntax(Tokenise(text)));

    REQUIRE( AST[0]->type == "AssignVariable" );
    REQUIRE( ((AssignVariable*)AST[0])->variable_type.name == "int" );
    REQUIRE( ((AssignVariable*)AST[0])->name == "foo" );
    REQUIRE( ((AssignVariable*)AST[0])->value->type == "CodeBlock" );
    REQUIRE( ((CodeBlock*)((AssignVariable*)AST[0])->value)->content.size() == 0 );
    REQUIRE( ((CodeBlock*)((AssignVariable*)AST[0])->value)->parameters[0].type_data.name == "string" );
    REQUIRE( ((CodeBlock*)((AssignVariable*)AST[0])->value)->parameters[0].name == "bar" );

    text = "int foo(string bar = 'abc') {}";
    AST = get<0>(AnalyseSyntax(Tokenise(text)));

    REQUIRE( AST[0]->type == "AssignVariable" );
    REQUIRE( ((AssignVariable*)AST[0])->variable_type.name == "int" );
    REQUIRE( ((AssignVariable*)AST[0])->name == "foo" );
    REQUIRE( ((AssignVariable*)AST[0])->value->type == "CodeBlock" );
    REQUIRE( ((CodeBlock*)((AssignVariable*)AST[0])->value)->content.size() == 0 );
    REQUIRE( ((CodeBlock*)((AssignVariable*)AST[0])->value)->parameters[0].type_data.name == "string" );
    REQUIRE( ((CodeBlock*)((AssignVariable*)AST[0])->value)->parameters[0].name == "bar" );
    REQUIRE( ((CodeBlock*)((AssignVariable*)AST[0])->value)->parameters[0].default_argument.value()->type == "Literal" );
    REQUIRE( ((Literal*)((CodeBlock*)((AssignVariable*)AST[0])->value)->parameters[0].default_argument.value())->l_string == "abc" );

    text = "int foo(string **bar) {}";
    AST = get<0>(AnalyseSyntax(Tokenise(text)));

    REQUIRE( AST[0]->type == "AssignVariable" );
    REQUIRE( ((AssignVariable*)AST[0])->variable_type.name == "int" );
    REQUIRE( ((AssignVariable*)AST[0])->name == "foo" );
    REQUIRE( ((AssignVariable*)AST[0])->value->type == "CodeBlock" );
    REQUIRE( ((CodeBlock*)((AssignVariable*)AST[0])->value)->content.size() == 0 );
    REQUIRE( ((CodeBlock*)((AssignVariable*)AST[0])->value)->parameters[0].type_data.name == "string" );
    REQUIRE( ((CodeBlock*)((AssignVariable*)AST[0])->value)->parameters[0].name == "bar" );
    REQUIRE( ((CodeBlock*)((AssignVariable*)AST[0])->value)->parameters[0].argument_expansion == Dictionary );

    text = "int foo(string *bar = 'abc') {}";
    AST = get<0>(AnalyseSyntax(Tokenise(text)));

    REQUIRE( AST[0]->type == "AssignVariable" );
    REQUIRE( ((AssignVariable*)AST[0])->variable_type.name == "int" );
    REQUIRE( ((AssignVariable*)AST[0])->name == "foo" );
    REQUIRE( ((AssignVariable*)AST[0])->value->type == "CodeBlock" );
    REQUIRE( ((CodeBlock*)((AssignVariable*)AST[0])->value)->content.size() == 0 );
    REQUIRE( ((CodeBlock*)((AssignVariable*)AST[0])->value)->parameters[0].type_data.name == "string" );
    REQUIRE( ((CodeBlock*)((AssignVariable*)AST[0])->value)->parameters[0].name == "bar" );
    REQUIRE( ((CodeBlock*)((AssignVariable*)AST[0])->value)->parameters[0].default_argument.value()->type == "Literal" );
    REQUIRE( ((Literal*)((CodeBlock*)((AssignVariable*)AST[0])->value)->parameters[0].default_argument.value())->l_string == "abc" );
    REQUIRE( ((CodeBlock*)((AssignVariable*)AST[0])->value)->parameters[0].argument_expansion == Array );
}