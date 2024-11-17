#include <boost/process.hpp>
#include "ATN.h"
#include "StrUtils.h"
#include "Exception.h"
#include <cassert>

ATN::ATN(vector<Prod> &ntermProd, int &startNumber)
{
	startIndex = startNumber;
	startNT = ntermProd[0].leftsym;
	for (int i = 0; i < ntermProd.size(); i++)
	{
		assert(startNT == ntermProd[i].leftsym);
		Prod prod = ntermProd[i];
		shared_ptr<ATNnode> state = make_shared<ATNnode>(i, true, this);
		state->prodNumber = i;
		state->rep = RepEnum::once;
		alternatives.push_back(state);
		for (int j = 0; j < prod.rhs.size(); j++)
		{
			shared_ptr<ATNnode> newstate = make_shared<ATNnode>(startNumber, false, this);
			newstate->prodNumber = i;
			state->rep = prod.repvec[j];
			ATNedge edge(newstate, prod.rhs[j]);
			state->edge = edge;
			startNumber++;
			state = newstate;
		}
		state->tail = true;
	}
	endIndex = startNumber-1;
}

shared_ptr<ATNnode> ATN::findByNumber(int n)
{
	for (int i = 0; i < alternatives.size(); i++)
	{
		shared_ptr<ATNnode> node = alternatives[i];
		node = node->edge.next;
		while (node)
		{
			if (node->number == n) return node;
			node = node->edge.next;
		}
	}
	return nullptr;
}

string ATN::writeDot()
{
	int numNT = startNT->number;
	string dotname = to_string(numNT) + ".dot";
	FILE *f = fopen(dotname.c_str(), "w");
	fprintf(f, "digraph %d {\n", numNT);
	fprintf(f, "rankdir = LR;\n");
	string name = startNT->name;
	fprintf(f, "P%s [label = <p<sub>%s</sub>>]\n", name.c_str(), name.c_str());
	vector<shared_ptr<ATNnode>> tails;
	for (int i = 0; i < alternatives.size(); i++)
	{
		shared_ptr<ATNnode> node = alternatives[i];
		fprintf(f, "P%s_%d [label = <p<sub>%s,%d</sub>>]\n", name.c_str(), i + 1, name.c_str(), i + 1);
		if (!node->edge.next)
			tails.push_back(node);
		node = node->edge.next;
		while (node)
		{
			fprintf(f, "P%d [label = <p<sub>%d</sub>>]\n", node->number + 1, node->number + 1);
			shared_ptr<ATNnode> nextnode = node->edge.next;
			assert(node->tail == !nextnode);
			if (!nextnode)
				tails.push_back(node);
			node = nextnode;
		}
	}
	fprintf(f, "P_%s [label = <p'<sub>%s</sub>>; shape = doublecircle]\n", name.c_str(), name.c_str());
	fprintf(f, "\n");
	for (int i = 0; i < alternatives.size(); i++)
	{
		shared_ptr<ATNnode> node = alternatives[i];
		fprintf(f, "P%s->P%s_%d [label = < &epsilon; > ; weight = 2]\n", name.c_str(), name.c_str(), i + 1);
		if (node->tail)
			continue;
		fprintf(f, "P%s_%d->P%d [label = < %s >]\n", name.c_str(), i+1, node->edge.next->number + 1, node->edge.label->name.c_str());
		node = node->edge.next;
		while (node->edge.next)
		{
			fprintf(f, "P%d->P%d [label = < %s >]\n", node->number + 1, node->edge.next->number + 1, node->edge.label->name.c_str());
			node = node->edge.next;
		}
	}
	for (int i = 0; i < tails.size(); i++)
	{
		shared_ptr<ATNnode> node = tails[i];
		if (node==alternatives[i])
			fprintf(f, "P%s_%d->P_%s [label = < &epsilon; >]\n", name.c_str(), i + 1, name.c_str());
		else
			fprintf(f, "P%d->P_%s [label = < &epsilon; >]\n", node->number + 1, name.c_str());
	}
	fprintf(f, "}");
	fclose(f);
	return dotname;
}

string ATNnode::toString()
{
	if (starting)
		return string_format("P{%s,%d}",atn->startNT->name.c_str(),number+1);
	else
		return string_format("P%d", number + 1);
}

void ATNset::makeFrom(Grammar* grammar)
{
	ownerGram = grammar;
	int startNumber = 0;
	for (int i = 0; i < grammar->ntermProd.size(); i++)
	{
		shared_ptr<ATN> atn = make_shared<ATN>(grammar->ntermProd[i], startNumber);
		atns.push_back(atn);
	}
}

void ATNset::writeDot()
{
	for (int i = 0; i < atns.size(); i++)
	{
		string dotname = atns[i]->writeDot();
		int pos = dotname.find_last_of('.');
		string pngname = dotname.substr(0, pos) + ".png";
		boost::process::system("c:\\Graphviz2.38\\bin\\dot.exe", "-Tpng", dotname, "-o", pngname);
	}
}

shared_ptr<ATNnode> ATNset::findNode(string name)
{
	if (name.substr(0, 2) == "P{")
	{
		int pos = name.find(',');
		string ntname = name.substr(2, pos - 2);
		string numstr = name.substr(pos+1, name.find('}') - pos -1);
		int num = atoi(numstr.c_str())-1;
		shared_ptr<ATN> atn = atns[ownerGram->getNT(ntname)->number];
		return atn->alternatives[num];
	}
	else
	{
		if (name[0] != 'P') throw Exception("bad name "+name);
		string numstr = name.substr(1);
		int num = atoi(numstr.c_str()) - 1;
		shared_ptr<ATN> atn = nullptr;
		for (int i = 0; i < atns.size(); i++)
		{
			if (atns[i]->endIndex >= num)
			{
				atn = atns[i];
				break;
			}
		}
		return atn->findByNumber(num);
	}
}
