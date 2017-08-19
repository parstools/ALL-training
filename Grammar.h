#pragma once
#include <string>
#include <vector>
#include <memory>
using namespace std;

enum class RepEnum { once, maybe, star, plus };

struct Symbol
{
	int number;
	string name;
	virtual bool isTerm() = 0;
	virtual bool isEps() { return false; };
	Symbol(string name, int number) : name(name), number(number) { }
};

enum class TKind { eps, eof, usuall };

struct SymbolT: public Symbol
{
	TKind kind;
	bool isTerm() { return true; }
	bool isEps() { return kind==TKind::eps; };
	SymbolT(string name, int number) : Symbol(name, number) { kind = TKind::usuall; }
	void setEof() { kind = TKind::eof; }
	void setEps() { kind = TKind::eps; }
};

struct SymbolNT : public Symbol
{
	bool isTerm() { return false; }
	SymbolNT(string name, int number) : Symbol(name,number) { }
};

class Grammar;

class Prod
{
	Grammar *ownerGram;
public:
	shared_ptr<SymbolNT> leftsym;
	vector<shared_ptr<Symbol>> rhs;
	vector<RepEnum> repvec;
	Prod(Grammar* ownerGram, shared_ptr<SymbolNT> leftsymb) : ownerGram(ownerGram), leftsym(leftsymb) {}
	void read(string line);
};

class Grammar
{
private:
	void gatherNonterminals(vector<string> &lines);
public:
	shared_ptr<SymbolNT> getNT(string name);
	shared_ptr<SymbolT> getT(string name);
	vector<vector<Prod>> ntermProd;
	vector<shared_ptr<SymbolNT>> nonterminals;
	vector<shared_ptr<SymbolT>> terminals;
	int numNTerm;
	Grammar();
	~Grammar();
	void read(vector<string> &lines);
	void read(string filename);
};

