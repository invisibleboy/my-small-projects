#include <iostream>
#include <fstream>
#include <sstream>
#include <assert.h>
#include "DataType.h"
#include "cacheflow.h"



int SetOutFile(const string &);
int ReadFile(const char *szFile, int);
int ControlFlow();
int CacheFlow();
int Memory2Cache(CFunction *);

int graphviz(CFunction *pFunc);

int main(int argc, char ** argv)
{
    if( argc < 3) 
    {
            cout << "Lack of args!\n";
            exit(1);
    }       
    
    SetOutFile(argv[1]);
	int nOffset = atoi(argv[2]) - 1;
    ReadFile(argv[1], 8192*nOffset);
    //ControlFlow();
    
    CCacheFlow *pCacheFlow = new CCacheFlow();
    pCacheFlow->Run();

    g_RcsFile.close();
    g_LcsFile.close();
    g_GenFile.close();
    return 0;
}

int ReadFile(const char *szFile, int nBigOffset)
{
        ifstream inf;
        inf.open(szFile);

        CFunction *pCurFunc = NULL;
        string szLine;

        while( inf.good() )
        {
                getline( inf, szLine);
                if( szLine.size() <= 4)
                        continue;
                if( szLine.find("controller") == 0 || szLine.find("Disassembly") == 0 )
                        continue;
                stringstream ss( szLine);
                string str1;
                ss >> str1;
                if( str1.find(':') == str1.npos )  // function or label line
                {
                        uint nAddr = hex2dec(str1);

                        nAddr += nBigOffset;
                
                        string szFunc;
                        ss >> szFunc;
                        szFunc = szFunc.substr(1, szFunc.size()-3);                     
                        pCurFunc = new CFunction(szFunc,nAddr); 
                        g_hFuncs[nAddr] = pCurFunc;
                }
                else                            // instruction line
                {
                        CInstruction *pInst = new CInstruction(szLine, nBigOffset);
                        pCurFunc->m_Insts.push_back(pInst);
						pInst->SetFunction( pCurFunc );
                        g_hInsts[pInst->GetAddr()] = pInst;
                }
        }
		
	// set function's end address
	map<uint, CFunction *>::iterator i2f_p = g_hFuncs.begin();
	for(; i2f_p != g_hFuncs.end(); ++ i2f_p )
	{
		CFunction *pFunc = i2f_p->second;
		pFunc->SetEnd( pFunc->m_Insts.back()->GetAddr() + 2);
	}

	return 0;
}


int SetOutFile(const string & szInFile)
{
        string szOutFile = szInFile + ".rcs";
        g_RcsFile.open(szOutFile.c_str());
        szOutFile = szInFile + ".lcs";
        g_LcsFile.open(szOutFile.c_str());
        szOutFile = szInFile + ".gen";
        g_GenFile.open( szOutFile.c_str() );
        return 0;
}
