#ifndef _FLOW_VAL_H
#define _FLOW_VAL_H

#include <map>
#include <set>

using namespace std;

class CFlowVal
{
public:
	CFlowVal() { }
	CFlowVal(int m_nSize);

	const set<int> &At(int index);
	set<int> &operator [] (int index);
	bool IsBottomAt( int index );
	int Size() { return m_nSize; }

	int Assign(  CFlowVal &flowVal );
	CFlowVal &Merge(  CFlowVal &flow1, CFlowVal &flow2 );
	CFlowVal &Unify(  CFlowVal &flowVal );

	int HitMiss( CFlowVal &flowVal, vector<bool> &hit, int &nMiss );


	int Init();
	int Dump(ostream &os, const vector<bool> &hit);
	friend ostream & operator << ( ostream &os, CFlowVal &flow );
	

private:
	int m_nSize;
	map<int, set<int> > m_Data;


};

#endif