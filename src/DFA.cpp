#include <algorithm> // remove and remove_if
#include "DFA.h"
#include "StrUtils.h"
#include "Exception.h"

string Stack::toString()
{
	if (v.size() == 0)
		return "[]";
	else if (v.size() == 1)
		return v[0]->toString();
	else
	{
		string res = "{";
		for (int i = 0; i < v.size(); i++)
		{
			if (i > 0) res += ",";
			res += v[i]->toString();
		}
		res += "}";
		return res;
	}
}

void Stack::init(string str)
{
	v.clear();
	int pos = 0;
	if (str == "") return;
	while (pos != string::npos)
	{
		int pos1 = string::npos;
		if (str[pos + 1] == '{')
		{
			pos1 = str.find('}', pos + 1);
			pos1 = str.find('P', pos1 + 1);
		}
		else
			pos1 = str.find('P', pos + 1);
		string name = str.substr(pos, pos1 - pos);
		shared_ptr<ATNnode> atnNode = atnSet->findNode(name);
		v.push_back(atnNode);
		pos = pos1;
	}
}

DFA::DFA(ATNset* atnset, shared_ptr<ParsConfigs> configs): atnset(atnset) {
	startState = make_shared<DFAstate>(this);
	//startState->startState();
	startState->startStateFromParser(configs);
	startState->closure();
	tailstate = startState;
};

void DFA::predict(string termName)
{
	shared_ptr<SymbolT> symterm = atnset->ownerGram->getT(termName);
	if (!symterm) throw Exception("Unknown symbol  " + termName);
	shared_ptr<DFAstate> newstate = tailstate->target(symterm);
	tailstate->next = newstate;
	tailstate = newstate;
}

void DFA::goInput(string input)
{
	for (int i = 0; i < input.size(); i++)
	{
		bool b = false;
		string name = string(1, input[i]);
		predict(name);
	}
}

void DFAstate::startState()
{
	shared_ptr<ATN> atn = dfa->atnset->atns[0];
	for (int i = 0; i < atn->alternatives.size(); i++)
	{
		shared_ptr<Configuration> config = make_shared<Configuration>(dfa->atnset);
		config->atnNode = atn->alternatives[i];
		config->prodNumber = i;
		//empty stack
		v.push_back(config);
	}
}

void DFAstate::startStateFromParser(shared_ptr<ParsConfigs> configs)
{
	for (int i = 0; i < configs->size(); i++)
	{
		shared_ptr<Configuration> config = make_shared<Configuration>(dfa->atnset);
		shared_ptr<Configuration> parsconfig = configs->get(i);
		config->atnNode = parsconfig->atnNode;
		config->prodNumber = parsconfig->prodNumber;
		config->stack->cloneFrom(parsconfig->stack);
		v.push_back(config);
	}
}

AmbigEnum DFAstate::checkAmbig(int &retProdNumber)
{
	if (v.size() == 0)
	{
		retProdNumber = -1;
		return AmbigEnum::none;
	}
	retProdNumber = v[0]->prodNumber;
	for (int i = 1; i < v.size(); i++)
	{
		if (v[i]->prodNumber != retProdNumber)
		{
			retProdNumber = -1;
			return AmbigEnum::ambig;
		}
	}
	return AmbigEnum::one;
}

AmbigEnum ParsConfigs::checkAmbig(int &retProdNumber)
{
	if (v.size() == 0)
	{
		retProdNumber = -1;
		return AmbigEnum::none;
	}
	retProdNumber = v[0]->prodNumber;
	for (int i = 1; i < v.size(); i++)
	{
		if (v[i]->prodNumber != retProdNumber)
		{
			retProdNumber = -1;
			return AmbigEnum::ambig;
		}
	}
	return AmbigEnum::one;
}

string Configuration::toString()
{
	string str = string_format("(%s,%d,%s)", atnNode->toString().c_str(), prodNumber + 1, stack->toString().c_str());
	if (prev)
		str += " -> " + prev->toString();
	return str;
}

///predict - not set new prodNumber
//repetition nothing add to closure, option does
vector<shared_ptr<Configuration>> Configuration::closure(bool predict)
{
	vector<shared_ptr<Configuration>> res;
	if (atnNode->tail)
	{
		if (stack->empty())
			return res;
		shared_ptr<Configuration> r = make_shared<Configuration>(atnset);
		r->stack->cloneFrom(stack);
		r->atnNode = r->stack->pop();
		if (predict)
			r->prodNumber = prodNumber;
		else
			r->prodNumber = r->atnNode->prodNumber;
		res.push_back(r);
	}
	else if (!atnNode->edge.label->isTerm())
	{
		int number = atnNode->edge.label.get()->number;
		shared_ptr<ATN> atn = atnset->atns[number];
		for (int i = 0; i < atn->alternatives.size(); i++)
		{
			shared_ptr<Configuration> r = make_shared<Configuration>(atnset);
			r->atnNode = atn->alternatives[i];
			if (predict)
				r->prodNumber = prodNumber;
			else
				r->prodNumber = i;
			r->stack->cloneFrom(stack);
			r->stack->push(atnNode->edge.next);
			res.push_back(r);
		}
	}
	if (!atnNode->tail)
	{
		shared_ptr<ATNnode> nextNode = atnNode->edge.next;
		if (nextNode->rep == RepEnum::maybe || nextNode->rep == RepEnum::star)
		{
			shared_ptr<Configuration> nextConfig = make_shared<Configuration>(atnset);
			nextConfig->weakCloneFrom(this);
			nextConfig->atnNode = nextNode;
			res.push_back(nextConfig);
			vector<shared_ptr<Configuration>> ret = nextConfig->closure(predict);
			res.insert(res.end(), ret.begin(), ret.end());
		}
	}
	return res;
}


void Configuration::weakCloneFrom(Configuration* other)
{
	atnNode = other->atnNode;
	atnset = other->atnset;
	prev = other->prev;
	prodNumber = other->prodNumber;
	stack->cloneFrom(other->stack);
}

///in usual cases return only one Configuration, but repetition give other options
vector<shared_ptr<Configuration>> Configuration::move(shared_ptr<SymbolT> symterm)
{
	vector<shared_ptr<Configuration>> res;
	if (atnNode->edge.label == symterm)
	{
		shared_ptr<Configuration> r = make_shared<Configuration>(atnset);
		r->atnNode = atnNode->edge.next;
		r->prodNumber = prodNumber;
		r->stack->cloneFrom(stack);
		res.push_back(r);
		if (atnNode->rep == RepEnum::plus || atnNode->rep == RepEnum::star)
		{
			shared_ptr<Configuration> rclone = make_shared<Configuration>(atnset);
			rclone->weakCloneFrom(this);
			res.push_back(rclone);
		}
		if (atnNode->rep == RepEnum::maybe || atnNode->rep == RepEnum::star)
		{
			//recursive
			vector<shared_ptr<Configuration>> ret = r->move(symterm);
			res.insert(res.end(), ret.begin(), ret.end());
		}
	}
	return res;
}

void DFAstate::closure()
{
	int n = 0;
	while (n < v.size())
	{
		shared_ptr<Configuration> config = v[n];
		vector<shared_ptr<Configuration>> ret = config->closure(true);
		for (int i = 0; i < ret.size(); i++)
			ret[i]->prev = config;
		v.insert(v.end(), ret.begin(), ret.end());
		n++;
	}
}


shared_ptr<DFAstate> DFAstate::target(shared_ptr<SymbolT> symterm)
{
	shared_ptr<DFAstate> result = make_shared<DFAstate>(dfa);
	for (int i = 0; i < v.size(); i++)
	{
		shared_ptr<Configuration> config = v[i];
		vector<shared_ptr<Configuration>> retconfig = config->move(symterm);
		for (int i = 0; i < retconfig.size(); i++)
			retconfig[i]->prev = config;
		result->v.insert(result->v.end(), retconfig.begin(), retconfig.end());
	}
	result->closure();
	return result;
}

void DFAstate::write()
{
	for (int i = 0; i < v.size(); i++)
	{
		shared_ptr<Configuration> config = v[i];
		printf("%s\n", config->toString().c_str());
			//printf("%d,%d,%d\n", config->atnNode->number, config->prodNumber, config->stack.size());
	}
	printf("\n");
}

void DFA::write()
{
	shared_ptr<DFAstate> state = startState;
	while (state)
	{
		state->write();
		state = state->next;
	}
}

ParsConfigs::ParsConfigs(ATNset *atnset) :atnset(atnset)
{
	shared_ptr<ATN> atn = atnset->atns[0];
	for (int i = 0; i < atn->alternatives.size(); i++)
	{
		shared_ptr<Configuration> config = make_shared<Configuration>(atnset);
		config->atnNode = atn->alternatives[i];
		config->prodNumber = i;
		//empty stack
		v.push_back(config);
	}
}

void ParsConfigs::consume(shared_ptr<SymbolT> symbolToEat)
{
	vector<shared_ptr<Configuration>> ret, tempv;
	for (int i = 0; i < v.size(); i++)
	{
		ret = v[i]->move(symbolToEat);
		tempv.insert(tempv.end(), ret.begin(), ret.end());
	}
	v = tempv;
}

bool is_null(shared_ptr<Configuration> config)
{
	return config == nullptr;
}

void ParsConfigs::filterProd(int prodNumber)
{
	for (int i = 0; i < v.size(); i++)
	{
		shared_ptr<Configuration> cfg = v[i];
		if (cfg->prodNumber != prodNumber)
			v[i] = nullptr;
	}
	v.erase(std::remove_if(v.begin(), v.end(), is_null), v.end());
}

void ParsConfigs::closureOneOrRep()
{
	vector<shared_ptr<Configuration>> ret,tempv;
	tempv.insert(tempv.end(), v.begin(), v.end());
	for (int i = 0; i < v.size(); i++)
	{
		ret = v[i]->closure(false);
		tempv.insert(tempv.end(), ret.begin(), ret.end());
	}
	v = tempv;
}

void ParsConfigs::denyOtherTerminals(shared_ptr<SymbolT> symbolToEat)
{
	for (int i = 0; i < v.size(); i++)
	{
		shared_ptr<Configuration> cfg = v[i];
		shared_ptr<ATNnode> node = cfg->atnNode;
		if (node->edge.label->isTerm() && node->edge.label != symbolToEat)
		{
			v[i] = nullptr;
		}
	}
	v.erase(std::remove_if(v.begin(), v.end(), is_null), v.end());
}
