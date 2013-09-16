#include "MemoryAllocator.h"
#include <assert.h>

CAllocator::CAllocator(ADDRINT nSize, ADDRINT nLineSizeShift, string szTraceFile) 
{
	m_nSize = nSize; 
	m_nLineSizeShift = nLineSizeShift;
	m_szTraceFile = szTraceFile;
}

void CAllocator::run()
{	
	m_32Addr2WriteCount.assign(m_nSize >> m_nLineSizeShift,0);


	readTrace();

	list<TraceE *>::iterator I = m_Trace.begin(), E = m_Trace.end();
	for(; I != E; ++ I)
	{
		TraceE *traceE = *I;
		if(traceE->_bCallOrRet)
		{
			int retv = allocate(traceE);
			if( retv != 0 )
			{
				cerr << "Error: memory overflow!" << endl;
				return;
			}
		}
		else
			deallocate(traceE);
	}

	dump();
}

void CAllocator::readTrace()
{
	ifstream inf;
	inf.open(m_szTraceFile.c_str());
	string szLine;
	while(getline(inf, szLine) )
		if( szLine[0] == ':')
			break;

	do
	{
		TraceE *traceE = NULL;
		string szFunc;
		UINT32 nID;
		bool bCallOrRet;
		ADDRINT nFrameSize;
		UINT64 nWriteCount;

		if(szLine.size() < 1)
			continue;		
		
		stringstream ss(szLine);
		if(szLine[0] == ':' )   // function entry
		{
			ss >> szFunc;
			szFunc = szFunc.substr(1);
			stringstream ss1(szFunc);	
			ss1 >> nID;	

			traceE = new TraceE(szFunc, true, 0, 0, nID);
			m_Id2TraceE[nID] = traceE;			
			
		}
		else   // function exit
		{
			ss >> nID;			
			ss >> nFrameSize;
			ss >> nWriteCount;
			traceE = new TraceE(szFunc, false, nFrameSize, nWriteCount, nID);
			
			TraceE *old = m_Id2TraceE[nID];
			if( old == NULL )
				cerr << "No entry for " << nID << endl;
			old->_nFrameSize = traceE->_nFrameSize;
			old->_nWriteCount = traceE->_nWriteCount;					
		}		
		
		m_Trace.push_back(traceE);		
	}while(getline(inf, szLine));
	inf.close();
}

void CAllocator::print(string szOutFile)
{			
	
	ofstream outf;
	outf.open(szOutFile.c_str());


	ADDRINT index = 0;
	ADDRINT nLines = m_nSize >> m_nLineSizeShift;
	for(; index < nLines; ++ index )
	{		
		outf << hex << (index << m_nLineSizeShift) << "\t" << m_32Addr2WriteCount[index] << endl;
	}
	

	outf.close();
}

void CStackAllocator::dump()
{
	if( !m_Blocks.empty() )	
		cerr << m_Blocks.front()->_nID << " is not popped!" << endl;
	string szOutFile = m_szTraceFile + ".stack";
	print(szOutFile);
}



int CStackAllocator::allocate(TraceE *traceE)
{
	if(traceE->_nFrameSize == 0 )
		return 0;
	MemBlock *newBlock = NULL, *lastBlock = NULL;
	
	if( m_Blocks.empty() )
	{
		newBlock = new MemBlock(m_nSize-1, traceE->_nFrameSize);
	}
	else
	{		
		lastBlock = m_Blocks.front();
		ADDRINT addr = lastBlock->_nStartAddr-lastBlock->_nSize;
		newBlock = new MemBlock(addr, traceE->_nFrameSize);
		
		//cerr << lastBlock->_nStartAddr << "--" << lastBlock->_nSize << endl;
		//cerr << "==allocate " << traceE->_nID << "-" << traceE->_nFrameSize << endl;
		assert(lastBlock->_nStartAddr > lastBlock->_nSize );
	}
	
	
	m_Blocks.push_front(newBlock);
	
	

	// update write count
	ADDRINT startIndex = newBlock->_nStartAddr >> m_nLineSizeShift;
	ADDRINT endIndex = (newBlock->_nStartAddr - newBlock->_nSize + 1 ) >> m_nLineSizeShift;
	 
	for(ADDRINT index = startIndex; index > endIndex; -- index )
	{
		this->m_32Addr2WriteCount[index] += traceE->_nWriteCount/(startIndex-endIndex+1);
	}
	
	return 0;
}

void CStackAllocator::deallocate(TraceE *traceE)
{
	if(traceE->_nFrameSize == 0 )
		return;
	//cerr << "==deallocate " << traceE->_nID << "-" << traceE->_nFrameSize << endl;
	if( m_Blocks.empty() )
		cerr << "No entry for traceE->_nID! " << endl;
	MemBlock *topBlock = m_Blocks.front();
	
	m_Blocks.pop_front();
}

void CHeapAllocator::dump()
{
	string szOutFile = m_szTraceFile + ".heap";
	print(szOutFile);
}

int CHeapAllocator::allocate(TraceE *traceE)
{
	if(traceE->_nFrameSize == 0 )
		return 0;
	int retv = 1;
	MemBlock *newBlock = NULL;
	list<MemBlock *>::iterator I = m_lastPlace, E = m_freeBlocks.end();
	do
	{
		MemBlock *block = *I;

		// 1. Delayed block merging
		list<MemBlock *>::iterator J = I;
		++ J;
		while( J != E && block->_nStartAddr - block->_nSize == (*J)->_nStartAddr )
		{
			block->_nSize += (*J)->_nSize;
			list<MemBlock *>::iterator T = J;
			++J;
			// release the merged block
			delete *T;
			m_freeBlocks.erase(T);
						
		}
		// 2. search the free block to use
		// if found the place
		if(block->_nSize >= traceE->_nFrameSize )
		{
			// allocate a new block and push it on the stack
			newBlock = new MemBlock(block->_nStartAddr, traceE->_nFrameSize);
			//cerr << hex << "alloc: " << block->_nStartAddr << "--" << traceE->_nFrameSize << endl;
			m_hId2Block[traceE->_nID] = newBlock;

			// split the block to use
			block->_nSize -= traceE->_nFrameSize;
			block->_nStartAddr -= traceE->_nFrameSize;

			// update last place
			m_lastPlace = I;
			if( block->_nSize == 0 )
			{
				delete block;
				list<MemBlock *>::iterator J = I;
				++ J;
				m_lastPlace = J;				
				m_freeBlocks.erase(I);
				
			}	

			retv = 0;
			break;
		}
		if( ++ I == E )		
			I = m_freeBlocks.begin();	
	}while( I != m_lastPlace );
	
	if( retv != 0 )
		return retv;	
	

	// 3. update write count
	ADDRINT startIndex = newBlock->_nStartAddr >> m_nLineSizeShift;
	ADDRINT endIndex = (newBlock->_nStartAddr - newBlock->_nSize + 1 ) >> m_nLineSizeShift;
	 
	for(ADDRINT index = startIndex; index > endIndex; -- index )
	{
		this->m_32Addr2WriteCount[index] += traceE->_nWriteCount/(startIndex-endIndex+1);
	}	
	return 0;
}

void CHeapAllocator::deallocate(TraceE *traceE)
{
	if(traceE->_nFrameSize == 0 )
		return ;
	MemBlock *block = m_hId2Block[traceE->_nID];
	assert(block->_nSize = traceE->_nFrameSize);	
	
	// if found the place to release back the used block
	list<MemBlock *>::iterator I = m_freeBlocks.begin(), E = m_freeBlocks.end();
	for(; I != E; ++ I )
	{
		MemBlock *nextBlock = *I;
		// if found the place to return back the used block
		if(nextBlock->_nStartAddr < block->_nStartAddr )
		{
			m_freeBlocks.insert(I, block);
			break;
		}
	}

	// if not found the next block
	if( I == E )
	{
		m_freeBlocks.push_back(block);
	}	
}


