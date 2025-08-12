#include "catch.hpp"

#include "../src/token.hpp"
#include "../src/lexer.hpp"

using namespace std;

TEST_CASE("Test Lexer Numbers")
{
    string text = "-12.4 .5 0. 2 -0b010 0o732 0xAF2 0xaf2";

    vector<Token*> tokens = Tokenise(text);

    REQUIRE( tokens[0]->type == "Float" );
    REQUIRE( get<2>(GetTokenValue(tokens[0])) == -12.4 );

    REQUIRE( tokens[1]->type == "Float" );
    REQUIRE( get<2>(GetTokenValue(tokens[1])) == .5 );

    REQUIRE( tokens[2]->type == "Float" );
    REQUIRE( get<2>(GetTokenValue(tokens[2])) == 0. );

    REQUIRE( tokens[3]->type == "Integer" );
    REQUIRE( get<1>(GetTokenValue(tokens[3])) == 2 );

    REQUIRE( tokens[4]->type == "Integer" );
    REQUIRE( get<1>(GetTokenValue(tokens[4])) == -0b010 );

    REQUIRE( tokens[5]->type == "Integer" );
    REQUIRE( get<1>(GetTokenValue(tokens[5])) == 0732 );

    REQUIRE( tokens[6]->type == "Integer" );
    REQUIRE( get<1>(GetTokenValue(tokens[6])) == 0xAF2 );

    REQUIRE( tokens[7]->type == "Integer" );
    REQUIRE( get<1>(GetTokenValue(tokens[7])) == 0xaf2 );
}

TEST_CASE("Test Lexer Invalid Numbers")
{
    string invalid_binary = "0b45";
    vector<Token*> invalid_binary_tokens = Tokenise(invalid_binary);

    REQUIRE( invalid_binary_tokens[0]->error->type == SyntaxError );
    REQUIRE( invalid_binary_tokens[0]->error->text == "Invalid character in binary integer" );

    string invalid_octal = "0o9A";
    vector<Token*> invalid_octal_tokens = Tokenise(invalid_octal);

    REQUIRE( invalid_octal_tokens[0]->error->type == SyntaxError );
    REQUIRE( invalid_octal_tokens[0]->error->text == "Invalid character in octal integer" );
}

TEST_CASE("Test Lexer Strings")
{
    string text = "'foo' \"bar\" ```\nfoobar\n```";

    vector<Token*> tokens = Tokenise(text);

    REQUIRE( tokens[0]->type == "String" );
    REQUIRE( get<0>(GetTokenValue(tokens[0])) == "foo" );

    REQUIRE( tokens[1]->type == "String" );
    REQUIRE( get<0>(GetTokenValue(tokens[1])) == "bar" );

    REQUIRE( tokens[2]->type == "String" );
    REQUIRE( get<0>(GetTokenValue(tokens[2])) == "\nfoobar\n" );

}

TEST_CASE("Test Lexer Unterminated Strings")
{
    string text = "\"foo\nbar";
    vector<Token*> tokens = Tokenise(text);

    REQUIRE( tokens[0]->error->type == SyntaxError );
    REQUIRE( tokens[0]->error->text == "Unterminated string literal" );

    text = "\"foo\n";
    tokens = Tokenise(text);

    REQUIRE( tokens[0]->error->type == SyntaxError );
    REQUIRE( tokens[0]->error->text == "Unterminated string literal" );

    text = "\"bar'\n";
    tokens = Tokenise(text);

    REQUIRE( tokens[0]->error->type == SyntaxError );
    REQUIRE( tokens[0]->error->text == "Unterminated string literal" );

    text = "```\n";
    tokens = Tokenise(text);

    REQUIRE( tokens[0]->error->type == SyntaxError );
    REQUIRE( tokens[0]->error->text == "Unterminated string literal" );

    text = "```a`\n";
    tokens = Tokenise(text);

    REQUIRE( tokens[0]->error->type == SyntaxError );
    REQUIRE( tokens[0]->error->text == "Unterminated string literal" );

    text = "```b``\n";
    tokens = Tokenise(text);

    REQUIRE( tokens[0]->error->type == SyntaxError );
    REQUIRE( tokens[0]->error->text == "Unterminated string literal" );

}

TEST_CASE("Test Lexer Escaping Strings")
{
    string text = "'\\a \\b \\f \\n \\r \\t \\v' '\\051' '\\x6e' '\\u5AF2' '\\u0001FA0A' '\\\n'";

    vector<Token*> tokens = Tokenise(text);

    REQUIRE( tokens[0]->type == "String" );
    REQUIRE( get<0>(GetTokenValue(tokens[0])) == "\a \b \f \n \r \t \v" );

    REQUIRE( tokens[1]->type == "String" );
    REQUIRE( get<0>(GetTokenValue(tokens[1])) == ")" ); // The character with code \051

    REQUIRE( tokens[2]->type == "String" );
    REQUIRE( get<0>(GetTokenValue(tokens[2])) == "n" ); // The character with code \x6e

    REQUIRE( tokens[3]->type == "String" );
    REQUIRE( get<0>(GetTokenValue(tokens[3])) == "嫲" ); // The character with code \u5AF2

    REQUIRE( tokens[4]->type == "String" );
    REQUIRE( get<0>(GetTokenValue(tokens[4])) == "🨊" ); // The character with code \u0001FA0A

    REQUIRE( tokens[5]->type == "String" );
    REQUIRE( get<0>(GetTokenValue(tokens[5])) == "\n" );
}

TEST_CASE("Test Lexer Comments")
{
    string text = "// \\\n \n /* \\*/ \n */";
    vector<Token*> tokens = Tokenise(text);

    REQUIRE( tokens.size() == 0 );

    text = "/* \n";
    tokens = Tokenise(text);

    REQUIRE( tokens[0]->error->type == SyntaxError );
    REQUIRE( tokens[0]->error->text == "Unterminated multiline comment" );
}

TEST_CASE("Test Lexer White Space")
{
    string text = " \t \f \r \n";
    
    vector<Token*> tokens = Tokenise(text);

    REQUIRE( tokens.size() == 0 );
}

TEST_CASE("Test Lexer Identifiers")
{
    string text = "foo Bar g100 6aa";

    vector<Token*> tokens = Tokenise(text);

    REQUIRE( tokens[0]->type == "Identifier" );
    REQUIRE( get<0>(GetTokenValue(tokens[0])) == "foo" );

    REQUIRE( tokens[1]->type == "Identifier" );
    REQUIRE( get<0>(GetTokenValue(tokens[1])) == "Bar" );

    REQUIRE( tokens[2]->type == "Identifier" );
    REQUIRE( get<0>(GetTokenValue(tokens[2])) == "g100" );

    REQUIRE_FALSE( tokens[3]->type == "Identifier" ); // Identifiers cannot start with a digit
}

TEST_CASE("Test Lexer Keywords")
{
    string text = "static";

    vector<Token*> tokens = Tokenise(text);

    REQUIRE( tokens[0]->type == "Keyword" );
    REQUIRE( get<0>(GetTokenValue(tokens[0])) == "static" );
}

TEST_CASE("Test Lexer Control Characters")
{
    string text = ". , ;";

    vector<Token*> tokens = Tokenise(text);

    REQUIRE( tokens[0]->type == "Control" );
    REQUIRE( get<0>(GetTokenValue(tokens[0])) == "." );

    REQUIRE( tokens[1]->type == "Control" );
    REQUIRE( get<0>(GetTokenValue(tokens[1])) == "," );

    REQUIRE( tokens[2]->type == "Control" );
    REQUIRE( get<0>(GetTokenValue(tokens[2])) == ";" );
}

TEST_CASE("Test Lexer Bracket Characters")
{
    string text = "( ) [ ] { }";

    vector<Token*> tokens = Tokenise(text);

    REQUIRE( tokens[0]->type == "Bracket" );
    REQUIRE( get<0>(GetTokenValue(tokens[0])) == "(" );

    REQUIRE( tokens[1]->type == "Bracket" );
    REQUIRE( get<0>(GetTokenValue(tokens[1])) == ")" );

    REQUIRE( tokens[2]->type == "Bracket" );
    REQUIRE( get<0>(GetTokenValue(tokens[2])) == "[" );

    REQUIRE( tokens[3]->type == "Bracket" );
    REQUIRE( get<0>(GetTokenValue(tokens[3])) == "]" );

    REQUIRE( tokens[4]->type == "Bracket" );
    REQUIRE( get<0>(GetTokenValue(tokens[4])) == "{" );

    REQUIRE( tokens[5]->type == "Bracket" );
    REQUIRE( get<0>(GetTokenValue(tokens[5])) == "}" );
}

TEST_CASE("Test Lexer Operator Characters")
{
    string text = "== != <= >= += -= *= /= + - * / $ % ^ = < > ! & |";

    vector<Token*> tokens = Tokenise(text);

    REQUIRE( tokens[0]->type == "Operator" );
    REQUIRE( get<0>(GetTokenValue(tokens[0])) == "==" );

    REQUIRE( tokens[1]->type == "Operator" );
    REQUIRE( get<0>(GetTokenValue(tokens[1])) == "!=" );

    REQUIRE( tokens[2]->type == "Operator" );
    REQUIRE( get<0>(GetTokenValue(tokens[2])) == "<=" );

    REQUIRE( tokens[3]->type == "Operator" );
    REQUIRE( get<0>(GetTokenValue(tokens[3])) == ">=" );

    REQUIRE( tokens[4]->type == "Operator" );
    REQUIRE( get<0>(GetTokenValue(tokens[4])) == "+=" );

    REQUIRE( tokens[5]->type == "Operator" );
    REQUIRE( get<0>(GetTokenValue(tokens[5])) == "-=" );

    REQUIRE( tokens[6]->type == "Operator" );
    REQUIRE( get<0>(GetTokenValue(tokens[6])) == "*=" );

    REQUIRE( tokens[7]->type == "Operator" );
    REQUIRE( get<0>(GetTokenValue(tokens[7])) == "/=" );

    REQUIRE( tokens[8]->type == "Operator" );
    REQUIRE( get<0>(GetTokenValue(tokens[8])) == "+" );

    REQUIRE( tokens[9]->type == "Operator" );
    REQUIRE( get<0>(GetTokenValue(tokens[9])) == "-" );

    REQUIRE( tokens[10]->type == "Operator" );
    REQUIRE( get<0>(GetTokenValue(tokens[10])) == "*" );

    REQUIRE( tokens[11]->type == "Operator" );
    REQUIRE( get<0>(GetTokenValue(tokens[11])) == "/" );

    REQUIRE( tokens[12]->type == "Operator" );
    REQUIRE( get<0>(GetTokenValue(tokens[12])) == "$" );

    REQUIRE( tokens[13]->type == "Operator" );
    REQUIRE( get<0>(GetTokenValue(tokens[13])) == "%" );

    REQUIRE( tokens[14]->type == "Operator" );
    REQUIRE( get<0>(GetTokenValue(tokens[14])) == "^" );

    REQUIRE( tokens[15]->type == "Operator" );
    REQUIRE( get<0>(GetTokenValue(tokens[15])) == "=" );

    REQUIRE( tokens[16]->type == "Operator" );
    REQUIRE( get<0>(GetTokenValue(tokens[16])) == "<" );

    REQUIRE( tokens[17]->type == "Operator" );
    REQUIRE( get<0>(GetTokenValue(tokens[17])) == ">" );

    REQUIRE( tokens[18]->type == "Operator" );
    REQUIRE( get<0>(GetTokenValue(tokens[18])) == "!" );

    REQUIRE( tokens[19]->type == "Operator" );
    REQUIRE( get<0>(GetTokenValue(tokens[19])) == "&" );

    REQUIRE( tokens[20]->type == "Operator" );
    REQUIRE( get<0>(GetTokenValue(tokens[20])) == "|" );
}

TEST_CASE("Test Lexer Invalid Characters")
{
    string text = "¬";
    vector<Token*> tokens = Tokenise(text);

    REQUIRE( tokens[0]->error->type == SyntaxError );
    REQUIRE( tokens[0]->error->text == "Invalid character" );

    text = "🨊";
    tokens = Tokenise(text);

    REQUIRE( tokens[0]->error->type == SyntaxError );
    REQUIRE( tokens[0]->error->text == "Invalid character" );
}