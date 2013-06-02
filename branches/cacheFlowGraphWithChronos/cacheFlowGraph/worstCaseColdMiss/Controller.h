#ifndef _CONTROLLER_H
#define _CONTROLLER_H

#include <string>
#include <vector>
#include <set>
#include <map>

using namespace std;

#include "../cacheFlowGraph/FlowVal.h"

class CController
{
public:
        CController(const string &szName);
public:
        vector<int> m_GenRcs;
        vector<int> m_GenLcs;

		CFlowVal *m_PosGenRcs;
		CFlowVal *m_PosGenLcs;
		string m_szFile;

private:
        int Read();
		int PosRead();
        
        
};

#endif