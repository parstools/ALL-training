#include "Parser.h"
#include "Exception.h"

Parser::Parser(string gramFile)
{
	grammar.read(gramFile);
	atnset.makeFrom(&grammar);
	configs = make_shared<ParsConfigs>(&atnset);
	//atnset.writeDot();
}

int Parser::predict(string restInput)
{
	shared_ptr<DFA> dfa = make_shared<DFA>(&atnset, configs);
	int retProdNumber = -1;
	for (int i = 0; i < restInput.length(); i++)
	{
		char c = restInput[i];
		dfa->goInput(string(1, c));
		dfa->write();
		AmbigEnum ambig = dfa->checkAmbig(retProdNumber);
		if (ambig != AmbigEnum::one)
		{
			printf("ambig\n");
		}
		else
		{
			printf("ok\n");
			return retProdNumber;
		}
	}
	return -1;
}

void Parser::goInput(string input)
{
	this->input = input;
	size_t len = input.length();
	int pos = 0;
	while (pos < len)
	{
		shared_ptr<SymbolT> symbolToEat = atnset.ownerGram->getT(string(1, input[pos]));
		int retProdNumber;
		AmbigEnum ambig = configs->checkAmbig(retProdNumber);
		if (ambig == AmbigEnum::none) break;
		if (ambig == AmbigEnum::one)
		{
			printf("consume %s\n", symbolToEat->name.c_str());
			configs->consume(symbolToEat);
			pos++;
		}
		else
		{
			int retProdNumber = predict(input.substr(pos));
			configs->filterProd(retProdNumber);
			configs->closureOneOrRep();
			configs->denyOtherTerminals(symbolToEat);
		}
	}
}
