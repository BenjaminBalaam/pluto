#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <regex>

#include "main.hpp"

using namespace std;

int main(int argc, char *argv[])
{
    fstream f;
    f.open(argv[1], ios::in);

    string raw_text;

    while (getline(f, raw_text)) {
        raw_text += "\n";
    }

    cout << raw_text;

    vector<Token*> tokens = vector<Token*>();

    regex re_string_start = regex("^[\"'`]");
    regex re_white_space = regex("^[ \t\f\r\n]");
    regex re_integer = regex("^-?(0[box])?[0-9]+");
    regex re_float = regex("^[0-9]+-?[0-9]+");
    regex re_identifier = regex("^[a-zA-Z_][a-zA-Z0-9_]*");

    smatch m;

    int current_char;

    bool currently_string = false;
    bool escaping_string = false;
    char string_start;

    String *string_token;

    while (raw_text != "")
    {
        if (currently_string)
        {
            if (raw_text[0] == '\n' && !escaping_string)
            {
                string_token->error = Error {SyntaxError, "Unterminated string literal"};

                break;
            }
            else if (raw_text[0] == string_start && !escaping_string)
            {
                currently_string = false;

                string_token->end = current_char;
                
                tokens.push_back(string_token);
            }
            else if (escaping_string)
            {
                string_token->content.append({raw_text[0]});

                EraseFront(&raw_text, &current_char, 1);

                escaping_string = false;

            }
            else if (raw_text[0] == '\\')
            {
                EraseFront(&raw_text, &current_char, 1);

                switch (raw_text[0]) {
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

                        regex re_octal = regex("^[0-7]{1,3}");
                        regex re_hex = regex("^x[0-9a-fA-F]{2}");
                        regex re_hex_16 = regex("^u[0-9a-fA-F]{4}");
                        regex re_hex_32 = regex("^u[0-9a-fA-F]{8}");

                        if (regex_search(raw_text, escape_m, re_octal))
                        {
                            string_token->content.append({(char)stoi(escape_m[0], 0, 8)});

                            EraseFront(&raw_text, &current_char, escape_m.length());

                            continue;
                        }
                        else if (regex_search(raw_text, escape_m, re_hex))
                        {
                            string_token->content.append({(char)stoi(escape_m[0], 0, 16)});

                            EraseFront(&raw_text, &current_char, 2);

                            continue;
                        }
                        else if (regex_search(raw_text, escape_m, re_hex_16))
                        {
                            string_token->content.append({(char)stoi(escape_m[0], 0, 16)});

                            EraseFront(&raw_text, &current_char, 4);

                            continue;
                        }
                        else if (regex_search(raw_text, escape_m, re_hex_32))
                        {
                            string_token->content.append({(char)stoi(escape_m[0], 0, 16)});

                            EraseFront(&raw_text, &current_char, 8);

                            continue;
                        }
                        else
                        {
                            escaping_string = true;

                            continue;
                        }
                }

                EraseFront(&raw_text, &current_char, 1);
            }
            else
            {
                string_token->content.append({raw_text[0]});

                EraseFront(&raw_text, &current_char, 1);
            }
        }
        else if (regex_search(raw_text, m, re_string_start))
        {
            currently_string = true;

            string_start = *((string)m[0]).data();

            string_token = new String("");

            string_token->start = current_char;
        }
        else if (regex_search(raw_text, m, re_white_space))
        {
            EraseFront(&raw_text, &current_char, m.length());
        }
        else if (regex_search(raw_text, m, re_integer))
        {
            Integer *new_integer = new Integer(stoi(m[0]));

            new_integer->start = current_char;
            new_integer->end = current_char + m.length();

            tokens.push_back(new_integer);

            EraseFront(&raw_text, &current_char, m.length());
        }
        else if (regex_search(raw_text, m, re_float))
        {
            Float *new_float = new Float(stof(m[0]));

            new_float->start = current_char;
            new_float->end = current_char + m.length();

            tokens.push_back(new_float);

            EraseFront(&raw_text, &current_char, m.length());
        }
        else if (regex_search(raw_text, m, re_identifier))
        {
            if (find(begin(Keywords), end(Keywords), m[0]) != end(Keywords))
            {
                Keyword *new_keyword = new Keyword(m[0]);

                new_keyword->start = current_char;
                new_keyword->end = current_char + m.length();

                tokens.push_back(new_keyword);

                EraseFront(&raw_text, &current_char, m.length());
            }
            else
            {
                Identifier *new_identifier = new Identifier(m[0]);

                new_identifier->start = current_char;
                new_identifier->end = current_char + m.length();

                tokens.push_back(new_identifier);

                EraseFront(&raw_text, &current_char, m.length());
            }
        }
    }

    for (Token* token : tokens)
    {
        cout << token << "\n";

        delete token;
    }

    return 0;
}

void EraseFront(string *text, int *current, int length)
{
    text->erase(0, length);

    *current += length;
}