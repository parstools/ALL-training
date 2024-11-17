#pragma once
#include "DFA.h"

class Parser
{
private:
	string input;
	ATNset atnset;
	Grammar grammar;
	shared_ptr<ParsConfigs> configs;
	int predict(string restInput);
public:
	Parser(string gramFile);
	void goInput(string input);
};

