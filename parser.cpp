#include <istream>
#include <iostream>
#include <stdlib.h>
#include <cstdio>
#include <cctype>
#include "parser.h"

using namespace std;

tokenMap TokenParser::keyWord;

void TokenParser::makeDelimToken()
{
	current.type = TYPE_DELIM;
	switch (ch)
	{
    case ',':
        current.value = T_COMMA;
        break;
    case ';':
        current.value = T_SEMIC;
        break;
    case '(':
        current.value = T_LBRACK;
        break;
    case ')':
        current.value = T_RBRACK;
        break;
    case EOF:
        current.value = T_EOF;
        break;
    }
}

void repl_terms(string& str, const char* find, const unsigned l1, const char* replace)
{
    int pos = str.find(find,0);
    while (pos != string::npos)
    {
        str.replace(pos, l1, replace);
        pos = str.find(find,pos);
    }
}


void TokenParser::makeOperationToken()
{
    current.type = TYPE_OPER;
    switch (ch)
    {
    case '*':
        current.value = T_MUL;
        break;
    case '/' :
		current.value = T_DIV;
        break;
    case '+' :
        current.value = T_PLUS;
        break;
    case '-' :
        current.value = T_MINUS;
        break;
    case '%' :
        current.value = T_MOD;
        break;
    case '^' :
        current.value = T_DEG;
        break;
    case '!':
        current.value = T_NOT;
        break;
    case '<':
        current.value = T_LSS;
        break;
    case '>':
        current.value = T_GRT;
        break;
    case '=':
        current.value = T_ASS;
        break;
    }
}

void TokenParser::getNextToken()
{
	if (pushbacked)
	{
		pushbacked = false;
		return;
	}
	States state = ST_START;
	while (state != ST_FINISH)
        switch (state)
        {
            case ST_START:
                while (isspace(ch))
                {
                    if (ch == '\n')
                        ++lineNumber;
                    getNextChar();
                }
                if (isalpha(ch) || ch == '_' || ch == '{' || ch =='}')
                { // ID или {block}
                    clearBuffer();
                    addCharToBuffer();
                    getNextChar();
                    while (isalpha(ch) || ch == '_' || isdigit(ch))
                    {
                        addCharToBuffer();
                        getNextChar();
                    }
                    tokenMap::iterator pos = keyWord.find(current.str);
                    if ( pos != keyWord.end())
                    {
                        current.type 	= TYPE_RES;
                        current.value 	= pos->second;
                    }
                    else
                        current.type = TYPE_ID;
                    state = ST_FINISH;

                }
                else if (isdigit(ch))
                { // Num
                    clearBuffer();
                    do
                    {
                        addCharToBuffer();
                        getNextChar();
                    } while (isdigit(ch));
                    current.isFloat = false;
                    if (ch == '.')
                    {
                        current.isFloat = true;
                        addCharToBuffer();
                        getNextChar();
                        while (isdigit(ch))
                        {
                            addCharToBuffer();
                            getNextChar();
                        }
                    }
                    current.type = TYPE_LIT;
                    current.value = atof(current.str.c_str());
                    state = ST_FINISH;
                }
                else if (ch == '\"')
                { // "string"
                    clearBuffer();
                    getNextChar();
                    state = ST_STR;
                }
                else if (ch=='/')
				{
					getNextChar();
					if (ch=='/')
					{
						while (ch!='\n')
							getNextChar();
						input.putback(ch);
					}
					else
					{
						input.putback(ch);
						makeOperationToken();
						getNextChar();
						state = ST_FINISH;
					}
				}
                else if (isOP())
                { // +, -, *, /, %, ^
                    makeOperationToken();
                    getNextChar();
                    state = ST_FINISH;
                }
                else if (ch =='|')
                { //  ||
                    getNextChar();
                    if (ch != '|')
                        error(ch, '|');
                    current.type = TYPE_OPER;
                    current.value = T_OR;
                    getNextChar();
                    state = ST_FINISH;
                }
                else if (ch == '&')
                { // &&
                    getNextChar();
                    if (ch != '&')
                        error(ch, '&');
                    current.type = TYPE_OPER;
                    current.value = T_AND;
                    getNextChar();
                    state = ST_FINISH;
                }
                else if (isOP3())
                { // =, !, <, >, ==, !=, <=, >=
                    makeOperationToken();
                    getNextChar();
                    if (ch == '=')
                    {
                        switch (current.getLong())
                        {
                        case T_ASS:
                            current.value = T_EQ;
                            break;
                        case T_NOT:
                            current.value = T_NEQ;
                            break;
                        case T_LSS:
                            current.value = T_LSSEQ;
                            break;
                        case T_GRT:
                            current.value = T_GRTEQ;
                            break;
                        }
                        getNextChar();
                    }
                    state = ST_FINISH;
                }
                else if (isDelim())
                { // Delimiter: ;, ,, (, ), EOF
                    makeDelimToken();
                    getNextChar();
                    state = ST_FINISH;

                }
                else
                    error(ch);

                break; // fin
            case ST_STR:
                while (ch != '\"')
                {
                    if (ch == EOF)
                        throw TokenParserException("Not closed string !");
                    addCharToBuffer();
                    getNextChar();
                }
                current.type = TYPE_LITSTR;
                repl_terms(current.str, "\\n", 2, "\n");
                repl_terms(current.str, "\\t", 2, "\t");
                getNextChar();
                state = ST_FINISH;
                break;
            case ST_FINISH:
                break;
        }
}
char* tokenName[] =
{
		"let", "int", "double", "void", "if",
		"then", "else", "while", "do", "out", "return", "in",
		"=", "||", "&&", "+", "-", "*", "/", "%", "^", "!",
		"==", "!=", "<", "<=", ">", ">=", "-", "+", ";", ",", "(", ")", "EOF"
};
