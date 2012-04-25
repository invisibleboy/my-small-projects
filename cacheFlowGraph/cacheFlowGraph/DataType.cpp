#include "DataType.h"
#include <iomanip>
#include <assert.h>

map<uint, CInstruction *> g_hInsts;
map<uint, CFunction *> g_hFuncs;
list<CBasicBlock *> g_Blocks;

CBasicBlock *g_pEntryBlock;
CBasicBlock *g_pExitBlock;
uint g_nEntryAddress;
uint g_nExitAddress;


//ex:   400140:	28 00 00 00 	lw $16,0($29)
//004001f0 <rt_MatMultRR_Dbl> lw        $13,8($7)
CInstruction::CInstruction(std::string &szCmd, int nBigOffset)
{               
        m_szCmd = szCmd;
        m_nOffset = 0;
        
        stringstream ss(szCmd);
        string str;

        ss >> str;        
		m_nAddr = hex2dec(str.substr(0,str.size()-1) );
        m_nAddr += nBigOffset;
        //m_nTag = m_nAddr/CACHE_LINE_SIZE;

  //      ss >> str;
  //      ss >> str;
  //      ss >> str;
  //      if( str.size() == 2 
		//	&& (isdigit(str[0]) || isdigit(str[1]) ) )
  //      {
  //              ss >> str;
  //              ss >> str;
  //      }
  //      m_szOpc = str;

		//// set m_nOffset, m_nTarget
  //      InstType instType = GetType();
  //      if( instType == CInstruction::SINGLE_JUMP || instType == CInstruction::DUAL_JUMP || instType == CInstruction::CALL)
  //      {
  //              ss >> str;
  //              str = str.substr(1, str.size()-1);
  //              stringstream ss2(str);
  //              ss2 >> m_nOffset;

		//		if( GetOpc() != "call" && GetOpc() != "jump" )
		//		{
		//			m_nTarget = GetOffset() + 2 + GetAddr();
		//		}
		//		else
		//			m_nTarget = GetOffset();
		//		
  //      }

		// set tag, cache line number
		uint setMask = CACHE_SET - 1;
        uint tag = GetAddr() / CACHE_LINE_SIZE;
        uint setIndex = tag & setMask;
        SetCacheSet(setIndex);
        SetTag( tag);

        m_pCallee = NULL;
        m_pBlock = NULL;

}

CInstruction::InstType CInstruction::GetType()
{
        if( m_szOpc == "rcall" || m_szOpc == "call" )
                return CInstruction::CALL;
        if( m_szOpc == "jmp" || m_szOpc == "rjmp" )
                return CInstruction::SINGLE_JUMP;
        if( m_szOpc == "ret" )
                return CInstruction::RET;
        if( m_szOpc == "brbc" || m_szOpc == "brbs" 
                || m_szOpc == "brcc" || m_szOpc == "brcs"
                || m_szOpc == "breq" || m_szOpc == "brne"
                || m_szOpc == "brge" || m_szOpc == "brlt"
                || m_szOpc == "brlo" || m_szOpc == "brpl"
                || m_szOpc == "brmi" || m_szOpc == "brsh"
                || m_szOpc == "brtc" || m_szOpc == "brts"
                || m_szOpc == "brvc" || m_szOpc == "brvs"
                || m_szOpc == "brhc" || m_szOpc == "brhs"
                || m_szOpc == "brid" || m_szOpc == "brie" )
                return CInstruction::DUAL_JUMP;
		if( m_szOpc == "sbrs" )
			return CInstruction::ONE_JUMP;
        return CInstruction::ORDINARY;
}

uint hex2dec(const string &szHex)
{
        uint nSize = szHex.size();
        uint nValue = 0;
        for( int i = 0; i < nSize; ++ i )
        {
                nValue = nValue << 4;
                if( szHex[i] <= '9' && szHex[i] >= '0' )
                        nValue += szHex[i] - 0x30;
                else if( szHex[i] <= 'f' && szHex[i] >= 'a' )
                        nValue += szHex[i] - 87;
                else
                {
					assert(false);
                        cout << szHex << " is not a valid number!\n";
                        exit(1);
                }
        }
        return nValue;
}

uint dec2dec(const string &szDec)
{
        uint nSize = szDec.size();
        uint nValue = 0;
        for( int i = 0; i < nSize; ++ i )
        {
                nValue = nValue * 10;
                if( szDec[i] <= '9' && szDec[i] >= '0' )
                        nValue += szDec[i] - 0x30;
                /*else if( szDec[i] <= 'f' && szDec[i] >= 'a' )
                        nValue += szDec[i] - 87;*/
                else
                {
					assert(false);
                        cout << szDec << " is not a valid number!\n";
                        exit(1);
                }
        }
        return nValue;
}

void CBasicBlock::Add( CInstruction *pInst) 
{ 
        m_Insts.push_back(pInst); 
        //pInst->SetBlock(this);

        m_nTermType = pInst->GetType();
}

int CopyCS(vector<int> &source, vector<int> &target, uint n )
{
    int nDiff = 0;
    for( uint i = 0; i < n; ++ i)
    {
        if( target[i] != source[i] )
        {
                target[i] = source[i];
                nDiff = 1;
        }
    }
        return nDiff;
}

int UnifyCS(vector<int> &first, vector<int> &second, vector<int> &target, uint n )
{
    int nDiff = 0;
    for( uint i = 0; i < n; ++ i)
    {
		if( first[i] != second[i] )       
			target[i] = CACHE::TOP;
		else
			target[i] = first[i];
    }
    return nDiff;
}

int ColdMiss(const vector<int> &source, const vector<int> &target, vector<bool> &hit, uint n, int &nHit)
{
        hit.assign(n, false);
        int num = 0;
        for( uint i = 0; i < n; ++ i)
        {
            if( target[i] != CACHE::BOTTOM )
            {
                if( target[i] == CACHE::TOP || target[i] != source[i] )
                        ++ num;
                else
                {
                        hit[i] = true;
						++ nHit;
                }
            }
        }
        return num;
}

int MergeCS(vector<int> &first, const vector<int> &second, vector<int> &target, uint n )
{
    int nDiff = 0;
    for( uint i = 0; i < n; ++ i)
    {
        if( second[i] == CACHE::BOTTOM )                
            target[i] = first[i];
        else
            target[i] = second[i];
    }

    return nDiff;
}

int Dump(std::ostream &os, const std::vector<int> &flow, const vector<bool> &hit)
{
        for( int i = 0; i < CACHE_SET; ++ i )
        {
                if( i%16 == 0 )
                        os << endl;
                if( flow[i] == CACHE::TOP )
                        os << "-----:";
                else if( flow[i] == CACHE::BOTTOM)
                        os << "+++++:";
                else
                {
                        if( hit[i]) 
                                os << "H";
                        os << setw(5);

                        os << flow[i] << ":";
                }
        }
        return 0;
}


////////////////// for posLCS///////////
int PosCopyCS(map<int, set<int> > &source, map<int, set<int> > &target, uint n )
{
    int nDiff = 0;
    for( uint i = 0; i < n; ++ i)
    {
        if( DiffSet( source[i], target[i] ) )
		{
			nDiff = 1;
			target[i].clear();
			target[i].insert( source[i].begin(), source[i].end() );
		}
    }
    return nDiff;
}

int PosUnifyCS(map<int, set<int> > &source, map<int, set<int> > &target, uint n )
{
        int nDiff = 0;
        for( uint i = 0; i < n; ++ i)
        {
			target[i].insert( source[i].begin(), source[i].end() );
        }

        return nDiff;
}

int PosColdMiss( map<int, set<int> > &source,  map<int, set<int> > &target, vector<bool> &hit, uint n, int &nHit)
{
        hit.assign(n, false);
        int num = 0;
        for( uint i = 0; i < n; ++ i)
        {
			if( !target[i].empty() )
			{
				if( CommonSet( source[i], target[i] ) )
				{
					hit[i] = true;
					++ nHit;
				}
				else
					  ++ num;
			}
        }
        return num;
}

int PosMergeCS(map<int, set<int> > &first,  map<int, set<int> > &second, map<int, set<int> > &target, uint n )
{
        int nDiff = 0;
        for( uint i = 0; i < n; ++ i)
        {
            if( second[i].empty() )          
			{					
				target[i].clear();
				target[i].insert( first[i].begin(), first[i].end() );
			}
            else
			{
				target[i].clear();
				target[i].insert( second[i].begin(), second[i].end() );
			}
        }

        return nDiff;
}

int PosDump(std::ostream &os, std::map<int, set<int> > &flow, const vector<bool> &hit)
{
        for( int i = 0; i < CACHE_SET; ++ i )
        {
            if( i%16 == 0 )
                    os << endl;
			if( hit[i] )
				os << "H";
			set<int> &pos = flow[i];
			set<int>::iterator s_p = pos.begin(), s_e = pos.end();
			for(; s_p != s_e; ++ s_p )
				os << setw(5) << *s_p << ",";
			os << ":\t";
        }
        return 0;
}

int DiffSet(const set<int> &s1, const set<int> &s2)
{
	set<int>::const_iterator s_p = s1.begin(), s_e = s1.end();
	for(; s_p != s_e; ++ s_p )
	{
		if( s2.find (*s_p )  ==  s2.end() )
			return 1;
	}

	return 0;
}

int CommonSet(const set<int> &s1, const set<int> &s2)
{
	set<int>::const_iterator s_p = s1.begin(), s_e = s1.end();
	for(; s_p != s_e; ++ s_p )
	{
		if( s2.find (*s_p )  !=  s2.end() )
			return 1;
	}

	return 0;
}