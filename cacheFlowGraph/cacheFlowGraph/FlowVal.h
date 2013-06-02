#ifndef _FLOW_VAL_H
#define _FLOW_VAL_H

#include <map>
#include <set>
typedef unsigned int uint;

using namespace std;

class CDataFlowVal
{
public:
	CDataFlowVal() {};
	CDataFlowVal( uint nSize ) {m_nSize = nSize;};

	int Size() {return m_nSize;}

public:
	virtual CDataFlowVal &Meet(CDataFlowVal &dfv) { return *this;};
	virtual CDataFlowVal &Unify(CDataFlowVal &dfv) {return *this;};
	virtual CDataFlowVal &Assign(CDataFlowVal &dfv) {return *this;};
	virtual bool operator == ( CDataFlowVal &dfv){return true;};


protected:
	uint m_nSize;
};

class CSimpleFlowVal : public CDataFlowVal
{
public:
	CSimpleFlowVal(uint nSize, bool bValue = true) { m_nSize = nSize; _value.assign(nSize, bValue); };
	
	void Set(uint index, bool bValue) { _value[index] = bValue;};
	bool Value(uint index) { return _value[index]; }

	CSimpleFlowVal &Assign(CSimpleFlowVal &sfv);
	CSimpleFlowVal &Meet(CSimpleFlowVal &sfv);
	CSimpleFlowVal &Unify(CSimpleFlowVal &sfv);
	bool operator == ( CSimpleFlowVal &sfv);
private:
	vector<bool> _value;

};

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