#include "pch.h"
#include <iostream>
#include"MemoryPool.h"
#include "test.h"
#define CRTDBG_MAP_ALLOC 
#include <stdlib.h> 
#include <crtdbg.h>

#ifdef _DEBUG
#define new new( _NORMAL_BLOCK, __FILE__, __LINE__ )                                            
#endif

using namespace std;

int main()
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	MemoryPool<class Test> memoryPool(5, true);
	MemoryPool<class Test>::BlockNodeInfo* blockNode = memoryPool.Alloc();
	memoryPool.Free(blockNode);

	return 0;
}