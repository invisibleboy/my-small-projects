#ifndef _CACHE_FLOW_H
#define _CACHE_FLOW_H

#include <fstream>
#include <map>
#include "DataType.h"
#include "FlowVal.h"

extern ofstream g_RcsFile, g_LcsFile, g_GenFile, g_DotFile;
class CCallSite
{
public:
        CCallSite(CFunction *pCaller, CFunction *pCallee, CBasicBlock *pBlock ) { m_pCaller = pCaller; m_pCallee = pCallee; m_pBlock = pBlock; }
public:
        CFunction *m_pCaller;
        CFunction *m_pCallee;
        CBasicBlock *m_pBlock;

        map<CBasicBlock *, vector<int> >  m_InRcsB, m_OutRcsB;
};

class CCacheFlow
{
public:
        enum MemBlockType
        {
                BOTTOM = -2,                    // no memory block
                TOP     = -1            // uncertain memory block
        };
public:

        map<CFunction *, vector<int> > m_GenRcsP, m_GenLcsP;

		map<CFunction *, CFlowVal *> m_PosGenRcsP, m_PosGenLcsP;

public:
        int Run();

private:
		int PreProcess();
		int ControlFlow();
        int TopoSort();
        //int GenAndCallsite();
		int Dominator();
		int PosDominator();

        int RcsAnalysis(CFunction *pFunc, vector<int> &Rcs);
        int LcsAnalysis(CFunction *pFunc, vector<int> &Lcs);
        int Write(ostream &os, const vector<int> &flow);
		

		int PosRcsAnalysis(CFunction *pFunc);
		int PosLcsAnalysis(CFunction *pFunc);
		int PosWrite(ostream &os, CFlowVal &);

		CFunction *ConstructFunction( CFunction *pOriFunc, uint nStart );
		int RemoveUnreached( CFunction *pFunction );
		int Graphviz(ostream &os);

private:
        list<CFunction *> m_WorkList;
		list<CFunction *> m_pseuFuncs;
};

#endif