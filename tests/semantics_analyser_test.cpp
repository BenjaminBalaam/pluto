#include "catch.hpp"

#include "../src/node.hpp"
#include "../src/lexer.hpp"
#include "../src/syntax_analyser.hpp"
#include "../src/semantics_analyser.hpp"

using namespace std;

TEST_CASE("Test Semantics Analyser Semicolons")
{
    string text = "a; b; c;";
    vector<Node*> AST = AnalyseSemantics(get<0>(AnalyseSyntax(Tokenise(text))));

    REQUIRE( AST[0]->type == "GetVariable" );
    REQUIRE( ((GetVariable*)AST[0])->name == "a" );
    REQUIRE( AST[1]->type == "GetVariable" );
    REQUIRE( ((GetVariable*)AST[1])->name == "b" );
    REQUIRE( AST[2]->type == "GetVariable" );
    REQUIRE( ((GetVariable*)AST[2])->name == "c" );

    text = "a;; b;";
    AST = AnalyseSemantics(get<0>(AnalyseSyntax(Tokenise(text))));

    REQUIRE( AST[0]->type == "GetVariable" );
    REQUIRE( ((GetVariable*)AST[0])->name == "a" );
    REQUIRE( AST[1]->type == "GetVariable" );
    REQUIRE( ((GetVariable*)AST[1])->name == "b" );

    text = "0; if (true) {} 0; 1;";
    AST = AnalyseSemantics(get<0>(AnalyseSyntax(Tokenise(text))));

    REQUIRE( AST[0]->type == "Literal" );
    REQUIRE( ((Literal*)AST[0])->l_integer == 0 );
    REQUIRE( AST[1]->type == "IfStatement" );
    REQUIRE( ((IfStatement*)AST[1])->if_expression->type == "Literal" );
    REQUIRE( ((Literal*)((IfStatement*)AST[1])->if_expression)->l_boolean == true );
    REQUIRE( ((IfStatement*)AST[1])->if_code_block->type == "CodeBlock" );
    REQUIRE( ((CodeBlock*)((IfStatement*)AST[1])->if_code_block)->content.size() == 0 );
    REQUIRE( AST[2]->type == "Literal" );
    REQUIRE( ((Literal*)AST[2])->l_integer == 0 );
    REQUIRE( AST[3]->type == "Literal" );
    REQUIRE( ((Literal*)AST[3])->l_integer == 1 );

    text = "0 1; 2;";
    
    try
    {
        AnalyseSemantics(get<0>(AnalyseSyntax(Tokenise(text))));

        FAIL();
    }
    catch (Node *node)
    {
        REQUIRE( node->error->type == SyntaxError );
        REQUIRE( node->error->text == "Missing ending ;" );
    }

    text = "0; 1; 2";
    
    try
    {
        AnalyseSemantics(get<0>(AnalyseSyntax(Tokenise(text))));

        FAIL();
    }
    catch (Node *node)
    {
        REQUIRE( node->error->type == SyntaxError );
        REQUIRE( node->error->text == "Missing ending ;" );
    }

    text = "if (true) { int a = 0 }";

    try
    {
        AnalyseSemantics(get<0>(AnalyseSyntax(Tokenise(text))));

        FAIL();
    }
    catch (Node *node)
    {
        REQUIRE( node->error->type == SyntaxError );
        REQUIRE( node->error->text == "Missing ending ;" );
    }
}

TEST_CASE("Test Semantics Analyser Check Statement")
{
    string text = "public static const;";
    
    try
    {
        AnalyseSemantics(get<0>(AnalyseSyntax(Tokenise(text))));

        FAIL();
    }
    catch (Node *node)
    {
        REQUIRE( node->error->type == SyntaxError );
        REQUIRE( node->error->text == "Invalid expression" );
    }

    text = "return;";

    try
    {
        AnalyseSemantics(get<0>(AnalyseSyntax(Tokenise(text))));

        FAIL();
    }
    catch (Node *node)
    {
        REQUIRE( node->error->type == SyntaxError );
        REQUIRE( node->error->text == "Return outside function" );
    }

    text = "break;";

    try
    {
        AnalyseSemantics(get<0>(AnalyseSyntax(Tokenise(text))));

        FAIL();
    }
    catch (Node *node)
    {
        REQUIRE( node->error->type == SyntaxError );
        REQUIRE( node->error->text == "Break outside loop" );
    }

    text = "continue;";

    try
    {
        AnalyseSemantics(get<0>(AnalyseSyntax(Tokenise(text))));

        FAIL();
    }
    catch (Node *node)
    {
        REQUIRE( node->error->type == SyntaxError );
        REQUIRE( node->error->text == "Continue outside loop" );
    }

    text = "if (true) { break; }";

    try
    {
        AnalyseSemantics(get<0>(AnalyseSyntax(Tokenise(text))));

        FAIL();
    }
    catch (Node *node)
    {
        REQUIRE( node->error->type == SyntaxError );
        REQUIRE( node->error->text == "Break outside loop" );
    }
}

TEST_CASE("Test Semantics Analyser Check Expression")
{
    string text = "if (if (true) {}) {}";

    try
    {
        AnalyseSemantics(get<0>(AnalyseSyntax(Tokenise(text))));

        FAIL();
    }
    catch (Node *node)
    {
        REQUIRE( node->error->type == SyntaxError );
        REQUIRE( node->error->text == "Invalid expression" );
    }

    text = "for (int i = 0; i < length; i += 1) { if (return) {} }";

    try
    {
        AnalyseSemantics(get<0>(AnalyseSyntax(Tokenise(text))));

        FAIL();
    }
    catch (Node *node)
    {
        REQUIRE( node->error->type == SyntaxError );
        REQUIRE( node->error->text == "Invalid expression" );
    }
}