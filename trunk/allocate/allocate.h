#include <string>
#include <stdint.h>
#include <map>
#include <fstream>
#include <set>
#include <list>

using namespace std; 

typedef int64_t INT64;
typedef uint64_t ADDRINT;
typedef unsigned int UINT32;

class CBlock
{
public:	
	CBlock() { _size = 0; }
	int Add(ADDRINT obj) { ++ _size; _elements.insert(obj);}
public:
	
	UINT32 _size;
	ADDRINT _rep;
	set<ADDRINT> _elements;	
};

class CAllocate
{
public:
	int Run(string szInput);
private:
	int Build();
	int DoAllocate1();
	
	int DoAllocate2();
	bool FindBest(const set<ADDRINT> &allocated, ADDRINT &obj, map<ADDRINT, ADDRINT> &costM, set<ADDRINT> &neighbors);
	int AddAllocate(set<ADDRINT> &allocated, CBlock *pBlock, ADDRINT obj, map<ADDRINT, map<ADDRINT, ADDRINT> > &fGraph, map<ADDRINT, set<ADDRINT> > &Atable);
	
	int Dump(ofstream &os);
	
	int AdjacentTable(int nFunc, map<ADDRINT, set<ADDRINT> > &);
private:
	string m_szInput;
	map<int, map<ADDRINT, map<ADDRINT, ADDRINT> > > m_graph;
	map<int, list<CBlock *> > m_Blocks;
};
