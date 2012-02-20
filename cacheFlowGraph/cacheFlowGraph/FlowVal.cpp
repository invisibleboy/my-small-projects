#include <iomanip>
#include <assert.h>
#include "DataType.h"
#include "FlowVal.h"


CFlowVal::CFlowVal(int nSize=CACHE_SET)
{
	m_nSize = nSize;
	Init();
}

int CFlowVal::Assign( CFlowVal &flowVal)
{
	int nDiff = 0;
	for( int i = 0; i < flowVal.Size(); ++ i)
    {
		const set<int> &src = flowVal.At(i);
		set<int> &des = m_Data[i];

		if( DiffSet( src, des ) )
		{
			nDiff = 1;
			des.clear();
			des.insert( src.begin(), src.end() );
		}
    }
    return nDiff;
}

CFlowVal &CFlowVal::Merge( CFlowVal &flow1, CFlowVal &flow2)
{
	for( int i = 0; i < flow1.Size(); ++ i )
	{
		const set<int> &first = flow1.At(i);
		const set<int> &second = flow2.At(i);
		set<int> &target = m_Data[i];

		if( flow2.IsBottomAt(i) )  // include both flow1 and flow2/CACHE::BOTTOM
		{
			if( &flow1 == this )   
			{
				// this possile empty state is subsitituted by flow1
				set<int>::const_iterator i_p = second.begin(), i_e = second.end();
				for(; i_p != i_e; ++ i_p )
				{
					if( *i_p != CACHE::BOTTOM )
						target.insert( *i_p );
				}
			}
			else
			{				
				target.clear();
				target.insert( first.begin(), first.end() );
				// this possile empty state is subsitituted by flow1
				set<int>::const_iterator i_p = second.begin(), i_e = second.end();
				for(; i_p != i_e; ++ i_p )
				{
					if( *i_p != CACHE::BOTTOM )
						target.insert( *i_p );
				}			
			}
		}
		else	// include only flow1
		{
			target.clear();
			target.insert( second.begin(), second.end() );
		}
	}
	return *this;
}

CFlowVal &CFlowVal::Unify(CFlowVal &flow)
{
	for( int i = 0; i < flow.Size(); ++ i )
	{
		const set<int> &second = flow.At(i);
		set<int> &target = m_Data[i];
		target.insert( second.begin(), second.end() );
	}
	return *this;
}


const set<int> & CFlowVal::At(int index)
{
	assert( index >= 0 && index < m_nSize );
	const set<int> &element = m_Data[index];
	return element;
}

set<int> & CFlowVal::operator [] (int index)
{	
	assert( index >= 0 && index < m_nSize );
	set<int> &element = m_Data[index];
	return element;
}

bool CFlowVal::IsBottomAt(int index )
{
	assert( index >= 0 && index < m_nSize );
	const set<int> &element = At(index);
	if( element.find( CACHE::BOTTOM ) != element.end() )
		return true;
	return false;
}

int CFlowVal::HitMiss( CFlowVal &flow, vector<bool> &hit, int &nMiss )
{
	int nHit = 0;
	hit.assign(m_nSize, false);
       
    for( uint i = 0; i < m_nSize; ++ i)
    {
		const set<int> &second = flow.At(i);
		const set<int> &first = m_Data[i];

		if( flow.IsBottomAt(i) && second.size() == 1 )  // skip the empty cache line
			continue;
		
		set<int>::const_iterator i_p = second.begin(), i_e = second.end();
		for(; i_p != i_e; ++ i_p )
		{
			if( *i_p == CACHE::BOTTOM  )
				continue;
			if( first.find( *i_p ) != first.end() )
			{
				hit[i] = true;
				++ nHit;
				break;
			}
		}


		if( hit[i] != true )
			++ nMiss;
    }
	return nHit;
}

int CFlowVal::Init()
{
	for( int i = 0; i < m_nSize; ++ i )
		m_Data[i].insert( CACHE::BOTTOM );
	return 0;
}

int CFlowVal::Dump(ostream &os, const vector<bool> &hit)
{
	for( int i = 0; i < m_nSize; ++ i )
    {
        if( i%16 == 0 )
            os << endl;
		if( hit[i] )
			os << "H";
		const set<int> &pos = this->At(i);
		set<int>::const_iterator s_p = pos.begin(), s_e = pos.end();
		for(; s_p != s_e; ++ s_p )
			os << setw(5) << *s_p << ",";
		os << ":\t";
    }

	return 0;
}

ostream & operator << (ostream &os, CFlowVal &flow )
{
	for( int i = 0; i < flow.Size(); ++ i )
    {
        if( i%16 == 0 )
            os << endl;
		const set<int> &pos = flow.At(i);
		set<int>::const_iterator s_p = pos.begin(), s_e = pos.end();
		for(; s_p != s_e; ++ s_p )
			os << setw(5) << *s_p << ",";
		os << ":\t";
    }
    return os;
}