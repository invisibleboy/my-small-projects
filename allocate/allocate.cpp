#include "allocate.h"
#include <string>
#include <sstream>
#include <set>
#include <assert.h>
#include <iostream>
using namespace std;

#define NPOS -1

UINT32 g_blockSize = 32;
UINT32 g_capacity;

int main(int argc, char *argv[])
{
	if( argc < 3 )
	{
		cerr << "lack of args" << endl;
		return -1;
	}
	stringstream ss(argv[1]);
	ss >> g_blockSize;
	g_capacity = g_blockSize/4;
	
	CAllocate alloc;
	alloc.Run(argv[2]);
	return 0;
}

int CAllocate::Run(string szInput)
{
	m_szInput = szInput;
	Build();
	DoAllocate2();
	
	ofstream outf;
	outf.open("alloc.txt");
	Dump(outf);
	outf.close();
	return 0;
}

int CAllocate::Build()
{
	ifstream inf;
	
	string szLine;
	int nFunc;
	ADDRINT obj1;
	
	inf.open(m_szInput.c_str() );
	while(inf.good() )
	{
		getline(inf, szLine);
		if( szLine.size() < 2 )
			continue;
		if( szLine.find("###") == 0 )  // a new function
		{
			stringstream ss(szLine.substr(3));
			ss >> nFunc;
			continue;
		}
		UINT32 index = szLine.find(":");
		if( index != NPOS )  // a new obj1
		{
			string szObj1 = szLine.substr(0, index);
			stringstream ss(szObj1);
			ss >> obj1;
			continue;
		}
		
		while( (index=szLine.find(";")) != NPOS)
		{
			string szRecord = szLine.substr(0,index);
			stringstream ss(szRecord);
			ADDRINT obj2, cost;
			ss >> obj2 >> cost;				
			m_graph[nFunc][obj1][obj2] = m_graph[nFunc][obj2][obj1] = cost;					
			szLine = szLine.substr(index+1);
			if( szLine.size() < 2 )
				break;
		}
	}
	
	inf.close();
	return 0;
}

int CAllocate::Dump(ofstream &os)
{
	map<int, list<CBlock *> >::iterator I = m_Blocks.begin(), E = m_Blocks.end();
	for( ; I != E; ++ I)
	{
		int nFunc = I->first;
		os << endl << "###" << nFunc <<  endl;
		list<CBlock *>::iterator J = I->second.begin(), E1 = I->second.end();
		for(; J != E1; ++ J)
		{
			CBlock *pBlock = *J;
			if( I->first == 0)
				os << endl << pBlock->_rep << ":\t";
			else
				os << endl << (int) pBlock->_rep << ":\t";
			set<ADDRINT>::iterator K = pBlock->_elements.begin(), E2 = pBlock->_elements.end();
			for(; K != E2; ++ K)
			{
				if( nFunc == 0)
					os << *K << "; ";
				else 
					os << (int)*K << "; ";
			}
			delete pBlock;
		}
	}
}

int CAllocate::AdjacentTable(int nFunc, map<ADDRINT, set<ADDRINT> > &Atable)
{
	map<ADDRINT, map<ADDRINT, ADDRINT> > &fGraph = m_graph[nFunc];
	std::map<ADDRINT, map<ADDRINT, ADDRINT> >::iterator I = fGraph.begin(), E = fGraph.end();
	for(; I != E; ++ I)
	{
		std::map<ADDRINT, map<ADDRINT, ADDRINT> >::iterator J = fGraph.begin();
		for(; J != E; ++ J)
		{
			if( I != J)
			{
				Atable[I->first].insert(J->first);
				Atable[J->first].insert(I->first);
			}
		}
	}
	
	return 0;
}

int UpdateGraph(map<ADDRINT, map<ADDRINT, ADDRINT> > &fGraph, map<ADDRINT, set<ADDRINT> > &Atable, CBlock *pBlock, ADDRINT obj)
{
	ADDRINT repObj = pBlock->_rep;
	std::set<ADDRINT> &neighbors = Atable[obj]; 
	set<ADDRINT>::iterator I = neighbors.begin(), E = neighbors.end();
	for(; I != E; ++ I)
	{
		fGraph[repObj][*I] += fGraph[obj][*I];
		fGraph[*I][repObj] = fGraph[repObj][*I];
		//fGraph[repObj][*I] = min(fGraph[repObj][*I], fGraph[obj][*I]);
		//fGraph[*I][repObj] = fGraph[repObj][*I];
		
		Atable[*I].erase(obj);		
	}
	return 0;
}

int CAllocate::DoAllocate1()
{
	// 
	map<int, map<ADDRINT, map<ADDRINT, ADDRINT> > >::iterator i_p = m_graph.begin(), i_e = m_graph.end();
	for(; i_p != i_e; ++ i_p )
	{
		int nFunc = i_p->first;
		cerr << endl << "For ###" << nFunc;
		UINT32 stackSize = m_graph[nFunc].size()*4;
		map<ADDRINT, map<ADDRINT, ADDRINT> > &fGraph = m_graph[nFunc];
		list<CBlock *> &blocks = m_Blocks[nFunc];
		
		map<ADDRINT, set<ADDRINT> > Atable;
		AdjacentTable(nFunc, Atable);
		
		set<ADDRINT> headVertice;
		set<ADDRINT> allocated;		
		map<ADDRINT, CBlock *> hBlocks;
		UINT32 nBlocks = stackSize/g_blockSize;
		for(int i = 0; i < nBlocks; ++ i)
        {
            CBlock *cb = new CBlock();
            blocks.push_back(cb);
        }
		
		// do allocation
		 do
        {
             // 2.1 find the most beneficial edge
             std::map<ADDRINT, std::set<ADDRINT> >::iterator I = Atable.begin(), E = Atable.end();
			 int nPrev = 0, nNext = 0;
			 bool bNFound = true;
			 for(; I != E; ++ I)
			 {
				 ADDRINT obj1 = I->first;
			     // the first object in each block is not allcoated ?? No, alloated
			     // skip allocated && non-head vertex
			     bool bHeadVertex = false;
			     if( headVertice.find(obj1) != headVertice.end() )
                    bHeadVertex = true;
				 if (allocated.find(obj1) != allocated.end() && !bHeadVertex )
					 continue;

				 set<ADDRINT> &neighbors = I->second;
				 set<ADDRINT>::iterator i2d_p = neighbors.begin(), i2d_e = neighbors.end();
				 for( ; i2d_p != i2d_e; ++ i2d_p)
				 {
					ADDRINT obj2 = *i2d_p;	
					ADDRINT cost = fGraph[obj1][obj2];
				     // skip the second headVertice		
					bool bHeadVertex2 = false;
					if( headVertice.find(obj2) != headVertice.end() )
						bHeadVertex2= true;
					if( bHeadVertex && bHeadVertex2 )
						continue;
					
					 if( bNFound || cost < fGraph[nPrev][nNext] )  // record the smallest one
					 {
						 nPrev = obj1;
						 nNext = obj2;
						 bNFound = false;
						 if( cost == 0)
							 break;
					 }
				 }
				 if( !bNFound && fGraph[nPrev][nNext] == 0)     // 0 is the smallest cost
					 break;
			 }
			// no more edges left, it means all objects has been allocated or no more could be allocated	
			if( bNFound)   				
				break;
			 

            CBlock* pBlock = NULL;
            if( headVertice.find(nPrev) != headVertice.end() )    // nPrev is the head vertex
            {
                assert(headVertice.find(nNext) == headVertice.end() );      // ????? with line 258
				if( hBlocks[nPrev]->_size < g_capacity)
				{
					//cerr << endl << "allocating " << nNext;
					pBlock = hBlocks[nPrev];
					pBlock->Add(nNext);
					hBlocks[nNext] = pBlock;
					allocated.insert(nNext);
					UpdateGraph(fGraph, Atable, pBlock, nNext);
				}				 
            }
            else if( headVertice.find(nNext) != headVertice.end() )   // nNext is the head vertex
            {
                assert(headVertice.find(nPrev) == headVertice.end() );      // ????? with line 258
                if( hBlocks[nNext]->_size < g_capacity )
				{
					//cerr << endl << "allocating " << nPrev;
					pBlock = hBlocks[nNext];
					pBlock->Add(nPrev);
					hBlocks[nPrev] = pBlock;
					allocated.insert(nPrev);
					UpdateGraph(fGraph, Atable, pBlock, nPrev);
				}               
            }
            else
            {
				bool bAllocated = false;
				std::list<CBlock *>::iterator I = blocks.begin(), E = blocks.end();
				for(; I != E; ++ I)
				{
					CBlock *block = *I;					
					if( block->_size + 1 < g_capacity)
					{
						bAllocated = true;
						pBlock = block;
						pBlock->Add(nPrev);
						hBlocks[nPrev] = pBlock;
						allocated.insert(nPrev);
						if( pBlock->_size == 1)
						{
							//cerr << endl << "allocating " << nPrev << " and " << nNext;
							pBlock->_rep = nPrev;			
							headVertice.insert(nPrev);
						}	
						else
						{
							UpdateGraph(fGraph, Atable, pBlock, nPrev);
						}
						//cerr << endl << "allocating " << nNext; 												
						pBlock->Add(nNext);
						hBlocks[nNext] = pBlock;
						allocated.insert(nNext);
						UpdateGraph(fGraph, Atable, pBlock, nNext);
						break;
					}
				}	
            }

            if( !pBlock )
            {
                Atable[nPrev].erase(nNext);
                Atable[nNext].erase(nPrev);
            }
        }while(true);
	}
	return 0;
}

bool CAllocate::FindBest(const set<ADDRINT> &allocated, ADDRINT &obj, map<ADDRINT, ADDRINT> &costM, set<ADDRINT> &neighbors)
{
	bool bFound = false;
	ADDRINT best;
	ADDRINT bestCost;
	std::set<ADDRINT>::iterator I = neighbors.begin(), E = neighbors.end();
	for(; I != E; ++ I)
	{
		if( allocated.find(*I) != allocated.end())
			continue;
		ADDRINT cost = costM[*I];
		if( !bFound || cost < bestCost)
		{
			bFound = true;
			best = *I;
			bestCost = cost;
		}
	}	
	obj = best;
	return bFound;
}

int CAllocate::AddAllocate(set<ADDRINT> &allocated, CBlock *pBlock, ADDRINT obj, map<ADDRINT, map<ADDRINT, ADDRINT> > &fGraph, map<ADDRINT, set<ADDRINT> > &Atable)
{
	if( pBlock->_size == g_capacity)
		return 0;
	ADDRINT obj2;
	bool bFound = FindBest(allocated, obj2, fGraph[obj], Atable[obj]);
	if( !bFound )   // allocation ends
		return 1;
	pBlock->Add(obj2);
	allocated.insert(obj2);
	UpdateGraph(fGraph, Atable, pBlock, obj2);	
	return AddAllocate(allocated, pBlock, obj, fGraph, Atable);		
}
int CAllocate::DoAllocate2()
{
	// 
	map<int, map<ADDRINT, map<ADDRINT, ADDRINT> > >::iterator i_p = m_graph.begin(), i_e = m_graph.end();
	for(; i_p != i_e; ++ i_p )
	{
		int nFunc = i_p->first;
		cerr << endl << "For ###" << nFunc;
		UINT32 stackSize = m_graph[nFunc].size()*4;
		map<ADDRINT, map<ADDRINT, ADDRINT> > &fGraph = m_graph[nFunc];
		list<CBlock *> &blocks = m_Blocks[nFunc];
		
		map<ADDRINT, set<ADDRINT> > Atable;
		AdjacentTable(nFunc, Atable);
		
		set<ADDRINT> allocated;
		UINT32 nBlocks = (stackSize+g_blockSize-1)/g_blockSize;
		for(int i = 0; i < nBlocks; ++ i)
        {
            CBlock *cb = new CBlock();
            blocks.push_back(cb);
        }
		
		 std::map<ADDRINT, std::set<ADDRINT> >::iterator I = Atable.begin(), E = Atable.end();		
		 for(; I != E; ++ I)
		 {
			 ADDRINT obj1 = I->first;
			 if( allocated.find(obj1) != allocated.end())
				 continue;
			 // the first object in each block is not allcoated ?? No, alloated
			 // skip allocated && non-head vertex
			CBlock *pBlock = blocks.front();
			if( pBlock->_size != 0)
				break;
			blocks.pop_front();
			blocks.push_back(pBlock);
			
			pBlock->Add(obj1);
			allocated.insert(obj1);
			pBlock->_rep = obj1;			
			int bOver = AddAllocate(allocated, pBlock, obj1, fGraph, Atable);	
			if( bOver)
				break;
			
		}            
	}
	return 0;
}