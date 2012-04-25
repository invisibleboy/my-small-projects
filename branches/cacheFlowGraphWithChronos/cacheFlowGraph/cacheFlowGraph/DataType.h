#ifndef _DATATYPE_H
#define _DATATYPE_H

#include <string>
#include <vector>
#include <map>
#include <list>
#include <sstream>
#include <iostream>
#include <set>

#define OFFSET (2<<15)

#define CACHE_SET	128
#define CACHE_LINE_SIZE 16
#define CACHE_ASSOCIATIVITY 1
#define CACHE_SIZE      (CACHE_SET*CACHE_LINE_SIZE*CACHE_ASSOCIATIVITY)

using namespace std;

typedef unsigned int uint;
class CFunction;
class CBasicBlock;

//#define UPPER

uint hex2dec(const string &szHex);
uint dec2dec(const string &szDec);

namespace CACHE
{
        enum MemoryMap
        {
                BOTTOM = -2,
                TOP = -1,
                ORDINARY
        };
}

class CInstruction
{
public:
        enum InstType
        {
                ORDINARY,
                SINGLE_JUMP,
                DUAL_JUMP,
				ONE_JUMP,
                CALL,
                RET
        };

public:
        CInstruction(string &szCmd, int);
        
        InstType GetType();
        uint GetAddr() {return m_nAddr;}
        string GetOpc() {return m_szOpc; }
        uint GetOffset() {return m_nOffset; }
        uint GetCacheSet() { return m_nCacheSet; }
        void SetCacheSet(uint nCacheSet) { m_nCacheSet = nCacheSet; }
        CFunction *GetCallee() { return m_pCallee; }
        void SetCallee(CFunction *pCallee) { m_pCallee = pCallee; }
        CBasicBlock *Parent() { return m_pBlock; }
        void SetBlock(CBasicBlock *pBlock ) { m_pBlock = pBlock; }
        uint GetTag() { return m_nTag; }
        void SetTag(uint nTag ) { m_nTag = nTag; }
		uint GetTarget() { return m_nTarget; }
		CFunction *GetFunction() { return m_pFunc; };
		void SetFunction(CFunction *pFunc ) { m_pFunc = pFunc; }
        
        friend inline ostream & operator << (ostream &os, CInstruction &inst)
        {
                os << inst.m_nAddr << ":\t" << inst.GetOpc();
                return os;
        }



private:
        uint m_nAddr;                   // the instruction address
        uint m_nTag;
        int m_nOffset;                  // For jump instructions, the offset
		int m_nTarget;					// for jump and call, the target address
        string m_szCmd;                 // the full string form for this instruction
        string m_szOpc;                 // the opcode
        uint m_nCacheSet;               // the cache set number for this instruction address
        CFunction *m_pCallee;   // For a call instruction, the callee function
        CBasicBlock *m_pBlock;  // the block this instruction belong to
		CFunction *m_pFunc;
};




class CBasicBlock
{
public:
        vector<CInstruction *> m_Insts;
        list<CBasicBlock *> m_Preds;
        list<CBasicBlock *> m_Succs;

public:
        CBasicBlock(uint nStart) { m_nStart = nStart; m_bCall = false;m_bEntryCall = true; m_nTermType = CInstruction::ORDINARY;};
		CBasicBlock(uint nStart, uint nId) { m_nStart = nStart; m_nId = nId; };
		uint GetId() { return m_nId; }
        uint GetStart() { return m_nStart; }
		void SetStart( uint nStart ) { m_nStart = nStart; }
        bool IsCall() { return m_bCall; }
        void SetCall( bool bCall) { m_bCall = bCall; }
        bool IsEntryCall() { return m_bEntryCall; }
        void SetEntryCall( bool bEntry) { m_bEntryCall = bEntry; }
        CInstruction::InstType GetTermType() { return m_nTermType; }
        friend inline ostream & operator << (ostream &os, CBasicBlock &bb)
        {
                vector<CInstruction *>::iterator i_p = bb.m_Insts.begin(), i_e = bb.m_Insts.end();
                for(; i_p != i_e; ++ i_p )
                {
                        os << *i_p << endl;
                }
                return os;
        }
        void Add( CInstruction *pInst);

private:
	uint m_nId;
        uint m_nStart;
        CInstruction::InstType m_nTermType;                     // The type of the last instruction of this block
        bool m_bCall;                                                           // whether it is a call instruction
        bool m_bEntryCall;                                                      // if it is a call instruction, whether its target address is the entry of a function

};

class CFunction
{
public:
        CFunction(string &szFunc, uint nAddr) { m_szFunc = szFunc; SetStart(nAddr);};
public:
        list<CInstruction *> m_Insts;
        list<CBasicBlock *> m_Blocks;
        string m_szFunc;
private:
        uint m_nStart;
        uint m_nEnd;

public:
	void SetEnd( uint nEnd ) { m_nEnd = nEnd; }
	uint GetEnd() { return m_nEnd; }
	uint GetStart() { return m_nStart; }
	void SetStart(uint nStart) { m_nStart = nStart; }
	string GetName() { return m_szFunc; }

        friend inline ostream & operator << (ostream &os, CFunction &f)
        {
                os << f.m_szFunc;
                return os;
        }
        
};

extern map<uint, CInstruction *> g_hInsts;
extern map<uint, CFunction *> g_hFuncs;
extern list<CBasicBlock *> g_Blocks;
extern CBasicBlock *g_pEntryBlock;
extern CBasicBlock *g_pExitBlock;
extern uint g_nEntryAddress;
extern uint g_nExitAddress;

int CopyCS(vector<int> &source, vector<int> &target, uint n );
int MergeCS(vector<int> &first, const vector<int> &second, vector<int> &target, uint n );
int UnifyCS(vector<int> &first, vector<int> &second, vector<int> &target, uint n );
int ColdMiss(const vector<int> &first, const vector<int> &second, vector<bool> &hit, uint n, int &nHit);
int Dump(ostream &os, const vector<int> &flow, const vector<bool> &hit);

int PosCopyCS(map<int, set<int> > &source, map<int, set<int> > &target, uint n );
int PosMergeCS(map<int, set<int> > &first, map<int, set<int> > &second, map<int, set<int> > &target, uint n );
int PosUnifyCS(map<int, set<int> > &source, map<int, set<int> > &target, uint n );
int PosColdMiss( map<int, set<int> > &first, map<int, set<int> > &second, vector<bool> &hit, uint n, int &nHit);
int PosDump(ostream &os, map<int, set<int> > &flow, const vector<bool> &hit);

int DiffSet( const set<int> &s1, const set<int> &s2);
int CommonSet( const set<int> &s1, const set<int> &s2);

uint hex2dec(const string &szHex);
uint dec2dec(const string &szDec );


#endif