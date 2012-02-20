#include <iostream>
#include <fstream>
#include <sstream>
#include "../cacheFlowGraph/cacheFlow.h"
#include "Controller.h"

using namespace std;
CController::CController(const string &szFile)
{
        m_szFile = szFile;
#ifndef UPPER
        Read();
#else
		PosRead();
#endif
}

int CController::Read()
{
        ifstream inf;
        inf.open(m_szFile.c_str());

        if( !inf.good() )
        {
                cerr << "Error open " << m_szFile << endl;
                exit(1);
        }

        uint nCacheSet = 0;
        inf >> nCacheSet;
        nCacheSet = CACHE_SET;
        
        m_GenLcs.assign(nCacheSet, CACHE::BOTTOM);
        m_GenRcs.assign(nCacheSet, CACHE::BOTTOM);
        for( uint i = 0; i < nCacheSet; ++ i )
        {
                inf >> m_GenRcs[i];
        }
        for( uint i = 0; i < nCacheSet; ++ i )
        {
                inf >> m_GenLcs[i];
        }

        inf.close();

        return 0;
}

int CController::PosRead()
{
    ifstream inf;
    inf.open(m_szFile.c_str());

    if( !inf.good() )
    {
            cerr << "Error open " << m_szFile << endl;
            exit(1);
    }

    uint nCacheSet = 0;
    nCacheSet = CACHE_SET;
    
	
    inf >> nCacheSet;
	m_PosGenRcs = new CFlowVal(nCacheSet);
    for( uint i = 0; i < nCacheSet; ++ i )
    {
		string szLine;
		getline(inf, szLine, ':');

		if( szLine.find(",") != szLine.npos )
		{
			uint nStart = 0;
			uint nEnd = szLine.find(",", nStart);
			while( nEnd != string::npos )
			{
				uint nNum;
				string szNum = szLine.substr( nStart, nEnd - nStart);
				stringstream ss(szNum);
				ss >> nNum;
				(*m_PosGenRcs)[i].insert(nNum);

				nStart = nEnd + 1;
				nEnd = szLine.find(",", nStart);				
			}
		}       
    }

	m_PosGenLcs = new CFlowVal(CACHE_SET);
    for( uint i = 0; i < nCacheSet; ++ i )
    {
		string szLine;
		getline(inf, szLine, ':');

		if( szLine.find(",") != szLine.npos )
		{
			uint nStart = 0;
			uint nEnd = szLine.find(",", nStart);
			while( nEnd != string::npos )
			{
				uint nNum;
				string szNum = szLine.substr( nStart, nEnd - nStart);
				stringstream ss(szNum);
				ss >> nNum;
				(*m_PosGenLcs)[i].insert( nNum);

				nStart = nEnd + 1;
				nEnd = szLine.find(",", nStart);				
			}
		}         
    }

    inf.close();

    return 0;
}