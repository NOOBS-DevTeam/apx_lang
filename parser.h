#ifndef PARSER_H_
#define PARSER_H_
#include <istream>
#include <cstdio>
#include <map>
#include "except.h"

using namespace std;

extern char* tokenName[];
// ВСЕ типы токенов
enum TokenType
{
	TYPE_ID, // Идентификатор
	TYPE_RES,
	TYPE_OPER, //Операция
	TYPE_DELIM, // Знак
	TYPE_LIT, // Численный литерал
	TYPE_LITSTR // Строковой литерал
};
// ВСЕ токены
enum Tokens
{
	// Reserved words
	T_VAR=1,
	T_INT,
    T_FLOAT,
    T_VOID,
    T_BEGIN,
    T_END,
    T_IF,
    T_THEN,
    T_ELSE,
    T_WHILE,
	T_DO,
	T_WRITE,
	T_RETURN,
	T_READ,

	// Operations
	T_ASS,	// =
	T_OR,	// ||
	T_AND,	// &&
	T_PLUS,	// +
	T_MINUS,	// -
	T_MUL,	// *
	T_DIV,	// /
	T_MOD,	// %
	T_DEG,	// ^
	T_NOT,	// !
	T_EQ,	// ==
	T_NEQ,	// !=
	T_LSS,	// <
	T_LSSEQ,	// <=
	T_GRT,	// >
	T_GRTEQ,	// >=
	T_MINUSUN, // Унарный +
	T_PLUSUN, // Унарный -

	// Знаки
	T_SEMIC, // ;
	T_COMMA, // ,
	T_LBRACK, // (
	T_RBRACK,	// )
	T_EOF
};
typedef map<string, Tokens> tokenMap;
//Собсна токен
struct Token
{
	TokenType type;
	long double value; //Значение
	string str;
	bool isFloat; //является ли веществ.
	long getLong()
	{
		return long(value);
	}
};

class TokenParser
{
	// Вывод ошибки
	void error(int _ch = 0, int expect = 0)
	{
		if ( _ch == 0 )
			throw TokenParserException(lineNumber);
		else if (expect == 0 )
			throw TokenParserException(lineNumber, _ch);
		throw TokenParserException(lineNumber, _ch, expect);
	}
	// Словарь ключевых слов
	static map<string, Tokens> keyWord;

	static void initKeyWords()
    {
        if (keyWord.size() >0)
            return;
        keyWord["let"]	  = T_VAR;
        keyWord["int"] 	  = T_INT;
        keyWord["double"] = T_FLOAT;
        keyWord["void"]   = T_VOID;
        keyWord["{"] 	  = T_BEGIN;
        keyWord["}"] 	  = T_END;
        keyWord["if"] 	  = T_IF;
        keyWord["then"]	  = T_THEN;
        keyWord["else"]   = T_ELSE;
        keyWord["while"]  = T_WHILE;
        keyWord["do"] 	  = T_DO;
        keyWord["out"] 	  = T_WRITE;
        keyWord["return"] = T_RETURN;
        keyWord["in"] 	  = T_READ;
    }
	istream& input;

	int ch;
	bool pushbacked;
	Token current;
	int lineNumber;

	enum States
	{
	    ST_START,
        ST_STR,
        ST_FINISH
    };
	// Является ли знаком
	bool isDelim()
	{
		return (ch == ';' || ch == ',' || ch == '(' || ch == ')' || ch == EOF );
	}
	//
	void makeDelimToken();
	// Является ли операцией (втор.)
	bool isOP()
	{
		return ( ch == '+' || ch == '-' || ch == '*' || ch == '%' || ch == '/' || ch == '^');
	}
	// Является ли операцией (перв.)
	bool isOP3()
	{
		return (ch == '=' || ch == '!' || ch == '<' || ch == '>' );
	}

	void makeOperationToken();

	void getNextChar()
	{
		ch = input.get();
	}
	// Очистка строки
	void clearBuffer()
	{
		current.str.clear();
	}
	//Добавление симв. в строку
	void addCharToBuffer()
	{
		current.str += ch;
	}

public:
	// Конструктор парсера
	TokenParser(istream& in) : input(in), pushbacked(false), lineNumber(0)
	{
		initKeyWords();
		getNextChar();
	}
	// Текущ. токен
	Token* currentToken()
	{
		return &current;
	}

	int getLineNumber()
	{
		return lineNumber;
	}

    void pushBack()
	{
		pushbacked = true;
	}

	void getNextToken();
};

#endif
