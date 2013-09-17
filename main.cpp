#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <string.h>

#include "parser.h"
#include "except.h"
#include "interpr.h"
#include "syntax.h"

using namespace std;

int main(int argc, char **argv)
{

	string file;
	int i = 1;

	if (argc == 1 || argc > 2)
		return 1;
	file = argv[i];

	try
	{
		ifstream input(file.c_str());
		syntax syntaticParser;
		progo* p = syntaticParser.parse(input);
		Interpreter interp(p);
		int r =  interp.run();
		delete p;
		return r;
	}
	catch (ParserException& e)
	{
		cerr << e.what() << endl;
		exit(1);
	}
	return 0;
}
