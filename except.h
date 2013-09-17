#ifndef EXCEPT_H_
#define EXCEPT_H_
#include <exception>
#include <string>
#include <sstream>

using namespace std;

extern char* tokenName[];
// ������� ����� ����������
class ParserException : public exception
{
protected:
	string Str; //����� ������
public:
	ParserException(const string& str = "") : Str(str)
	{
		//�����������
	}
	virtual const char* what() const throw()
	{
		return Str.c_str();
	}

	virtual ~ParserException() throw () {}
};
// ���������� ������� �������
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
// ���������� ���������. �������
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
