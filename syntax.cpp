#include <vector>

#include "syntax.h"
#include "parser.h"
#include "except.h"

using namespace std;

void syntax::let()
//Объявление переменной
{
	assert_token(TYPE_RES, T_VAR);
	//Проверка
	this->tokens->getNextToken();
	if (token()->type != TYPE_RES)
		error("Wrong type, 'int' or 'float' expected");
	ElementTypes type;
	if (token()->getLong() == T_INT)
		type = INT;
	else if (token()->getLong() == T_FLOAT)
		type = FLOAT;
	else
		error("Wrong type, 'int' or 'float' expected");

	this->tokens->getNextToken();
	assert_token(TYPE_ID, 0);
	if (currvarttable->contains(token()->str))
		error("Redeclaration of '" + token()->str +"'");
	currvarttable->put(this->token()->str, var(type, 0.0));
	this->tokens->getNextToken();
	for(;;)
	{
		if (token()->type != TYPE_DELIM)
			break;
        else if (token()->getLong()!=T_COMMA)
			break;
		tokens->getNextToken();
		assert_token(TYPE_ID, 0);
		if (currvarttable->contains(token()->str))
			error("Redeclaration of '" + token()->str +"'");
		currvarttable->put(token()->str, var(type, 0));
		tokens->getNextToken();
	}
}

progo* syntax::parse(istream& is)
{
    this->prog = new progo;
    this->tokens = new TokenParser(is);
    this->tokens->getNextToken();
    Program();
    delete this->tokens;
    return this->prog;
}

void syntax::Program()
{
	while (token()->type != TYPE_DELIM || token()->getLong() != T_EOF )
	{
		if (token()->type != TYPE_RES)
        	error("Wrong definition");
		if (token()->getLong() == T_VAR)
		{
			currvarttable = &(prog->global);
			let();
		}
		else
			Func();
	}
    checkProgram();
}

void syntax::Func()
{
	if (token()->type != TYPE_RES)
		error("Wrong definition");
	func fun;
	string fun_name;
	switch (token()->getLong())
	{
	case T_VOID:
		fun.return_type = VOID;
		break;
	case T_FLOAT:
		fun.return_type = FLOAT;
		break;
	case T_INT:
		fun.return_type = INT;
		break;
	default:
		error("Wrong return type");
	}
	tokens->getNextToken();
	assert_token(TYPE_ID, 0);
	fun_name = token()->str;

	tokens->getNextToken();
	assert_token(TYPE_DELIM, T_LBRACK);

	tokens->getNextToken();
	bool expected = false;
	for(;;)
	{
		if (token()->type == TYPE_DELIM && token()->getLong() == T_RBRACK)
        {
            if (expected)
                error("Unexpected ';");
			break;
        }
        expected =false;
		if (token()->type != TYPE_RES)
			error("Wrong argument type");

		var var1;
		if (token()->getLong() == T_INT)
			var1.type = INT;
		else if (token()->getLong() == T_FLOAT)
			var1.type = FLOAT;
		else
			error("Wrong argument type");
		tokens->getNextToken();
		assert_token (TYPE_ID, 0);
		if (fun.local.contains(token()->str))
			error("Redeclaration of '" + token()->str + "'");
		fun.local.put(token()->str, var1);

		tokens->getNextToken();

		if (token()->type == TYPE_DELIM)
		{
			if (token()->getLong() == T_SEMIC)
			{
				expected = true;
				tokens->getNextToken();
			}
			else if (token()->getLong() != T_RBRACK)
				error("Wrong delimitor");
		}
		else
            error("Wrong delimitor");
	}
	fun.args_count = fun.local.getSize();
	fun.name = fun_name;
	tokens->getNextToken();

	if (token()->type == TYPE_DELIM && token()->getLong() == T_SEMIC)
	{
		fun.declared = false;
		if (prog->functions.contains(fun_name))
		{
			if (!fun.hasParamsEqualTo(prog->functions.get(fun_name)))
				error("Function's already declared!");
		}
		else
			prog->functions.put(fun_name, fun);
		tokens->getNextToken();
		return;
	}
	else
	{
		fun.declared = true;
		if (prog->functions.contains(fun_name))
		{
			func& f = prog->functions.get(fun_name);
			if (!fun.hasParamsEqualTo(f) || f.declared)
				error("Function's already declared!");
			f.declared = true;
			currfunc = &f;
		}
		else
		{
			int i = prog->functions.put(fun_name, fun);
			currfunc = &(prog->functions.get(i));
		}
		parseBlock();
	}
}

void syntax::parseBlock()
{
	assert_token(TYPE_RES, T_BEGIN);

	tokens->getNextToken();

	if (!(token()->type == TYPE_RES && token()->getLong()==T_END))
	{
		ST();

		while ( token()->type == TYPE_DELIM && token()->getLong()==T_SEMIC){
			tokens->getNextToken();
			if (token()->type == TYPE_RES && token()->getLong()==T_END)
				error("';' can't stand before 'end'");
			ST();
		}
	}

	assert_token(TYPE_RES, T_END);

	tokens->getNextToken();
}

void syntax::ST()
{
	if (token()->type == TYPE_RES)
	{
		long t = token()->getLong();
		if (t == T_VAR)
		{
			currvarttable = &(currfunc->local);
			let();
		}
		else if (t == T_IF)
		{
			tokens->getNextToken();
			eval_exp();
			int start = currfunc->code.size();
			currfunc->code.push_back(instruct(TIfNot));
			assert_token(TYPE_RES, T_THEN);
			tokens->getNextToken();
			ST();
			int gt1 = currfunc->code.size();
			if (token()->type == TYPE_RES && token()->getLong() == T_ELSE)
			{
				currfunc->code.push_back( instruct(TGoto) );
				++gt1;

				tokens->getNextToken();
				ST();

				(currfunc->code)[start].val = gt1;
				(currfunc->code)[gt1-1].val = currfunc->code.size();

			}
            else
                (currfunc->code)[start].val = gt1;
		}
		else if (t == T_BEGIN)
		{
			parseBlock();
		}
		else if (t == T_WHILE)
		{
			int start = currfunc->code.size();
			tokens->getNextToken();
			eval_exp();

			assert_token(TYPE_RES, T_DO);

			int gt = currfunc->code.size();
			currfunc->code.push_back( instruct(TIfNot) );

			tokens->getNextToken();
			ST();

			currfunc->code.push_back( instruct(TGoto, start) );
			(currfunc->code)[gt].val = currfunc->code.size();
		}
		else if (t == T_RETURN)
		{
			if (currfunc->return_type == VOID)
				error("Function can't return a value");
			tokens->getNextToken();
			eval_exp();
			currfunc->code.push_back( instruct(TReturn, currfunc->return_type) );
		}
		else if (t == T_WRITE)
		{
			tokens->getNextToken();
			assert_token(TYPE_DELIM, T_LBRACK);
			tokens->getNextToken();
			int params = 0;
			bool expected = false;
			for(;;)
			{
				if (token()->type == TYPE_DELIM && token()->getLong()==T_RBRACK)
				{
					if (expected)
						error("Unexpected comma");
					break;
				}
				expected = false;
				params++;
				if (token()->type==TYPE_LITSTR)
				{
					instruct i = instruct(TStrLit, 0);
					i.str=token()->str;
					currfunc->code.push_back( i );
					tokens->getNextToken();
				}
				else
					eval_exp();

				if (token()->type == TYPE_DELIM)
				{
					if (token()->getLong()==T_COMMA)
					{
						expected = true;
						tokens->getNextToken();
					}
					else if (token()->getLong()!=T_RBRACK)
                        error("You're doing it wrong!");
				}
				else
					error("You're doing it wrong!");
			}
			if (params == 0)
				error("You're doing it wrong!");
			currfunc->code.push_back( instruct(TWrite, params) );
			tokens->getNextToken();
		}
		else if (t == T_READ)
		{
			tokens->getNextToken();
			if (token()->type != TYPE_ID)
				error("You're doing it wrong!");
			string& id = token()->str;
			if (!containsId(id))
				error(id +" is not declared!");
			currfunc->code.push_back( makeVarPoliz(id) );
			currfunc->code.push_back( instruct(TRead) );
			tokens->getNextToken();
		}
		else
			error("You're doing it wrong!");
	}
	else if (token()->type == TYPE_ID)
	{
		string id = token()->str;
		tokens->getNextToken();
		if (token()->type == TYPE_DELIM && token()->getLong() == T_LBRACK)//нашли функцию
		{
			if (!prog->functions.contains(id))
				error(id+"is not declared");

			tokens->getNextToken();
			int params = 0;
			bool expected = false; //Нужен аргумент?
			for(;;)
			{
				if (token()->type == TYPE_DELIM && token()->getLong() == T_RBRACK)
				{
					if (expected)
						error("Unexpected comma");
					break;//Передали все аргументы
				}
				expected = false;
				params++;
				eval_exp();
				if (token()->type == TYPE_DELIM)
				{
					if (token()->getLong()==T_COMMA)
					{
						expected = true;
						tokens->getNextToken();
					}
					else if (token()->getLong()!=T_RBRACK)
						error("You forgot about comma in function call");
				}
				else
					error("You forgot about comma in function call");
			}
			func& f = prog->functions.get(id); // Проверка вызова функции
			if (params != f.args_count)
				error("Wrong arguments in function '" + f.name +"' call");
			currfunc->code.push_back(instruct(TFunction, prog->functions.getPos(id))); //Ложим функцию в стек
			if (f.return_type != VOID)
				currfunc->code.push_back(instruct(TPOP));
				//Без этого не пашет :c
			tokens->getNextToken();
		}
		else if (token()->type == TYPE_OPER && token()->getLong() == T_ASS)
		{
			// =
			if (!containsId(id))
				error(id+" is not declared");
			currfunc->code.push_back(makeVarPoliz(id));
			tokens->getNextToken();
			eval_exp();
			currfunc->code.push_back(instruct(TOperation, T_ASS));
		}
		else
            error("Expected '(' or '='");
	}
    else //Не может быть
		error("You've found a bug! Visit http://github.com/NOOBS-DevTeam/Appendix and open an issue!");
}
// LR такой LR
void syntax::eval_exp()
{
	eval_exp1();
	while (token()->type == TYPE_OPER  && token()->getLong()==T_OR)
	{
		tokens->getNextToken();
		eval_exp1();
		currfunc->code.push_back(instruct(TOperation, T_OR));
	}
}


void syntax::eval_exp1()
{
	eval_exp2();
	while (token()->type == TYPE_OPER && token()->getLong()==T_AND)
	{
		tokens->getNextToken();
		eval_exp2();
		currfunc->code.push_back( instruct(TOperation, T_AND) );
	}
}

void syntax::eval_exp2()
{
	eval_exp7();

	if (token()->type == TYPE_OPER && (token()->getLong() == T_EQ || token()->getLong() == T_NEQ))
	{
		int tt = token()->getLong();
		tokens->getNextToken();
		eval_exp7();
		currfunc->code.push_back(instruct(TOperation,tt));
	}

}

void syntax::eval_exp3()
{
	eval_exp4();

	while (token()->type == TYPE_OPER && (token()->getLong() == T_PLUS || token()->getLong() == T_MINUS))
	{
		int tt = token()->getLong();
		tokens->getNextToken();
		eval_exp4();
		currfunc->code.push_back( instruct(TOperation, tt) );
	}

}

void syntax::eval_exp4()
{
	eval_exp5();

	while (token()->type == TYPE_OPER && (token()->getLong() == T_MUL || token()->getLong() == T_DIV || token()->getLong() == T_MOD ))
	{
		int tt = token()->getLong();
		tokens->getNextToken();
		eval_exp5();
		currfunc->code.push_back( instruct(TOperation, tt) );
	}
}

void syntax::eval_exp5()
{
	eval_exp6();

	if (token()->type == TYPE_OPER && (token()->getLong() == T_DEG))
	{
		int tt = token()->getLong();
		tokens->getNextToken();
		eval_exp5();
		currfunc->code.push_back( instruct(TOperation, tt) );
	}
}

void syntax::eval_exp6()
{
	//If not ID then...
	if (token()->type != TYPE_ID)
	{
		 //If literal
		 if (token()->type == TYPE_LIT)
		 {
			instruct i(TLiteral,INT);
			if (token()->isFloat)
				i.val = FLOAT;
			i.value = token()->value;
			currfunc->code.push_back(i);
			tokens->getNextToken();
		}
		//If operation
		else if (token()->type == TYPE_OPER)
		{
			long op = token()->getLong();

			if (op == T_NOT)
			{/*ЩИТО?*/}
			else if (op == T_PLUS)
			{
				op = T_PLUSUN;
			}
			else if (op == T_MINUS)
			{
				op = T_MINUSUN;
			}
			else
				error("Wrong oper.");

			tokens->getNextToken();
			eval_exp6();

			currfunc->code.push_back(instruct(TOperation, op));

		}
		else if (token()->type == TYPE_DELIM && token()->getLong() == T_LBRACK)
		{
			tokens->getNextToken();
			eval_exp();
			assert_token(TYPE_DELIM, T_RBRACK);
			tokens->getNextToken();

		}
		else
			error ("Wrong expression");

	}
	else
	{ // Identifier
		assert_token(TYPE_ID, 0);

		string id = token()->str;
		int line = tokens->getLineNumber();

		tokens->getNextToken();

		if (token()->type == TYPE_DELIM && token()->getLong() == T_LBRACK) {
			// Function call

			if (!prog->functions.contains(id))
				error(id+" is not declared");

			func& f = prog->functions.get(id);

			if (f.return_type == VOID)
				error("You forgot about 'return' in "+f.name+"!");

			tokens->getNextToken();

			int params = 0;
			bool expected = false;
			for(;;)
			{
				if (token()->type == TYPE_DELIM && token()->getLong() == T_RBRACK)
				{
					if (expected)
						error("Unexpected comma found");
					break;
				}
				expected =false;
				params++;
				eval_exp();

				if (token()->type == TYPE_DELIM)
				{
					if (token()->getLong()==T_COMMA)
					{
						expected = true;
						tokens->getNextToken();
					}
					else if (token()->getLong()!=T_RBRACK)
						error("You forgot about comma");
				}
				else
					error("You forgot about comma");
			}
			tokens->getNextToken();

			if (params != f.args_count)
				error("Wrong arguments in '" + f.name +"' call");

			currfunc->code.push_back( instruct(TFunction, prog->functions.getPos(id)) );

		}
		else
		{
			// Identifier
			if ( !containsId(id) )
				throw syntaxException(id+" is not declared", line);

			currfunc->code.push_back( makeVarPoliz(id) );

		}

	}
}

void syntax::eval_exp7()//Логические опер.
{
	eval_exp3();
	if (token()->type == TYPE_OPER && (token()->getLong() == T_GRT ||
										token()->getLong() == T_GRTEQ ||
										token()->getLong() == T_LSS ||
										token()->getLong() == T_LSSEQ ))
	{
		int tt = token()->getLong();
		tokens->getNextToken();
		eval_exp3();
		currfunc->code.push_back( instruct(TOperation, tt) );
	}
}
