#include <iostream>
#include <fstream>
#include "Grammar.h"
#include <algorithm>
#include "Exception.h"
#include "StrUtils.h"
#include <cassert>


void Prod::read(string line)
{
	for (int j = 3; j < line.length(); j++)
	{
		char symbolChar = line[j];
		shared_ptr<Symbol> sym;
		if (symbolChar >= 'A' && symbolChar <= 'Z' || symbolChar >= 'a' && symbolChar <= 'z')
		{
			if (symbolChar >= 'A' && symbolChar <= 'Z')
			{
				sym = ownerGram->getNT(string(1, symbolChar));
			}
			else if (symbolChar >= 'a' && symbolChar <= 'z')
			{
				sym = ownerGram->getT(string(1, symbolChar));
			}
			rhs.push_back(sym);
			if (j > 3)
				repvec.push_back(RepEnum::once);
		}
		else if (symbolChar == '?' || symbolChar == '*' || symbolChar == '+')
		{
			if (j==3)throw Exception("First give char: " + line);
			switch (symbolChar)
			{
			case '?': repvec.push_back(RepEnum::maybe); break;
			case '*': repvec.push_back(RepEnum::star); break;
			default: repvec.push_back(RepEnum::plus);
			}
		}
		else throw Exception("Symbol must be upper or lower letter or ?+* :" + line);
	}
	if (repvec.size()<rhs.size())
		repvec.push_back(RepEnum::once);
}

Grammar::Grammar()
{
	shared_ptr<SymbolT> eps = make_shared<SymbolT>("",0);
	terminals.push_back(eps);
	shared_ptr<SymbolT> eof = make_shared<SymbolT>("$",1);
	terminals.push_back(eof);
}


Grammar::~Grammar()
{
}


shared_ptr<SymbolNT> Grammar::getNT(string name)
{
	for (int i = 0; i < nonterminals.size(); i++)
	{
		if (nonterminals[i].get()->name == name)
			return nonterminals[i];
	}
	throw Exception("not found nonterminal " + name);
}

shared_ptr<SymbolT> Grammar::getT(string name)
{
	for (int i = 0; i < terminals.size(); i++)
	{
		if (terminals[i].get()->name == name)
			return terminals[i];
	}
	shared_ptr<SymbolT> term = make_shared<SymbolT>(name, terminals.size());
	terminals.push_back(term);
	return term;
}

void Grammar::gatherNonterminals(vector<string> &lines)
{
	vector<char> nonterminalNames;
	for (int i = 0; i < lines.size(); i++)
	{
		string line = lines[i];
		char ntnameCh = line[0];
		vector<char>::iterator it = find(nonterminalNames.begin(), nonterminalNames.end(), ntnameCh);
		if (it == nonterminalNames.end())
		{
			nonterminalNames.push_back(ntnameCh);
			shared_ptr<SymbolNT> ntsym = make_shared<SymbolNT>(string(1, ntnameCh), nonterminals.size());
			nonterminals.push_back(ntsym);
		}
	}
	ntermProd.resize(nonterminals.size());
}

void Grammar::read(vector<string> &lines)
{
	gatherNonterminals(lines);
	for (int i = 0; i < lines.size(); i++)
	{
		string line = lines[i];
		string ntname = string(1, line[0]);
		shared_ptr<SymbolNT> ntsym = getNT(ntname);
		Prod prod(this, ntsym);
		prod.read(line);
		int ntNum = ntsym.get()->number;
		ntermProd[ntNum].push_back(prod);
	}
}

void Grammar::read(string filename)
{
	ifstream infile("../samples/" + filename);
	string line;
	skipBOM(infile);
	vector<string> lines;
	while (getline(infile, line)) {
		line = trim(line);
		if (line.length() == 0) continue;
		if (line[0] == '#') continue;
		if (line.find("->") != 1)
			throw Exception("must be '->' after one nonterminal, is: " + line);
		lines.push_back(line);
		cout << line.c_str() << endl;
	}
	read(lines);
}
