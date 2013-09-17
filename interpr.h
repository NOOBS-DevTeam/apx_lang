#ifndef INTERPR_H_
#define INTERPR_H_
#include <iostream>
#include <stack>
#include <vector>
#include <string.h>
#include <map>
#include "except.h"

using namespace std;

extern bool debug;
extern int global_error;
// ��� ����
enum ElementTypes
{
    VOID,
    INT,
    FLOAT,
    STRING
};
// ��� ����������
enum InstType
{
    TFunction,
    TLiteral,
    TVariable,
    TStrLit,
    TOperation,
    TIfNot,
    TGoto,
    TReturn,
    TWrite,
    TRead,
    TPOP
};

struct progo;
struct func;
struct instruct;
struct var;
// ������� ������
template <class T> class table
{
	vector<T> items;
	map<string, int> index;

public:
	void clearIndex()
	{
		index.clear();
	}

	bool contains(const string& str)
	{
		map<string, int>::iterator pos = index.find(str);
		if (pos != index.end() )
			return true;
		return false;
	}

	int put(const string& str, const T& item)
	{
		items.push_back(item);
		index.insert(make_pair(str, items.size() - 1 ));
		return items.size() - 1;
	}

	int getPos(const string& str)
	{
		return index[str];
	}

	T& get(const string& str)
	{
		return items[index[str]];
	}

	T& get(int i)
	{
		return items[i];
	}

	int getSize() const
	{
		return items.size();
	}
};
// ������ ����������
struct var
{
	ElementTypes type;
	long double value;
	var(ElementTypes t = FLOAT, long double v = 0) : type(t), value(v)
	{}
};
// ������ ����������
struct instruct
{
    InstType inst_type;
    int val;
    long double value;
    bool local;
    string str;
    instruct(InstType t, int v=0) : inst_type(t), val(v) {};
};
// ������ �������
struct func
{
    ElementTypes return_type;
    int args_count;
    //����������
    table<var> local;
    //����
    vector<instruct> code;
    bool declared;
    string name;

    func() : args_count(0) {}

    bool hasParamsEqualTo(func& f)
    {
    	bool ret = false;
    	if (args_count == f.args_count && return_type == f.return_type)
    		ret = true;
    	for (int i=0; i<args_count; ++i)
    		ret &= local.get(i).type == f.local.get(i).type;
    	return ret;
    }

};
// ������ �����
struct progo
{
    table<var> global; // ������� ����. ����������
    table<func> functions; //������� ������ �����
    int main;
};

class Interpreter
{
	progo* progr;
	void proc(int);
	stack<instruct> exec_stack;
	long double getValue(table<var>& local);
	ElementTypes getType(table<var>& local);
	void setValue(table<var>& local, long double);
	instruct getInstruction(InstType type);
	string getString();

public:
	Interpreter(progo* program) : progr(program)
	{

	}

	int run()
	{
		try
		{
            proc(progr->main);
		}

		catch (ParserException&)
		{
			throw;
		}

		instruct i = exec_stack.top();

		return int(exec_stack.top().value); // ������� return
	}
};

#endif
