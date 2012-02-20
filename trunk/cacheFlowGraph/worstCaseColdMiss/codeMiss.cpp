#include <sstream>
#include <vector>
#include <fstream>
#include "Controller.h"
#include "../cacheFlowGraph/cacheFlow.h"

int g_Sequence[128];
int ParserInput( int argc, const char * * argv );
ofstream g_outf;

//int main(int argc, const char * *argv)
//{
//        string szPath = "C:\\Users\\qali\\Downloads\\";
//        string szOutFile = szPath + "controlSequence.txt";
//        g_outf.open(szOutFile.c_str());
//
//        CController *cons[3];
//        cons[0] = new CController(szPath+"fcontroller1.dasm.gen");
//        cons[1] = new CController(szPath+"fcontroller2.dasm.gen");
//        cons[2] = new CController(szPath+"fcontroller3.dasm.gen");
//
//        ParserInput(argc, argv);
//
//        vector<int> GenRcs;
//        GenRcs.assign( CACHE_SET, CACHE::BOTTOM );
//        string szSeq;
//        
//        cout << "\n===========code miss: " << endl;
//        int nLength = argc - 1;
//        for( int i = 0; i < nLength; ++ i )
//        {
//                szSeq += argv[i+1];
//                int nCon = g_Sequence[i] - 1;
//                CController *con = cons[nCon];
//                vector<bool> hit;
//                int nCodeMiss = ColdMiss(GenRcs, con->m_GenLcs, hit, CACHE_SET);
//                cout << nCodeMiss  << " ; ";
//
//                g_outf << endl << "=====================For " << szSeq << "======================" << endl;
//                Dump(g_outf, GenRcs, hit);
//                g_outf << "\n************First Code Miss: " << nCodeMiss << "********" << endl;
//                Dump(g_outf, con->m_GenLcs, hit);
//
//
//                MergeCS(GenRcs, con->m_GenRcs, GenRcs, CACHE_SET);
//        }
//
//        g_outf.close();
//        return 0;
//}


int main(int argc, const char * *argv)
{
        string szPath = "C:\\Users\\qali\\Downloads\\";
        string szOutFile = szPath + "controlSequence.txt";
        g_outf.open(szOutFile.c_str());

        CController *cons[3];
        cons[0] = new CController(szPath+"controller1.dasm.gen");
        cons[1] = new CController(szPath+"controller2.dasm.gen");
        cons[2] = new CController(szPath+"controller3.dasm.gen");

        ParserInput(argc, argv);


#ifndef UPPER
		vector<int>  GenRcs;
		GenRcs.assign(CACHE_SET, CACHE::BOTTOM);
        string szSeq;
        cout << "\n===========code miss: " << endl;
        int nLength = argc - 1;
        for( int i = 0; i < nLength; ++ i )
        {
                szSeq += argv[i+1];
                int nCon = g_Sequence[i] - 1;
                CController *con = cons[nCon];
                vector<bool> hit;
				int nHit = 0;
                int nMiss = ColdMiss(GenRcs, con->m_GenLcs, hit, CACHE_SET, nHit);
				cout << nHit << "/" << nMiss  << " ; ";

                g_outf << endl << "=====================For " << szSeq << "======================" << endl;
                Dump(g_outf, GenRcs, hit);
                g_outf << "\n************First Cold Miss: " << nHit << "/" << nMiss << "********" << endl;
                Dump(g_outf, con->m_GenLcs, hit);


                MergeCS(GenRcs, con->m_GenRcs, GenRcs, CACHE_SET);
        }
#else
		/////////////
		CFlowVal *GenRcs = new CFlowVal(CACHE_SET);
        string szSeq;
		 cout << "\n===========cold hit/miss: " << endl;
        int nLength = argc - 1;
        for( int i = 0; i < nLength; ++ i )
        {
                szSeq += argv[i+1];
                int nCon = g_Sequence[i] - 1;
                CController *con = cons[nCon];
                vector<bool> hit;
				int nMiss = 0;
				int nHit = GenRcs->HitMiss( *(con->m_PosGenLcs), hit, nMiss );
				cout << nHit << "/" << nMiss  << " ; ";

                g_outf << endl << "=====================For " << szSeq << "======================" << endl;
				GenRcs->Dump( g_outf, hit);
                g_outf << "\n************First Cold Hit/Miss: " << nHit << "/" << nMiss << "***********" << endl;
				con->m_PosGenLcs->Dump( g_outf, hit);

				GenRcs->Merge( *(con->m_PosGenLcs), *GenRcs );
        }
		//////////////////////////
#endif

        g_outf.close();
        return 0;
}


int ParserInput( int argc, const char * * argv )
{
        if( argc < 2 )
        {
                cout << "Lack of args!\n";
                exit(1);
        }
        int nLength = argc - 1;
        
        for( int i = 0; i < nLength; ++ i )
        {
                string szArg = argv[i+1];
                stringstream ss( szArg );
                ss >> g_Sequence[i];
        }
        return 0;
}