#include "catch.hpp"

#include "../src/node.hpp"
#include "../src/lexer.hpp"
#include "../src/syntax_analyser.hpp"

using namespace std;

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

TEST_CASE("Test Syntax Analyser Types")
{
    string text = "foo<bar> test<a, b, c> this<is<a, test<>>>";

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
    REQUIRE( ((Type*)AST[2])->name == "this" );
    REQUIRE( ((Type*)AST[2])->content[0].name == "is" );
    REQUIRE( ((Type*)AST[2])->content[0].content[0].name == "a" );
    REQUIRE( ((Type*)AST[2])->content[0].content[1].name == "test" );
    REQUIRE( ((Type*)AST[2])->content[0].content[1].content.size() == 0 );
}

TEST_CASE("Test Syntax Analyser Code Block")
{
    string text = "{'foo'}; () {'foo'} (int foo) {'bar'}; (int *foo = 0, float **bar) -> string {'foobar'}";

    vector<Node*> AST = get<0>(AnalyseSyntax(Tokenise(text)));

    REQUIRE( AST[0]->type == "CodeBlock" );
    REQUIRE( !((CodeBlock*)AST[0])->return_type );
    REQUIRE( ((CodeBlock*)AST[0])->parameters.size() == 0 );
    REQUIRE( ((CodeBlock*)AST[0])->content[0]->type == "Literal" );

    REQUIRE( AST[1]->type == "CodeBlock" );
    REQUIRE( !((CodeBlock*)AST[1])->return_type );
    REQUIRE( ((CodeBlock*)AST[1])->parameters.size() == 0 );
    REQUIRE( ((CodeBlock*)AST[1])->content[0]->type == "Literal" );
    REQUIRE( ((Literal*)((CodeBlock*)AST[1])->content[0])->l_string == "foo" );

    REQUIRE( AST[2]->type == "CodeBlock" );
    REQUIRE( !((CodeBlock*)AST[2])->return_type );
    REQUIRE( ((CodeBlock*)AST[2])->parameters[0].type_data.name == "int" );
    REQUIRE( ((CodeBlock*)AST[2])->parameters[0].name == "foo" );
    REQUIRE( ((CodeBlock*)AST[2])->content[0]->type == "Literal" );
    REQUIRE( ((Literal*)((CodeBlock*)AST[2])->content[0])->l_string == "bar" );

    REQUIRE( AST[3]->type == "CodeBlock" );
    REQUIRE( ((CodeBlock*)AST[3])->return_type.value().name == "string" );
    REQUIRE( ((CodeBlock*)AST[3])->parameters[0].type_data.name == "int" );
    REQUIRE( ((CodeBlock*)AST[3])->parameters[0].name == "foo" );
    REQUIRE( ((CodeBlock*)AST[3])->parameters[0].default_argument.value()->type == "Literal" );
    REQUIRE( ((Literal*)((CodeBlock*)AST[3])->parameters[0].default_argument.value())->l_integer == 0 );
    REQUIRE( ((CodeBlock*)AST[3])->parameters[0].argument_expansion == Array );
    REQUIRE( ((CodeBlock*)AST[3])->parameters[1].type_data.name == "float" );
    REQUIRE( ((CodeBlock*)AST[3])->parameters[1].name == "bar" );
    REQUIRE( ((CodeBlock*)AST[3])->parameters[1].argument_expansion == Dictionary );
    REQUIRE( ((CodeBlock*)AST[3])->content[0]->type == "Literal" );
    REQUIRE( ((Literal*)((CodeBlock*)AST[3])->content[0])->l_string == "foobar" );
}

TEST_CASE("Test Syntax Analyser Invalid Code Block")
{
    string text = "(";

    try
    {
        AnalyseSyntax(Tokenise(text));
    }
    catch (Node* node)
    {
        REQUIRE( node->error->type == SyntaxError );
        REQUIRE( node->error->text == "Missing ending )" );
    }

    text = "(int foo =";

    try
    {
        AnalyseSyntax(Tokenise(text));
    }
    catch (Node* node)
    {
        REQUIRE( node->error->type == SyntaxError );
        REQUIRE( node->error->text == "Missing ending )" );
    }

    text = "(,";
    
    try
    {
        AnalyseSyntax(Tokenise(text));
    }
    catch(Node* node)
    {
        REQUIRE( node->error->type == SyntaxError );
        REQUIRE( node->error->text == "Missing argument" );
    }

    text = "(0)";

    try
    {
        AnalyseSyntax(Tokenise(text));
    }
    catch(Node* node)
    {
        REQUIRE( node->error->type == SyntaxError );
        REQUIRE( node->error->text == "Invalid character in code block definition" );
    }

    text = "(int 0)";

    try
    {
        AnalyseSyntax(Tokenise(text));
    }
    catch(Node* node)
    {
        REQUIRE( node->error->type == SyntaxError );
        REQUIRE( node->error->text == "Invalid character in code block definition" );
    }

    text = "() -> 0";

    try
    {
        AnalyseSyntax(Tokenise(text));
    }
    catch(Node* node)
    {
        REQUIRE( node->error->type == SyntaxError );
        REQUIRE( node->error->text == "Missing code block body" );
    }

    text = "() -> 0 {}";

    try
    {
        AnalyseSyntax(Tokenise(text));
    }
    catch(Node* node)
    {
        REQUIRE( node->error->type == SyntaxError );
        REQUIRE( node->error->text == "Invalid character in code block definition" );
    }

    text = "() -> string {";

    try
    {
        AnalyseSyntax(Tokenise(text));
    }
    catch(Node* node)
    {
        REQUIRE( node->error->type == SyntaxError );
        REQUIRE( node->error->text == "Missing ending }" );
    }

    text = "() -> string 0";

    try
    {
        AnalyseSyntax(Tokenise(text));
    }
    catch(Node* node)
    {
        REQUIRE( node->error->type == SyntaxError );
        REQUIRE( node->error->text == "Invalid character in code block definition" );
    }

    text = "{";

    try
    {
        AnalyseSyntax(Tokenise(text));
    }
    catch(Node* node)
    {
        REQUIRE( node->error->type == SyntaxError );
        REQUIRE( node->error->text == "Missing ending }" );
    }
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

    text = "int foo = 0"; // No ending semi-colon

    try
    {
        AnalyseSyntax(Tokenise(text));
    }
    catch (Node* node)
    {
        REQUIRE( node->error->type == SyntaxError );
        REQUIRE( node->error->text == "Missing ending ;" );
    }

    text = "int foo = ;";

    try
    {
        AnalyseSyntax(Tokenise(text));
    }
    catch (Node* node)
    {
        REQUIRE( node->error->type == SyntaxError );
        REQUIRE( node->error->text == "Missing assignment value" );
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

TEST_CASE("Test Syntax Analyser Function Call")
{
    string text = "foo(); bar(0.);";
    vector<Node*> AST = get<0>(AnalyseSyntax(Tokenise(text)));

    REQUIRE( AST[0]->type == "FunctionCall" );
    REQUIRE( ((FunctionCall*)AST[0])->name == "foo" );
    REQUIRE( ((FunctionCall*)AST[0])->arguments.size() == 0 );

    REQUIRE( AST[1]->type == "FunctionCall" );
    REQUIRE( ((FunctionCall*)AST[1])->name == "bar" );
    REQUIRE( ((Literal*)((FunctionCall*)AST[1])->arguments[0])->l_float == 0. );

    text = "foo(";
    
    try
    {
        AnalyseSyntax(Tokenise(text));
    }
    catch(Node* node)
    {
        REQUIRE( node->error->type == SyntaxError );
        REQUIRE( node->error->text == "Missing ending )" );
    }

    text = "foo(bar, ";
    
    try
    {
        AnalyseSyntax(Tokenise(text));
    }
    catch(Node* node)
    {
        REQUIRE( node->error->type == SyntaxError );
        REQUIRE( node->error->text == "Missing ending )" );
    }

    text = "foo(,";
    
    try
    {
        AnalyseSyntax(Tokenise(text));
    }
    catch(Node* node)
    {
        REQUIRE( node->error->type == SyntaxError );
        REQUIRE( node->error->text == "Missing argument" );
    }
}
