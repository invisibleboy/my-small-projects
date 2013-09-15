#include "MemoryAllocator.h"

int main(int argc, char *argv[])
{
	if( argc < 5 )
	{
		cerr << "Lack of args: need three args!" << endl;
		return -1;
	}

	ADDRINT nSize;
	stringstream ss(argv[1]);
	ss >> nSize;
	nSize <<= 10;  // in unit of Kilo bytes

	ADDRINT nLineSizeShift;
	stringstream ss1(argv[2]);
	ss1 >> nLineSizeShift;

	string szFile = argv[3];

	bool bStackAllocator = true;
	stringstream ss2(argv[4]);
	ss2 >> bStackAllocator;	
	

	CAllocator *allocator;

if(bStackAllocator)
	allocator = new CStackAllocator(nSize, nLineSizeShift, szFile);
else
	allocator = new CHeapAllocator(nSize, nLineSizeShift, szFile);


	allocator->run();
	return 0;
}
