#ifndef EXCEPT_H_
#define EXCEPT_H_
#include <exception>
#include <string>
#include <sstream>

using namespace std;

extern char* tokenName[];
// Главный класс исключений
class ParserException : public exception
{
protected:
	string Str; //Текст ошибки
public:
	ParserException(const string& str = "") : Str(str)
	{
		//конструктор
	}
	virtual const char* what() const throw()
	{
		return Str.c_str();
	}

	virtual ~ParserException() throw () {}
};
// Исключение парсера токенов
class TokenParserException : public ParserException
{
public:
    TokenParserException(int line)
	{
		ostringstream os;
		os << "Token parser exception: Bad char, line: " << line+1 << '!';
		Str = os.str() ;
	}
	TokenParserException(int line, char bad)
	{
		ostringstream os;
		os << "Token parser exception: Bad char: '" << bad << "', line: " << line+1 << '!';
		Str = os.str() ;
	}
	TokenParserException(int line, char bad, char expect )
	{
		ostringstream os;
		os 	<< "Token parser exception: Bad char: '" << bad << "', expected: '"
			<< expect << "', line: " << line+1 << '!';
		Str = os.str() ;
	}
	TokenParserException(const string& str)
	{
		Str = "Token parser exception: ";
		Str += str;
	}
};
// Исключение синтаксич. парсера
class syntaxException : public ParserException
{
public:
	syntaxException(const string& str)
	{
		Str = "Parser exception: ";
		Str += str;
	}
    syntaxException(const string& str, int line)
    {
    	ostringstream os;
        os << "Parser exception: " << str << ", line: " << line+1 << '!';
        Str = os.str() ;
    }
	syntaxException(int tok, int line)
    {
    	ostringstream os;
        os << "Parser exception: Expected '" << tokenName[tok] << "', line: " << line+1 << '!';
        Str = os.str() ;
    }
};

class RuntimeException : public ParserException
{
public:
	RuntimeException(const string& str)
	{
		Str = "Runtime exception: ";
		Str += str;
	}
};

#endif
