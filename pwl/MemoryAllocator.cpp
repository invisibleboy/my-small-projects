#include "MemoryAllocator.h"
#include <assert.h>

CAllocator::CAllocator(ADDRINT nSizePower, ADDRINT nLineSizeShift, string szTraceFile) 
{
	m_nSize = 1 << nSizePower;  // in power	
	m_nSizePower = nSizePower;
	m_nLineSizeShift = nLineSizeShift;
	m_szTraceFile = szTraceFile;
}

void CAllocator::run()
{	
	m_32Addr2WriteCount.assign(m_nSize >> m_nLineSizeShift,0.0);


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

	map<UINT32, TraceE*> hId2Entry;
	// skip the prolog
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
			ss1 >>hex>> nID;	

			traceE = new TraceE(szFunc, true, 16, 0, nID);
			hId2Entry[nID] = traceE;			
			
		}
		else   // function exit
		{
			ss >>hex >> nID;			
			ss >>hex >> nFrameSize;
			ss >>hex >> nWriteCount;
			// assume there are four push instructions on average
			traceE = new TraceE(szFunc, false, nFrameSize+16, nWriteCount, nID);
		    
			
			TraceE *old = hId2Entry[nID];
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
		outf << hex << (index << m_nLineSizeShift) << "\t" <<dec << m_32Addr2WriteCount[index] << endl;
	}
	

	outf.close();
}

void CStackAllocator::dump()
{
	if( !m_Blocks.empty() )	
		cerr << hex << m_Blocks.front()->_nID << " is not popped!" <<dec << endl;
	char digits[16];
	sprintf(digits, "%d", m_nSizePower);
	string szOutFile = m_szTraceFile + "_" + digits;
	sprintf(digits, "%d", 1);
	szOutFile = szOutFile + "_" + digits;
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
		if(lastBlock->_nStartAddr <= lastBlock->_nSize )
		{
			cerr << hex << lastBlock->_nStartAddr << " cannot afford " << lastBlock->_nSize << " bytes!" << endl;
			assert(false );
		}
		ADDRINT addr = lastBlock->_nStartAddr-lastBlock->_nSize;		
		
		//cerr << lastBlock->_nStartAddr << "--" << lastBlock->_nSize << endl;
		//cerr << "==allocate " << traceE->_nID << "-" << traceE->_nFrameSize << endl;
		
		newBlock = new MemBlock(addr, traceE->_nFrameSize);
	}
	
	
	m_Blocks.push_front(newBlock);
	
	

	// update write count
	ADDRINT startIndex = newBlock->_nStartAddr >> m_nLineSizeShift;
	ADDRINT endIndex = (newBlock->_nStartAddr - newBlock->_nSize + 1 ) >> m_nLineSizeShift;
	 
    double dAvgCount = ((double)traceE->_nWriteCount)/(startIndex-endIndex+1);
    //cerr <<hex << "dealloc " << traceE->_nID << ":" << traceE->_nFrameSize << ":"<< traceE->_nWriteCount << ":" << nAvgCount << endl;
	for(ADDRINT index = startIndex; index >= endIndex; -- index )
	{
		this->m_32Addr2WriteCount[index] += dAvgCount;
		if( index == 0 )
			break;
	}
	
	return 0;
}

void CStackAllocator::deallocate(TraceE *traceE)
{
	if(traceE->_nFrameSize == 0 )
		cerr << "Frame size is zero!" << endl;
	//cerr << "==deallocate " << traceE->_nID << "-" << traceE->_nFrameSize << endl;
	if( m_Blocks.empty() )
		cerr << "No entry for traceE->_nID! " << endl;
	MemBlock *topBlock = m_Blocks.front();
	delete topBlock;
	m_Blocks.pop_front();
}

void CHeapAllocator::dump()
{
	char digits[16];
	sprintf(digits, "%d", m_nSizePower);
	string szOutFile = m_szTraceFile + "_" + digits;
	sprintf(digits, "%d", 0);
	szOutFile = szOutFile + "_" + digits;
	print(szOutFile);
}

int CHeapAllocator::allocate(TraceE *traceE)
{
	if(traceE->_nFrameSize == 0 )
	{
		cerr << "Frame size is zero!" << endl;
		return 0;
	}
	int retv = 1;
	MemBlock *newBlock = NULL;
	bool bMergeLastPlace = false;
	list<MemBlock *>::iterator I = m_lastPlace, E = m_freeBlocks.end();
	do
	{
		MemBlock *block = *I;

		// 1. Delayed block merging		
		list<MemBlock *>::iterator J = I;
		++ J;
		while( J != E && block->_nStartAddr - block->_nSize == (*J)->_nStartAddr )
		{
			if( J == m_lastPlace )
			{
				bMergeLastPlace = true;
				m_lastPlace == I;
			}
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
				if( m_lastPlace == E )
					m_lastPlace = m_freeBlocks.begin();			
				m_freeBlocks.erase(I);
				
			}	

			retv = 0;
			break;
		}
		if( ++ I == E )		
			I = m_freeBlocks.begin();	
	}while( I != m_lastPlace && !bMergeLastPlace);
	
	if( retv != 0 )
	{
		cerr << "While allocating a frame of " << hex << traceE->_nFrameSize << dec << endl;
		return retv;
	}	
	

	// 3. update write count
	ADDRINT startIndex = newBlock->_nStartAddr >> m_nLineSizeShift;
	ADDRINT endIndex = (newBlock->_nStartAddr - newBlock->_nSize + 1 ) >> m_nLineSizeShift;
	 
	double dAvgCount = ((double)traceE->_nWriteCount)/(startIndex-endIndex+1);
	for(ADDRINT index = startIndex; index >= endIndex; -- index )
	{
		this->m_32Addr2WriteCount[index] += dAvgCount;
		if(index == 0 )
			break;
	}	
	return 0;
}

void CHeapAllocator::deallocate(TraceE *traceE)
{
	if(traceE->_nFrameSize == 0 )
	{
		cerr << "Frame size is zero!" << endl;
		return ;
	}
	MemBlock *block = m_hId2Block[traceE->_nID];
	assert(block->_nSize = traceE->_nFrameSize);	
	//cerr << hex << "dealloc: " << block->_nStartAddr << "--" << traceE->_nFrameSize << endl;	
	if( block->_nStartAddr == 0x12d7 )
		block = block;

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


