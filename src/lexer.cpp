#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <regex>

#include "lexer.hpp"
#include "token.hpp"

using namespace std;

vector<Token*> Tokenise(string text)
{
    vector<Token*> tokens = vector<Token*>();

    regex re_string_start = regex("^([\"']|(```))");
    regex re_white_space = regex("^[ \t\f\r\n]");
    regex re_integer = regex("^((0[box][0-9A-Fa-f]+)|([0-9]+))");
    regex re_float = regex("^(([0-9]+\\.[0-9]*)|([0-9]*\\.[0-9]+))");
    regex re_bool = regex("^((false)|(true))");
    regex re_identifier = regex("^([a-zA-Z_][a-zA-Z0-9_]*)");
    regex re_control = regex("^[.,:;]");
    regex re_bracket = regex("^[\\(\\)\\[\\]\\{\\}]");
    regex re_operator = regex("^(([=!<>\\+\\-\\*\\/]=)|[\\+\\-\\*\\/\\$%\\^=<>!&\\|])");
    regex re_comment_start = regex("^(/[/\\*])");
    regex re_comment_end = regex("^(\\*/)");

    smatch m;

    int current_char = 0;
    bool escaping = false;

    bool currently_string = false;
    char string_start;
    String *string_token;

    bool currently_comment = false;
    bool multiline_comment = false;
    int comment_start = 0;

    while (text != "")
    {
        if (currently_string)
        {
            if (string_start == '`' && text != "\n" && text[0] != '`')
            {
                string_token->content.append({text[0]});

                EraseFront(&text, &current_char, 1);

                continue;
            }
            else if (text[0] == '\n' && !escaping)
            {
                string_token->error = Error {SyntaxError, "Unterminated string literal"};

                string_token->end = current_char;

                return vector<Token*> { string_token };
            }
            else if (text[0] == string_start && !escaping)
            {
                if (string_start == '`' && !(text.size() >= 3 && text[1] == '`' && text[2] == '`'))
                {
                    string_token->content.append({text[0]});

                    EraseFront(&text, &current_char, 1);
                }
                else
                {
                    currently_string = false;

                    string_token->end = current_char;
                    
                    tokens.push_back(string_token);

                    if (string_start == '`')
                    {
                        EraseFront(&text, &current_char, 3);
                    }
                    else
                    {
                        EraseFront(&text, &current_char, 1);
                    }
                }
            }
            else if (escaping)
            {
                string_token->content.append({text[0]});

                EraseFront(&text, &current_char, 1);

                escaping = false;

            }
            else if (text[0] == '\\')
            {
                EraseFront(&text, &current_char, 1);

                switch (text[0]) {
                    case 'a':
                        string_token->content.append("\a");
                        break;
                    case 'b':
                        string_token->content.append("\b");
                        break;
                    case 'f':
                        string_token->content.append("\f");
                        break;
                    case 'n':
                        string_token->content.append("\n");
                        break;
                    case 'r':
                        string_token->content.append("\r");
                        break;
                    case 't':
                        string_token->content.append("\t");
                        break;
                    case 'v':
                        string_token->content.append("\v");
                        break;
                    default:
                        smatch escape_m;

                        regex re_octal = regex("^[0-3][0-7]{0,2}");
                        regex re_hex = regex("^x[0-9a-fA-F]{2}");
                        regex re_hex_16 = regex("^u[0-9a-fA-F]{4}");
                        regex re_hex_32 = regex("^u[0-9a-fA-F]{8}");

                        if (regex_search(text, escape_m, re_octal))
                        {
                            string_token->content.append({(char)stoi(escape_m[0], 0, 8)});

                            EraseFront(&text, &current_char, escape_m.length());

                            continue;
                        }
                        else if (regex_search(text, escape_m, re_hex))
                        {
                            string number = ((string)escape_m[0]).substr(1);

                            string_token->content.append({(char)stoi(number, 0, 16)});

                            EraseFront(&text, &current_char, 3);

                            continue;
                        }
                        else if (regex_search(text, escape_m, re_hex_32))
                        {
                            string number = ((string)escape_m[0]).substr(1);

                            string_token->content.append(UnicodeToUTF8(stol(number, 0, 16)));

                            EraseFront(&text, &current_char, 9);

                            continue;
                        }
                        else if (regex_search(text, escape_m, re_hex_16))
                        {
                            string number = ((string)escape_m[0]).substr(1);

                            string_token->content.append(UnicodeToUTF8(stol(number, 0, 16)));

                            EraseFront(&text, &current_char, 5);

                            continue;
                        }
                        else
                        {
                            escaping = true;

                            continue;
                        }
                }

                EraseFront(&text, &current_char, 1);
            }
            else
            {
                string_token->content.append({text[0]});

                EraseFront(&text, &current_char, 1);
            }
        }
        else if (currently_comment)
        {
            if (multiline_comment && text == "\n")
            {
                Token *t = new Token();

                t->error = Error {SyntaxError, "Unterminated multiline comment"};

                t->start = comment_start;

                t->end = current_char;

                return vector<Token*> { t };
            }
            if (!multiline_comment && text[0] == '\n' && !escaping)
            {
                currently_comment = false;

                EraseFront(&text, &current_char, 1);
            }
            else if (multiline_comment && regex_search(text, re_comment_end) && !escaping)
            {
                currently_comment = false;

                EraseFront(&text, &current_char, 2);
            }
            else if (escaping)
            {
                EraseFront(&text, &current_char, 1);
            }
            else if (text[0] == '\\')
            {
                escaping = true;

                EraseFront(&text, &current_char, 1);
            }
            else
            {
                EraseFront(&text, &current_char, 1);
            }
        }
        else if (regex_search(text, m, re_string_start))
        {
            currently_string = true;

            string_start = ((string)m[0])[0];

            string_token = new String("");

            string_token->start = current_char;

            EraseFront(&text, &current_char, m[0].length());
        }
        else if (regex_search(text, m, re_comment_start))
        {
            currently_comment = true;

            comment_start = current_char;

            if (m[0] == "//")
            {
                multiline_comment = false;
            }
            else
            {
                multiline_comment = true;
            }

            EraseFront(&text, &current_char, 2);
        }
        else if (regex_search(text, m, re_white_space))
        {
            EraseFront(&text, &current_char, m.length());
        }
        else if (regex_search(text, m, re_float))
        {
            Float *new_float = new Float(stod(m[0]));

            new_float->start = current_char;
            new_float->end = current_char + m.length();

            tokens.push_back(new_float);

            EraseFront(&text, &current_char, m.length());
        }
        else if (regex_search(text, m, re_integer))
        {
            int length = m[0].length();

            string string_value = m[0];

            int value = 0;

            if (string_value[0] == '0' && string_value.size() != 0)
            {
                if (string_value[1] == 'b')
                {
                    string_value.erase(0, 2);

                    try
                    {
                        value = stoi(string_value, 0, 2);
                    }
                    catch (invalid_argument)
                    {
                        Token *t = new Token();

                        t->error = Error {SyntaxError, "Invalid character in binary integer"};
                        t->start = current_char;
                        t->end = current_char + length;

                        return vector<Token*> { t };
                    }
                }
                else if (string_value[1] == 'o')
                {
                    string_value.erase(0, 2);

                    try
                    {
                        value = stoi(string_value, 0, 8);
                    }
                    catch (invalid_argument)
                    {
                        Token *t = new Token();

                        t->error = Error {SyntaxError, "Invalid character in octal integer"};
                        t->start = current_char;
                        t->end = current_char + length;

                        return vector<Token*> { t };
                    }
                }
                else if (string_value[1] == 'x')
                {
                    string_value.erase(0, 2);

                    try
                    {
                        value = stoi(string_value, 0, 16);
                    }
                    catch (invalid_argument)
                    {
                        Token *t = new Token();

                        t->error = Error {SyntaxError, "Invalid character in hexadecimal integer"};
                        t->start = current_char;
                        t->end = current_char + length;

                        return vector<Token*> { t };
                    }
                }
                else
                {
                    value = stoi(string_value);
                }
            }
            else
            {
                value = stoi(string_value);
            }

            Integer *new_integer = new Integer(value);

            new_integer->start = current_char;
            new_integer->end = current_char + m.length();

            tokens.push_back(new_integer);

            EraseFront(&text, &current_char, m.length());
        }
        else if (regex_search(text, m, re_bool))
        {
            Boolean *new_boolean = new Boolean(m[0] == "true" ? true : false);

            new_boolean->start = current_char;
            new_boolean->end = current_char + m.length();

            tokens.push_back(new_boolean);

            EraseFront(&text, &current_char, m.length());
        }
        else if (regex_search(text, m, re_identifier))
        {
            if (find(begin(Keywords), end(Keywords), m[0]) != end(Keywords))
            {
                Keyword *new_keyword = new Keyword(m[0]);

                new_keyword->start = current_char;
                new_keyword->end = current_char + m.length();

                tokens.push_back(new_keyword);

                EraseFront(&text, &current_char, m.length());
            }
            else
            {
                Identifier *new_identifier = new Identifier(m[0]);

                new_identifier->start = current_char;
                new_identifier->end = current_char + m.length();

                tokens.push_back(new_identifier);

                EraseFront(&text, &current_char, m.length());
            }
        }
        else if (regex_search(text, m, re_control))
        {
            Control *new_control = new Control(m[0]);

            new_control->start = current_char;
            new_control->end = current_char + m.length();

            tokens.push_back(new_control);

            EraseFront(&text, &current_char, m.length());
        }
        else if (regex_search(text, m, re_bracket))
        {
            Bracket *new_bracket = new Bracket(m[0]);

            new_bracket->start = current_char;
            new_bracket->end = current_char + m.length();

            tokens.push_back(new_bracket);

            EraseFront(&text, &current_char, m.length());
        }
        else if (regex_search(text, m, re_operator))
        {
            Operator *new_operator = new Operator(m[0]);

            new_operator->start = current_char;
            new_operator->end = current_char + m.length();

            tokens.push_back(new_operator);

            EraseFront(&text, &current_char, m.length());
        }
        else
        {
            Token *t = new Token();

            t->error = Error {SyntaxError, "Invalid character"};
            t->start = current_char;
            t->end = current_char + 1;

            return vector<Token*> { t };
        }
    }

    return tokens;
}

// https://stackoverflow.com/a/19968992: By Mark Ransom https://stackoverflow.com/users/5987/mark-ransom
string UnicodeToUTF8(unsigned int codepoint)
{
    string out;

    if (codepoint <= 0x7f)
        out.append(1, static_cast<char>(codepoint));
    else if (codepoint <= 0x7ff)
    {
        out.append(1, static_cast<char>(0xc0 | ((codepoint >> 6) & 0x1f)));
        out.append(1, static_cast<char>(0x80 | (codepoint & 0x3f)));
    }
    else if (codepoint <= 0xffff)
    {
        out.append(1, static_cast<char>(0xe0 | ((codepoint >> 12) & 0x0f)));
        out.append(1, static_cast<char>(0x80 | ((codepoint >> 6) & 0x3f)));
        out.append(1, static_cast<char>(0x80 | (codepoint & 0x3f)));
    }
    else
    {
        out.append(1, static_cast<char>(0xf0 | ((codepoint >> 18) & 0x07)));
        out.append(1, static_cast<char>(0x80 | ((codepoint >> 12) & 0x3f)));
        out.append(1, static_cast<char>(0x80 | ((codepoint >> 6) & 0x3f)));
        out.append(1, static_cast<char>(0x80 | (codepoint & 0x3f)));
    }
    return out;
}

void EraseFront(string *text, int *current, int length)
{
    text->erase(0, length);

    *current += length;
}