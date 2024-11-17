#pragma once
#include "ATN.h"

enum class AmbigEnum {one, ambig, none};

struct ATNset;
struct Stack
{
private:
	vector<shared_ptr<ATNnode>> v;
	ATNset* atnSet;
public:
	bool empty() { return v.empty(); }
	shared_ptr<ATNnode> pop() {
		shared_ptr<ATNnode> elem = v.back();
		v.pop_back();
		return elem;
	}

	void push(shared_ptr<ATNnode> elem) {
		v.push_back(elem);
	}

	size_t size()
	{
		return v.size();
	}

	string toString();
	void init(string str);
	Stack(ATNset* atnSet) : atnSet(atnSet) {}
	void cloneFrom(shared_ptr<Stack> otherStack)
	{
		v = otherStack->v;
	}
};

struct Configuration
{
private:
	ATNset* atnset;
	void weakCloneFrom(Configuration* other);
public:
	shared_ptr<Configuration> prev;
	Configuration(ATNset* atnset) : atnset(atnset) {
		stack = make_shared<Stack>(atnset);
	}
	vector<shared_ptr<Configuration>> closure(bool predict);
	vector<shared_ptr<Configuration>> move(shared_ptr<SymbolT> symterm);
	shared_ptr<ATNnode> atnNode;
	int prodNumber;
	shared_ptr<Stack> stack;
	string toString();
};

struct DFA;

class ParsConfigs
{
private:
	ATNset *atnset;
	vector<shared_ptr<Configuration>> v;
public:
	ParsConfigs(ATNset *atnset);
	void filterProd(int prodNumber);
	void closureOneOrRep();
	void denyOtherTerminals(shared_ptr<SymbolT> symbolToEat);
	size_t size() { return v.size(); }
	shared_ptr<Configuration> get(int i) { return v[i]; }
	void consume(shared_ptr<SymbolT> symbolToEat);
	AmbigEnum checkAmbig(int &retProdNumber);
};

struct DFAstate
{
private:
	DFA* dfa;
public:
	shared_ptr<DFAstate> next;
	vector<shared_ptr<Configuration>> v;
	DFAstate(DFA* dfa) : dfa(dfa) {}
	void startState();
	void startStateFromParser(shared_ptr<ParsConfigs> configs);
	void closure();
	void write();
	shared_ptr<DFAstate> target(shared_ptr<SymbolT> symterm);
	AmbigEnum checkAmbig(int &retProdNumber);
};

struct DFA
{
private:
	shared_ptr<DFAstate> startState;
	shared_ptr<DFAstate> tailstate;
public:
	ATNset* atnset;
	DFA(ATNset* atnset, shared_ptr<ParsConfigs> configs);
	void predict(string termName);
	void goInput(string input);
	void write();
	AmbigEnum checkAmbig(int &retProdNumber)
	{
		return tailstate->checkAmbig(retProdNumber);
	}
};

