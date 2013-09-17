#ifndef SYNTAX_H_
#define SYNTAX_H_
#include <istream>
#include "interpr.h"
#include "parser.h"

using namespace std;

/*char* error_msg[]=
{
	"Wrong variable or function definition",
}*/
//�� ���������� :c
class syntax
{
	progo* prog;
	TokenParser* tokens;

	func* currfunc;
	table<var>* currvarttable;
	// LR ������
    void eval_exp();
    void eval_exp1();
    void eval_exp2();
    void eval_exp3();
    void eval_exp4();
    void eval_exp5();
    void eval_exp6();
    void eval_exp7();
    void Program();
    void let();
    void Func();
    void parseBlock();
    void ST();
	// ����������� ������
	bool tokenIs(TokenType type, long val)
	{
		return 	(token()->type == type && token()->getLong() == val);
	}
	// �����������
	void checkProgram()
	{
		int main_cnt = 0;
		int size = prog->functions.getSize();
		for (int i=0; i<size; ++i)
		{
			func& f = prog->functions.get(i);
			if (!f.declared)
				error("Function '" + f.name + "' is not defined");
			if (f.name == "main" && f.return_type == INT)
			{
				prog->main = i;
				main_cnt++;
			}
		}
		if (!main_cnt)
			throw syntaxException("There is no 'int main(...)' function");
		if (main_cnt > 1)
			throw syntaxException("There are more than one 'int main(...)' function");
	}
	// �������� �� ������� �����.
	bool containsId(const string id)
	{
		return currfunc->local.contains(id) || prog->global.contains(id);
	}
	// ��������� ����������
	instruct makeVarPoliz(const string id)
	{
		instruct v(TVariable);
		if (currfunc->local.contains(id))
		{
			// Local variable;
			v.local=true;
			v.val=currfunc->local.getPos(id);
		}
		else
		{
			// Global variable
			v.local = false;
			v.val= prog->global.getPos(id);
		}
		return v;
	}
	// ����� ������
    void error(const int number)
    {
        throw syntaxException(number, tokens->getLineNumber());
    }
	// ����� ������
    void error(const string str)
    {
    	error(str.c_str());
    }
	// ����� �����
    Token* token()
    {
        return tokens->currentToken();
    }
    // �������� �����
    void assert_token(TokenType _type, long _val)
    {
    	if (token()->type != _type || (_val != 0 && token()->getLong() != _val))
    		throw syntaxException(_val, tokens->getLineNumber() );
    }

public:
	progo* parse(istream& is);
};

#endif
