#include <set>
#include <iomanip>
#include <sstream>
#include <assert.h>
#include "cacheFlow.h"

ofstream g_RcsFile, g_LcsFile, g_GenFile;

int CCacheFlow::Run()
{
	//ControlFlow();
    //TopoSort();

  /*  list<CFunction *>::iterator f_p = m_WorkList.begin(), f_e = m_WorkList.end();
    for(; f_p != f_e; ++ f_p )
    {
        CFunction *pFunc = *f_p;*/
	Graphviz();
#ifndef UPPER
		vector<int> Rcs, Lcs;
        RcsAnalysis( NULL, Rcs );
        LcsAnalysis( NULL, Lcs);
#else
        
        PosRcsAnalysis( pFunc );
        PosLcsAnalysis( pFunc );
#endif
        
   //     g_RcsFile << "\n====================" << pFunc->m_szFunc << "================" << endl;
   //     g_LcsFile << "\n====================" << pFunc->m_szFunc << "================" << endl;
   //     vector<bool> hit;
   //     hit.assign(CACHE_SET, false);
#ifndef UPPER
    //    Dump(g_RcsFile, m_GenRcsP[pFunc], hit );
   //     Dump(g_LcsFile, m_GenLcsP[pFunc], hit );
#else

		m_PosGenRcsP[pFunc]->Dump(g_RcsFile, hit);
		m_PosGenLcsP[pFunc]->Dump(g_LcsFile, hit );
#endif


        
                g_GenFile << CACHE_SET << endl;
#ifndef UPPER
                Write(g_GenFile, Rcs);
#else
                g_GenFile << *m_PosGenRcsP[pFunc];
				//PosWrite(g_GenFile, m_PosGenRcsP[pFunc]);
#endif
                g_GenFile << endl << endl;

#ifndef UPPER
                Write(g_GenFile, Lcs);
#else
                g_GenFile << *m_PosGenLcsP[pFunc];
				//PosWrite(g_GenFile, m_PosGenLcsP[pFunc]);
#endif
       
    return 0;
}


int CCacheFlow::TopoSort()
{
    // print graphviz
    ofstream outf;
    outf.open("pgc.dot");
    outf << "digraph{\n";

    list<CFunction *> tmpList;
    map<CFunction *, set<CFunction *> > Pcg;
    map<uint, CFunction *>::iterator i2f_p = g_hFuncs.begin(), i2f_e = g_hFuncs.end();
    for(; i2f_p != i2f_e; ++ i2f_p )
    {
        CFunction *pFunc = i2f_p->second;

        outf << pFunc->m_szFunc << "[label=\"" << pFunc->m_szFunc << "\"];\n";

        tmpList.push_back(pFunc);
        list<CBasicBlock *>::iterator b_p = pFunc->m_Blocks.begin(), b_e = pFunc->m_Blocks.end();
        for(; b_p != b_e; ++ b_p )
        {
            CBasicBlock *bb = *b_p;
			vector<CInstruction *>::iterator i_p = bb->m_Insts.begin(), i_e = bb->m_Insts.end();
			for( ; i_p != i_e; ++ i_p )
			{
				CInstruction *pInst = *i_p;
				if( pInst->GetCallee() != NULL )
				{
					CFunction *pCallee = pInst->GetCallee();
                    Pcg[pFunc].insert(pCallee);
				}
			}
        }
    }
    /// print PCG
    {
        map<CFunction *, set<CFunction *> >::iterator f2f_p = Pcg.begin(), f2f_e = Pcg.end();
        for(; f2f_p != f2f_e; ++ f2f_p )
        {
                

                cout << endl << f2f_p->first->m_szFunc << ":\t";
                set<CFunction *>::iterator f_p = f2f_p->second.begin(), f_e = f2f_p->second.end();
                for(; f_p != f_e; ++ f_p )
                {
                        cout << (*f_p)->m_szFunc << "\t";
                        
                        outf << f2f_p->first->m_szFunc << " -> " << (*f_p)->m_szFunc << ";\n";
                }
        }

        outf << "}\n";
        outf.close();
    }

	// detect loop
    bool bSucc = true;
    while( true )
    {
        bSucc = false;
        list<CFunction *>::iterator l_p = tmpList.begin(), l_e = tmpList.end();
        for(; l_p != l_e; ++ l_p )
        {
                CFunction *pFunc = *l_p;
                if( Pcg[pFunc].empty() )
                {
                        m_WorkList.push_back(pFunc);
                        bSucc = true;
                        break;
                }
        }

        if( !bSucc )
                break;

        map<CFunction *, set<CFunction *> >::iterator f2f_p = Pcg.begin(), f2f_e = Pcg.end();
        for(; f2f_p != f2f_e; ++ f2f_p )
        {
			f2f_p->second.erase(*l_p);
        }

        tmpList.erase(l_p);
    }
    if( !bSucc && !tmpList.empty() )
    {
        cout << "recursive function calling exist!\n";
        list<CFunction *>::iterator l_p = tmpList.begin(), l_e = tmpList.end();
        for(; l_p != l_e; ++ l_p )
        {
                CFunction *pFunc = *l_p;
                cout << pFunc->m_szFunc << "\t";
                exit(1);
        }
    }
    return 0;
}

int CCacheFlow::RcsAnalysis(CFunction *pFunc, vector<int> &Rcs)
{
    Rcs.assign(CACHE_SET, CACHE::BOTTOM);
    // 1. initilize
    map<CBasicBlock *, vector<int> > InRcs, OutRcs, GenRcs;
    list<CBasicBlock *>::iterator b_p = g_Blocks.begin(), b_e = g_Blocks.end();
    for(; b_p != b_e; ++ b_p )
    {
        CBasicBlock *bb = *b_p;
        
        vector<int> &GenRcsB = GenRcs[bb];
        vector<int> &InRcsB = InRcs[bb];
        vector<int> &OutRcsB = OutRcs[bb];
        GenRcsB.assign(CACHE_SET, CACHE::BOTTOM);
        InRcsB.assign(CACHE_SET, CACHE::BOTTOM);
        OutRcsB.assign(CACHE_SET, CACHE::BOTTOM);

        vector<CInstruction *>::iterator i_p = bb->m_Insts.begin(), i_e = bb->m_Insts.end();
        for(; i_p != i_e; ++ i_p )
        {
            CInstruction *pInst = *i_p;
			GenRcsB[pInst->GetCacheSet()] = pInst->GetTag();							        
        }
    }

    // 2. cache flow analysis               
    bool bChange = true;
    bool bFirst = true;
    while( bChange || bFirst )
    {
        if( bFirst )
                bFirst = false;
        else
                bChange = false;
        b_p = g_Blocks.begin();
        for(; b_p != b_e; ++ b_p )
        {
            CBasicBlock *bb = *b_p;
            vector<int> &GenRcsB = GenRcs[bb];
            vector<int> &InRcsB = InRcs[bb];
            vector<int> &OutRcsB = OutRcs[bb];
            // InRcsB := unify all preceding blocks' OutRcsB
            vector<int> tmpOut(CACHE_SET, CACHE::BOTTOM);
            list<CBasicBlock *>::iterator pred_p = bb->m_Preds.begin(), pred_e = bb->m_Preds.end();
            for( ; pred_p != pred_e; ++ pred_p )
            {
                CBasicBlock *pPred = *pred_p;
                vector<int> &OutPredB = OutRcs[pPred];
				if( pred_p == bb->m_Preds.begin() )
					CopyCS( OutPredB, tmpOut, CACHE_SET );
				else
					UnifyCS( OutPredB, tmpOut, tmpOut, CACHE_SET );
            }

            int nDiff = CopyCS( tmpOut, InRcsB, CACHE_SET );
            if( nDiff == 1 )
                    bChange = true;

            // OutRcsB := InRcsB + GenRcsB
            MergeCS(InRcsB,GenRcsB, OutRcsB, CACHE_SET);
        }
    }
    // 3. record current function's GenRcsP
  
    vector<int> &outRcsTail = OutRcs[g_pExitBlock];
    CopyCS( outRcsTail, Rcs, CACHE_SET );

    return 0;
}

int CCacheFlow::LcsAnalysis(CFunction *pFunc, vector<int> &Lcs)
{
       Lcs.assign(CACHE_SET, CACHE::BOTTOM);
        // 1. initilize
        map<CBasicBlock *, vector<int> > InLcs, OutLcs, GenLcs;
        list<CBasicBlock *>::iterator b_p = g_Blocks.begin(), b_e = g_Blocks.end();
        for(; b_p != b_e; ++ b_p )
        {
            CBasicBlock *bb = *b_p;
            
            vector<int> &GenLcsB = GenLcs[bb];
            vector<int> &InLcsB = InLcs[bb];
            vector<int> &OutLcsB = OutLcs[bb];
            GenLcsB.assign(CACHE_SET, CACHE::BOTTOM);
            InLcsB.assign(CACHE_SET, CACHE::BOTTOM);
            OutLcsB.assign(CACHE_SET, CACHE::BOTTOM);
           
            vector<CInstruction *>::reverse_iterator i_p = bb->m_Insts.rbegin(), i_e = bb->m_Insts.rend();
            for(; i_p != i_e; ++ i_p )
			{
                CInstruction *pInst = *i_p;
                GenLcsB[pInst->GetCacheSet()] = pInst->GetTag();     
			}
        }

        // 2. cache flow analysis               
        bool bChange = true;
        bool bFirst = true;
        while( bChange || bFirst )
        {
                if( bFirst )
                        bFirst = false;
                else
                        bChange = false;
				b_p = g_Blocks.begin();
                for(; b_p != b_e; ++ b_p )
                {
                        CBasicBlock *bb = *b_p;
                        vector<int> &GenLcsB = GenLcs[bb];
                        vector<int> &InLcsB = InLcs[bb];
                        vector<int> &OutLcsB = OutLcs[bb];
                        // OutLcsB := unify all succeding blocks' InLcsB
                        vector<int> tmpIn(CACHE_SET, CACHE::BOTTOM);
                        list<CBasicBlock *>::iterator succ_p = bb->m_Succs.begin(), succ_e = bb->m_Succs.end();
                        for( ; succ_p != succ_e; ++ succ_p )
                        {
                            CBasicBlock *pSucc = *succ_p;
                            vector<int> &InSuccB = InLcs[pSucc];
							if( succ_p == bb->m_Succs.begin() )
								CopyCS( InSuccB, tmpIn, CACHE_SET );
							else
								UnifyCS( InSuccB, tmpIn, tmpIn, CACHE_SET );
                        }

                        int nDiff = CopyCS( tmpIn, OutLcsB, CACHE_SET );
                        if( nDiff == 1 )
                                bChange = true;

                        // InLcsB := InLcsB + GenLcsB
                        MergeCS(OutLcsB,GenLcsB,InLcsB, CACHE_SET);
                }
        }
        // 3. record current function's GenLcsP
       
        vector<int> &InLcsTail = InLcs[g_pEntryBlock];
        CopyCS( InLcsTail, Lcs, CACHE_SET );

        return 0;
}


int CCacheFlow::PosRcsAnalysis(CFunction *pFunc)
{
        CFlowVal *GenRcsP = m_PosGenRcsP[pFunc] = new CFlowVal(CACHE_SET);
        // 1. initilize
        map<CBasicBlock *, CFlowVal *> InRcs, OutRcs, GenRcs;
        list<CBasicBlock *>::iterator b_p = pFunc->m_Blocks.begin(), b_e = pFunc->m_Blocks.end();
        for(; b_p != b_e; ++ b_p )
        {
                CBasicBlock *bb = *b_p;
                
                CFlowVal *GenRcsB = GenRcs[bb] = new CFlowVal(CACHE_SET);				
                InRcs[bb] = new CFlowVal(CACHE_SET);
                OutRcs[bb] = new CFlowVal(CACHE_SET);
                vector<CInstruction *>::iterator i_p = bb->m_Insts.begin(), i_e = bb->m_Insts.end();
                for(; i_p != i_e; ++ i_p )
                {
                    CInstruction *pInst = *i_p;
					(*GenRcsB)[pInst->GetCacheSet()].clear();
					(*GenRcsB)[pInst->GetCacheSet()].insert(pInst->GetTag());
					if( pInst->GetCallee() != NULL )    // identifying the jumpOut instructions and callOut instructions
					{
						CFunction *pCallee = pInst->GetCallee();
						assert( pCallee != NULL );
						CFlowVal *GenRcsCallee = m_PosGenRcsP[pCallee];	
						GenRcsB->Merge( *GenRcsB, *GenRcsCallee );		
					}
                }
        }

        // 2. cache flow analysis               
        bool bChange = true;
        bool bFirst = true;
        while( bChange || bFirst )
        {
            if( bFirst )
                bFirst = false;
            else
                bChange = false;
            b_p = pFunc->m_Blocks.begin();
            for(; b_p != b_e; ++ b_p )
            {
                CBasicBlock *bb = *b_p;
                CFlowVal *GenRcsB = GenRcs[bb];
                CFlowVal *InRcsB = InRcs[bb];
                CFlowVal *OutRcsB = OutRcs[bb];
                // InRcsB := unify all preceding blocks' OutRcsB
                CFlowVal * tmpOut = new CFlowVal(CACHE_SET);
                list<CBasicBlock *>::iterator pred_p = bb->m_Preds.begin(), pred_e = bb->m_Preds.end();
                for( ; pred_p != pred_e; ++ pred_p )
                {
                    CBasicBlock *pPred = *pred_p;
                    CFlowVal *OutPredB = OutRcs[pPred];
					if( pred_p == bb->m_Preds.begin() )
						tmpOut->Assign( *OutPredB );
					else
						tmpOut->Unify( *OutPredB );
                }

				int nDiff = InRcsB->Assign(*tmpOut);
                if( nDiff == 1 )
					bChange = true;
				if( bFirst || bChange )
                // OutRcsB := InRcsB + GenRcsB
					OutRcsB->Merge( *InRcsB, *GenRcsB );
            }
    }
    // 3. record current function's GenRcsP
    CBasicBlock *tailBlock = pFunc->m_Blocks.back();
    CFlowVal *outRcsTail = OutRcs[tailBlock];
	GenRcsP->Assign( *outRcsTail );

    return 0;
}

int CCacheFlow::PosLcsAnalysis(CFunction *pFunc)
{
        CFlowVal *GenLcsP = m_PosGenLcsP[pFunc] = new CFlowVal(CACHE_SET);

        // 1. initilize
        map<CBasicBlock *, CFlowVal * > InLcs, OutLcs, GenLcs;
        list<CBasicBlock *>::iterator b_p = pFunc->m_Blocks.begin(), b_e = pFunc->m_Blocks.end();
        for(; b_p != b_e; ++ b_p )
        {
                CBasicBlock *bb = *b_p;
                
                CFlowVal *GenLcsB = GenLcs[bb] = new CFlowVal(CACHE_SET);
                InLcs[bb] = new CFlowVal(CACHE_SET);
                OutLcs[bb] = new CFlowVal(CACHE_SET);

                vector<CInstruction *>::reverse_iterator i_p = bb->m_Insts.rbegin(), i_e = bb->m_Insts.rend();
                for(; i_p != i_e; ++ i_p )
                {
					CInstruction *pInst = *i_p;
					(*GenLcsB)[pInst->GetCacheSet()].clear();
					(*GenLcsB)[pInst->GetCacheSet()].insert( (*i_p)->GetTag() );
					if( pInst->GetCallee() != NULL )    // identifying the jumpOut instructions
					{
						CFunction *pCallee = pInst->GetCallee();
						assert( pCallee != NULL );
						CFlowVal *GenLcsCallee = m_PosGenLcsP[pCallee];	
						GenLcsB->Merge( *GenLcsB, *GenLcsCallee );			
					}
                }
        }

        // 2. cache flow analysis               
        bool bChange = true;
        bool bFirst = true;
        while( bChange || bFirst )
        {
                if( bFirst )
                    bFirst = false;
				else
                    bChange = false;
                b_p = pFunc->m_Blocks.begin();
                for(; b_p != b_e; ++ b_p )
                {
                    CBasicBlock *bb = *b_p;
                
					CFlowVal *GenLcsB = GenLcs[bb];
					CFlowVal *InLcsB = InLcs[bb];
					CFlowVal *OutLcsB = OutLcs[bb];
                     // OutLcsB := unify all succeding blocks' InLcsB
                    CFlowVal *tmpIn = new CFlowVal(CACHE_SET);
                    list<CBasicBlock *>::iterator succ_p = bb->m_Succs.begin(), succ_e = bb->m_Succs.end();
                    for( ; succ_p != succ_e; ++ succ_p )
                    {
                        CBasicBlock *pSucc = *succ_p;
                        CFlowVal *InSuccB = InLcs[pSucc];
						if( succ_p == bb->m_Succs.begin() )
							tmpIn->Assign( *InSuccB );
						else
							tmpIn->Unify( *InSuccB );
                    }

					int nDiff = OutLcsB->Assign(*tmpIn);
                    if( nDiff == 1 )
                        bChange = true;
					if( bChange || bFirst )
						// InLcsB := OutLcsB + GenLcsB
						InLcsB->Merge( *OutLcsB, *GenLcsB );
                }
        }
        // 3. record current function's GenLcsP
        CBasicBlock *headBlock = pFunc->m_Blocks.front();
        CFlowVal *InLcsHead = InLcs[headBlock];
		GenLcsP->Assign( *InLcsHead );

        return 0;
}

// construction pseudo functions for entries called/jumpped from other functions
int CCacheFlow::PreProcess()
{
	
    map<uint, CFunction *>::iterator i2f_p = g_hFuncs.begin();
    for(; i2f_p != g_hFuncs.end(); ++ i2f_p )
    {
        CFunction *pFunc = i2f_p->second;
		list<CInstruction *> &insts = pFunc->m_Insts;
        list<CInstruction *>::iterator i_p = insts.begin();             
        for(; i_p != insts.end(); ++ i_p )
        {
			CInstruction *pInst = *i_p;
			if( pInst->GetType() == CInstruction::CALL 
				|| pInst->GetType() == CInstruction::DUAL_JUMP 
				|| pInst->GetType() == CInstruction::SINGLE_JUMP )
			{
				if( pInst->GetOffset() == 0 )			// ignore the strange use of call as : "call .+0"
					continue;
				uint nTarget = pInst->GetTarget();
				//skip jumping from the same function
				if( nTarget >= pFunc->GetStart() && nTarget < pFunc->GetEnd() )
					continue;
				CInstruction *pTargetInst = g_hInsts[nTarget];
				if( nTarget != pTargetInst->GetFunction()->GetStart() )
				{
					CFunction * pseuFunc = ConstructFunction( pTargetInst->GetFunction(), nTarget );
					m_pseuFuncs.push_back( pseuFunc );
				}                
			}
		}
	}

	list<CFunction *>::iterator f_p = m_pseuFuncs.begin(), f_e = m_pseuFuncs.end();
	for(; f_p != f_e; ++ f_p )
	{
		CFunction *pseuFunc = *f_p;
		g_hFuncs[pseuFunc->GetStart()] = pseuFunc;
	}

	
	return 0;
}

int CCacheFlow::ControlFlow()
{

   
    return 0;
}


CFunction *CCacheFlow::ConstructFunction(CFunction *pOriFunc, uint nStart )
{
	
	char digits[16];
	itoa(nStart, digits, 10);
	string szName = pOriFunc->GetName() + digits;

	CFunction *pFunc = new CFunction( szName, nStart );
	list<CInstruction *>::iterator i_p = pOriFunc->m_Insts.begin(), i_e = pOriFunc->m_Insts.end();
	for(; i_p != i_e; ++ i_p )
	{
		pFunc->m_Insts.push_back( *i_p );
	}
	pFunc->SetEnd( pOriFunc->GetEnd() );
	return pFunc;
}

int CCacheFlow::RemoveUnreached( CFunction *pFunction )
{	
	list<CBasicBlock *> &blocks = pFunction->m_Blocks;
	map<CBasicBlock *, set<CBasicBlock *> > hReach;
	CBasicBlock *headBlock = blocks.front();

	set<CBasicBlock *> reachBlocks;
	reachBlocks.insert( headBlock);
	bool bChange = true;
	while( bChange )
	{
		bChange = false;
		int nCount = reachBlocks.size();
		set<CBasicBlock *> addBlocks;
		set<CBasicBlock *>::iterator s_p = reachBlocks.begin(), s_e = reachBlocks.end();
		for(; s_p != s_e; ++ s_p )
		{
			CBasicBlock *bb = *s_p;
			addBlocks.insert( bb->m_Succs.begin(), bb->m_Succs.end() );
		}
		reachBlocks.insert( addBlocks.begin(), addBlocks.end() );
		if( reachBlocks.size()  != nCount )
			bChange = true;
	}

	list<CBasicBlock *> tmpBlocks;
	list<CBasicBlock *>::iterator b_p;
	for( b_p = blocks.begin(); b_p != blocks.end(); ++ b_p )
		if( reachBlocks.find( *b_p) != reachBlocks.end() )
			tmpBlocks.push_back( *b_p );
	blocks.clear();
	blocks.insert(blocks.end(), tmpBlocks.begin(), tmpBlocks.end() );
	return 0;
}

int CCacheFlow::Write(std::ostream &os, const std::vector<int> &flow)
{
        for( int i = 0; i < CACHE_SET; ++ i )
        {
                if( i%16 == 0 )
                        os << endl;
                os << setw(5) << flow[i] << "\t";
        }
        return 0;
}


int CCacheFlow::Graphviz()
{    
    string szFileName = "cfg.dot";
    ofstream os;
    os.open(szFileName.c_str());

    os << "digraph{" << endl;

	// node
    list<CBasicBlock *>::iterator b_p = g_Blocks.begin(), b_e = g_Blocks.end();
    for(; b_p != b_e; ++ b_p )
    {
        CBasicBlock *bb = *b_p;
		if( bb->m_Insts.empty() )
			os << "exit[label=\"exit";
		else
			os << bb->GetStart() << "[label=\"" << "block " << bb->GetId() << "\\n";
        vector<CInstruction *>::iterator i_p = bb->m_Insts.begin(), i_e = bb->m_Insts.end();
        for(; i_p != i_e; ++ i_p )
        {
            os << **i_p << "\\n";
        }
        os << "\", shape=box];" << endl;    
    }

	b_p = g_Blocks.begin();
    for(; b_p != b_e; ++ b_p )
    {
        CBasicBlock *bb = *b_p;
		list<CBasicBlock *>::iterator succ_p = bb->m_Succs.begin(), succ_e = bb->m_Succs.end();
        for(; succ_p != succ_e; ++ succ_p )
        {
			os << bb->GetStart() << " -> ";
			if( (*succ_p)->m_Insts.empty() )
				os << "exit";
			else
				os << (*succ_p)->GetStart();
			os << ";" << endl;
        }
	}

    os << "}" << endl;
    return 0;
}


