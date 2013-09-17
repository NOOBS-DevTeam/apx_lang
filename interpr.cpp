#include <cmath>
#include <iostream>
#include <queue>
#include <vector>

#include "interpr.h"
#include "except.h"
#include "parser.h"
#include "syntax.h"
using namespace std;


long double Interpreter::getValue(table<var>& local)
{
    instruct& in = exec_stack.top();
    exec_stack.pop();
    if (in.inst_type == TLiteral)
    {
        return in.value;
	}
	else if (in.inst_type == TVariable)
	{
		if (in.local)
			return local.get(in.val).value;
		else
			return progr->global.get(in.val).value;
	}
	return 0;
}

ElementTypes Interpreter::getType(table<var>& local)
{
	instruct in = exec_stack.top();
	if (in.inst_type == TLiteral)
		return ElementTypes(in.val);
    else if (in.inst_type == TVariable)
    {
		if (in.local)
			return local.get(in.val).type;
		else
			return progr->global.get(in.val).type;
    }
	return VOID;
}

void Interpreter::setValue(table<var>& local, long double v)
{
	instruct& in = exec_stack.top();
	exec_stack.pop();
    if (in.inst_type == TVariable)
    {
		if (in.local)
		{
			var& var1 = local.get(in.val);
			var1.value = (var1.type == INT) ? ( (long long)v ):(v);
		}
		else
		{
			var& var1 = progr->global.get(in.val);
			var1.value = (var1.type == INT) ? ( (long long)v ):(v);
		}
	}
}
string Interpreter::getString()
{
	instruct& in = exec_stack.top();
	exec_stack.pop();
    if (in.inst_type == TStrLit)
		return in.str;
}

void Interpreter::proc(int f) // main
{
	unsigned where = 0;
	func& func1 = progr->functions.get(f);
	table<var> local = func1.local;
	if (f != progr->main && func1.args_count > 0)
	{
		// args

        for (int i = func1.args_count-1; i>=0; --i)
        {
            var& v = local.get(i);
            long double d =getValue(local);
			v.value = (v.type == INT)? ((long long)d):d;
		}
	}

	bool was_return = false;

	while(where != func1.code.size())
	{
		instruct instr = func1.code[where];
		switch (instr.inst_type)
		{
        case TVariable:
        case TStrLit:
        case TLiteral:
            exec_stack.push(instr);
            break;
        case TFunction:
        {
            func& f = progr->functions.get(instr.val);
            if (f.args_count>0)
            {
                vector<long double> vec(f.args_count);
                for (int i=0; i<f.args_count; ++i)
                    vec[i] = getValue(local);
                for (int i=f.args_count-1; i>=0; --i)
                {
                    instruct in (TLiteral, FLOAT);
                    in.value = vec[i];
                    exec_stack.push(in);
                }
            }
            proc(instr.val);
            break;
        }
        case TIfNot:
        {
            long double v = getValue(local);
            if (v==0)
            {
                where = instr.val;
                continue;
            }
            break;
        }
        case TGoto:
        {
            where = instr.val;
            continue;
        }
        case TReturn:
        {
            was_return = true;
            long double d = getValue(local);
            instruct in(TLiteral);
            in.val = instr.val;
            in.value = (instr.val == INT)? ((long long)d) : (d);
            exec_stack.push(in);
            break;
        }
        case TRead:
        {
            long double v;
            cin >> v;
            setValue(local, v);
            break;
        }
        case TWrite:
        {
            int num = instr.val;
            deque<instruct> params;
            for (int i=0; i<num; ++i)
            {
                params.push_front(exec_stack.top());
                exec_stack.pop();
            }
            for (int i=0; i<num; ++i)
            {
                instruct& curr = params[i];
                if (curr.inst_type == TStrLit)
                    cout << params[i].str;
                else if (curr.inst_type == TLiteral)
                    cout << params[i].value;
                    else if (curr.inst_type == TVariable)
                        if (curr.local)
                            cout << local.get(curr.val).value;
                        else
                            cout << progr->global.get(curr.val).value;
            }
            break;
        }
        case TPOP:
            exec_stack.pop();
            break;
        case TOperation:
        {
            switch (instr.val)
            {
            case T_ASS:
            {
                long double d = getValue(local);
                setValue(local, d);
                break;
            }
            case T_OR:
            {
                long double d1 = getValue(local);
                long double d2 = getValue(local);
                instruct in(TLiteral);
                in.val = INT;
                in.value = (d1 != 0 || d2 != 0) ? 1 : 0;
                exec_stack.push(in);
                break;
            }
            case T_AND:
            {
                long double d1 = getValue(local);
                long double d2 = getValue(local);
                instruct in(TLiteral);
                in.val = INT;
                in.value = (d1 != 0 && d2 != 0) ? 1 : 0;
                exec_stack.push(in);
                break;
            }
			case T_PLUS:
			{
                ElementTypes t1 = getType(local);
				long double d1 = getValue(local);
				ElementTypes t2 = getType(local);
				long double d2 = getValue(local);
				ElementTypes t = ( t1==FLOAT || t2==FLOAT) ? FLOAT:INT;
				instruct in(TLiteral);
				in.val = t;
				long double res = d1 + d2;
				in.value = (t == INT) ? ((long long)res) : res;
				exec_stack.push(in);
				break;
			}
			case T_MINUS:
            {
                ElementTypes t1 = getType(local);
                long double d1 = getValue(local);
                ElementTypes t2 = getType(local);
				long double d2 = getValue(local);
				ElementTypes t = ( t1==FLOAT || t2==FLOAT) ? FLOAT:INT;
				instruct in(TLiteral);
				in.val = t;
				long double res = d2 - d1;
				in.value = (t == INT) ? ((long long)res) : res;
				exec_stack.push(in);
				break;
			}
			case T_MUL: // *
			{
				ElementTypes t1 = getType(local);
				long double d1 = getValue(local);
				ElementTypes t2 = getType(local);
				long double d2 = getValue(local);
				ElementTypes t = ( t1==FLOAT || t2==FLOAT) ? FLOAT:INT;
                instruct in(TLiteral);
				in.val = t;
				long double res = d2*d1;
				in.value = (t == INT) ? ((long long)res) : res;
				exec_stack.push(in);
				break;
			}
			case T_DIV: // /
			{
                ElementTypes t1 = getType(local);
				long double d1 = getValue(local);
				ElementTypes t2 = getType(local);
				long double d2 = getValue(local);
				ElementTypes t = ( t1==FLOAT || t2==FLOAT) ? FLOAT:INT;
                instruct in(TLiteral);
				in.val = t;
				if (d1 == 0)
					throw RuntimeException("OMG!");
				long double res = 0;
				if (t == INT)
					res = ((long long)d2) / ((long long) d1);
				else
                    res = d2/d1;
				in.value = res;
				exec_stack.push(in);
				break;
			}
			case T_MOD:// mod
			{
				ElementTypes t1 = getType(local);
				long double d1 = getValue(local);
				ElementTypes t2 = getType(local);
				long double d2 = getValue(local);
                ElementTypes t = ( t1==FLOAT || t2==FLOAT) ? FLOAT:INT;
				if (t==FLOAT)
					throw RuntimeException("Operation '%' supports only integer args");
				instruct in(TLiteral, INT);
				if (d1 == 0)
					throw RuntimeException("OMG!");
				long double res = ((long long)d2) % ((long long) d1);
				in.value = res;
				exec_stack.push(in);
				break;
			}
			case T_DEG:// ^
			{
				ElementTypes t1 = getType(local);
				long double d1 = getValue(local);
				ElementTypes t2 = getType(local);
				long double d2 = getValue(local);
				ElementTypes t = ( t1==FLOAT || t2==FLOAT) ? FLOAT:INT;
				instruct in(TLiteral);
				in.val = t;
				long double res = pow(d2,d1);
				//if (errno)
					//throw RuntimeException("You're doing it wrong!");
				in.value = (t == INT) ? ((long long)res) : res;
				exec_stack.push(in);
				break;
			}
			case T_NOT:// !
            {
                long double d1 = getValue(local);
				instruct in(TLiteral);
				in.val = INT;
				in.value = (d1 == 0) ? 1 : 0;
				exec_stack.push(in);
				break;
			}
			case T_EQ://==
			{
				long double d1 = getValue(local);
				long double d2 = getValue(local);
				instruct in(TLiteral);
				in.val = INT;
				in.value = (d1 == d2 ) ? 1 : 0;
				exec_stack.push(in);
				break;
			}
            case T_NEQ:// !=
            {
				long double d1 = getValue(local);
				long double d2 = getValue(local);
				instruct in(TLiteral);
				in.val = INT;
                in.value = (d1 != d2 ) ? 1 : 0;
				exec_stack.push(in);
				break;
			}
			case T_LSS://<
			{
				long double d1 = getValue(local);
				long double d2 = getValue(local);
				instruct in(TLiteral);
				in.val = INT;
                in.value = (d2 < d1 ) ? 1 : 0;
				exec_stack.push(in);
				break;
			}
			case T_LSSEQ: //<=
			{
				long double d1 = getValue(local);
				long double d2 = getValue(local);
				instruct in(TLiteral);
				in.val = INT;
				in.value = (d2 <= d1 ) ? 1 : 0;
				exec_stack.push(in);
				break;
			}
			case T_GRT: //>
			{
				long double d1 = getValue(local);
				long double d2 = getValue(local);
				instruct in(TLiteral);
				in.val = INT;
				in.value = (d2 > d1 ) ? 1 : 0;
                exec_stack.push(in);
				break;
			}
			case T_GRTEQ: //>=
			{
				long double d1 = getValue(local);
				long double d2 = getValue(local);
				instruct in(TLiteral);
				in.val = INT;
				in.value = (d2 >= d1 ) ? 1 : 0;
				exec_stack.push(in);
				break;
			}
			case T_MINUSUN:
			{
				ElementTypes t1 = getType(local);
				long double d1 = getValue(local);
                instruct in(TLiteral);
				in.val = t1;
				in.value = -d1;
				exec_stack.push(in);
				break;
			}
			case T_PLUSUN:
				break;
			default:;
            }

		break;
		}
    default:
		throw RuntimeException("Smth went wrong :c");
	}

    where++;
    if (was_return)
        break;
    }
	// Возврат значения
	if (func1.return_type != VOID)
	{
		if (!was_return)
		{
			instruct in(TLiteral);
			in.val = func1.return_type;
			in.value = 0;
			exec_stack.push(in);
		}
	}
}
