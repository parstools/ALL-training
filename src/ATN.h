#pragma once
#include "Grammar.h"
#include <memory>
using namespace std;

struct ATNnode;

struct ATNedge
{
	shared_ptr<ATNnode> next;
	shared_ptr<Symbol> label;
	ATNedge() {}
	ATNedge(shared_ptr<ATNnode> next, shared_ptr<Symbol> label):next(next), label(label) {};
};


struct ATN;
struct ATNnode
{
private:
	ATN* atn;
public:
	RepEnum rep;
	bool starting;
	int number;
	bool tail = false;
	ATNedge edge;
	int prodNumber;
	ATNnode(int number, bool starting, ATN* atn):number(number),starting(starting),atn(atn) {}
	string toString();
};

struct ATNset;

struct ATN
{
	int startIndex, endIndex;
	shared_ptr<SymbolNT> startNT;
	vector<shared_ptr<ATNnode>> alternatives;
	ATN(vector<Prod> &ntermProd, int &startNumber);
	string writeDot();
	shared_ptr<ATNnode> findByNumber(int n);
};

struct ATNset
{
	Grammar *ownerGram;
	vector<shared_ptr<ATN>> atns;
	void makeFrom(Grammar* grammar);
	void writeDot();
	shared_ptr<ATNnode> findNode(string name);
};
