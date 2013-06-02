#include <iostream>
#include <fstream>
#include <sstream>
#include <assert.h>
#include <map>
#include "../cacheFlowGraph/DataType.h"
#include "../cacheFlowGraph/cacheflow.h"




static std::map<uint, map<uint, CBasicBlock *> > g_hBlocks;


int SetOutFile(string );
int ReadCode(const char *szFile, int);
int ReadCFG1(const char *, int nBigOffset );
int ReadCFG2(const char *, int nBigOffset );

int main(int argc, char ** argv)
{
	char *p = new char[10];
    if( argc < 3) 
    {
            cout << "Lack of args!\n";
            exit(1);
    }       
	
    
    SetOutFile(argv[1]);
	int nOffset = atoi(argv[2]) - 1;
	string szBasename = argv[1];
	string szDisFile = szBasename + ".dis";
	string szCFG1 = szBasename + ".map";
	string szCFG2 = szBasename + ".cfg";
	string szGraph = szBasename + ".dot";

	ReadCode(szDisFile.c_str(), nOffset * OFFSET );
	ReadCFG1(szCFG1.c_str(), nOffset * OFFSET );
	ReadCFG2(szCFG2.c_str(), nOffset * OFFSET );
    //ControlFlow();
    
    CCacheFlow *pCacheFlow = new CCacheFlow();
    pCacheFlow->Run();

    g_RcsFile.close();
    g_LcsFile.close();
    g_GenFile.close();
	g_DotFile.close();
    return 0;
}

int ReadCode(const char *szFile, int nBigOffset)
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
        if( szLine.find("start_addr") == 0 || szLine.find("Disassembly") == 0 )
                continue;
		if( szLine.find("...") != string::npos )
			break;
		// skip comment line for source file info
		if( szLine.find(".c:") != string::npos)
			continue;
		if( szLine.find("format") != string::npos )
			continue;
		if( szLine.find("Disassembly") != string::npos )
			continue;
        stringstream ss( szLine);
        string str1;
        ss >> str1;
		//ex: 00400140 <__start>:
		//rt_MatMultRR_Dbl():
        if( str1.find(':') == str1.npos )  // function or label line
        {
			uint nAddr = hex2dec(str1);
			nAddr = TransAddress(nAddr, nBigOffset);
            string szFunc;
			ss >> szFunc;
            szFunc = szFunc.substr(1, szFunc.size()-3);                     
            pCurFunc = new CFunction(szFunc,nAddr); 
			g_hFuncs[nAddr] = pCurFunc;

			if( szFunc == "main" )
				g_nEntryAddress = nAddr;
                //g_hFuncs[nAddr] = pCurFunc;
        }
		// instruction line
		// ex:   400140:	28 00 00 00 	lw $16,0($29)
        else                            
        {
			if( pCurFunc == NULL )
				continue;
			 
            CInstruction *pInst = new CInstruction(szLine, nBigOffset);
            pCurFunc->m_Insts.push_back(pInst);
			pInst->SetFunction( pCurFunc );
            g_hInsts[pInst->GetAddr()] = pInst;	

			
			if( pCurFunc->GetName() == "main" )
				g_nExitAddress = pInst->GetAddr();
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

// for block's insts, start address
int ReadCFG2(const char *szFile, int nBigOffset )
{ 
	ifstream inf;
    inf.open(szFile);

	uint nProcId = -1;
	CFunction *pCurFunc = NULL;
	list<CInstruction *>::iterator cur_p;
	CBasicBlock *pCurBlock = NULL;

    string szLine;

    while( inf.good() )
    {
        getline( inf, szLine);
		if( szLine.size() < 4 )
			continue;
		if( szLine.find("cfg:") != string::npos )
		{
			if( pCurFunc != NULL )
			{
				for(; cur_p != pCurFunc->m_Insts.end(); ++ cur_p )
				{
					CInstruction *pInst = *cur_p;
					pCurBlock->m_Insts.push_back( pInst );
				}
				pCurFunc = NULL;
			}

			uint index1 = szLine.find('[');
			uint index2 = szLine.find(']');
			string szProcNum = szLine.substr(index1+1, index2-index1-1);

			nProcId = dec2dec(szProcNum);
		}
		//ex:
		// 0 : 00400328 : [  1 ,   ]  P0
		else
		{
			uint nBlockId = -1;
			uint nStart = -1;
			

			stringstream ss(szLine);
			string str;

			ss >> str;
			nBlockId = dec2dec( str);
			CBasicBlock *pBlock = g_hBlocks[nProcId][nBlockId];


			ss >> str; // for ":"
			ss >> str;
			nStart = hex2dec( str);
			nStart = TransAddress(nStart, nBigOffset);
			pBlock->SetStart( nStart );		
			if( nStart == g_nEntryAddress )
				g_pEntryBlock = pBlock;

			
			if( pCurFunc == NULL)
			{
				pCurFunc = g_hFuncs[ nStart ];
				assert(pCurFunc != NULL );
				cur_p = pCurFunc->m_Insts.begin();
			}
			else
			{
				for(; cur_p != pCurFunc->m_Insts.end(); ++ cur_p )
				{
					CInstruction *pInst = *cur_p;
					if( pInst->GetAddr() < nStart )
						pCurBlock->m_Insts.push_back( pInst );
					else
						break;
				}
			}
			pCurFunc->m_Blocks.push_back(pBlock);
			pCurBlock = pBlock;			
		}
	}
	inf.close();
	
	// deal with the last block
	if( pCurFunc != NULL )
	{
		for(; cur_p != pCurFunc->m_Insts.end(); ++ cur_p )
		{
			CInstruction *pInst = *cur_p;
			pCurBlock->m_Insts.push_back( pInst );			
		}
		if( pCurFunc->GetName() == "main" )
			g_pExitBlock = pCurBlock;
	}


	return 0;
}

// for block's ID, succs, preceeds
int ReadCFG1( const char *szFile, int nBigOffset )
{
	ifstream inf;
    inf.open(szFile);

	//0(2.0): [ 2 ]
	map<uint, set<uint> > hSuccs;
	map<uint, CBasicBlock *> hId2Block;
	while( inf.good() )
	{
		string szLine;
		getline(inf, szLine);
		if(szLine.find("dump") != string::npos )
			continue;
		if( szLine.size() < 4 )
			continue;
		stringstream ss(szLine);
		string str;
		ss >> str;

		uint index1 = str.find('(');
		string szBlockId = str.substr(0,index1);
		uint nGBlockId = dec2dec(szBlockId);
		uint index2 = str.find('.');
		string szProcId = str.substr(index1+1, index2-index1-1);
		uint nProcId = dec2dec(szProcId);
		index1 = str.find(')');
		szBlockId = str.substr(index2+1, index1-index2-1);
		uint nBlockId = dec2dec(szBlockId);
	
		CBasicBlock *pBlock = new CBasicBlock( 0, nGBlockId);
		g_hBlocks[nProcId][nBlockId] = pBlock;
		hId2Block[nGBlockId] = pBlock;
		g_Blocks.push_back( pBlock );

		// scan for succeeding blocks
		ss >> str;
		assert( str == "[" );
		ss >> str;
		while( str != "]" )
		{
			uint nSucc = dec2dec(str);
			hSuccs[nGBlockId].insert( nSucc );
			ss >> str;
		}
	}
	inf.close();


	// construct CFG
	map<uint, set<uint> >::iterator u2us_p = hSuccs.begin(), u2us_e = hSuccs.end();
	for(; u2us_p != u2us_e; ++ u2us_p )
	{
		CBasicBlock *pBlock = hId2Block[u2us_p->first];
		set<uint>::iterator s_p = u2us_p->second.begin(), s_e = u2us_p->second.end();
		for(; s_p != s_e; ++ s_p )
		{
			CBasicBlock *pSuccBlock = hId2Block[*s_p];
			pBlock->m_Succs.push_back( pSuccBlock );
			pSuccBlock->m_Preds.push_back( pBlock );
		}
	}

	return 0;
}


int SetOutFile(string szBaseFile)
{
	char szAssoc[17], szBlockSize[17], szCacheSet[17];
	itoa(CACHE_SET, szCacheSet, 10);		
	itoa(CACHE_LINE_SIZE, szBlockSize, 10 );
	itoa(CACHE_ASSOCIATIVITY, szAssoc, 10);

	string szOutFile = szBaseFile + "_" + szCacheSet + "_" + szBlockSize + "_" + szAssoc + "_loop";

	

        /*string szOutFile = szInFile + ".rcs";
        g_RcsFile.open(szOutFile.c_str());
        szOutFile = szInFile + ".lcs";
        g_LcsFile.open(szOutFile.c_str());*/
        szOutFile = szOutFile + ".gen";
        g_GenFile.open( szOutFile.c_str() );
		cerr << endl << "writing into " << szOutFile << endl;

		szOutFile = szBaseFile + ".dot";
		g_DotFile.open(szOutFile.c_str() );
		cerr << endl << "writing into " << szOutFile << endl;
        return 0;
}
