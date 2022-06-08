#include "pch.h"
#include <iostream>

#include "test.h"
#include <thread>
#include <vector>
#include <chrono>
#include"MemoryPool.h"

using namespace std;

enum LOCK_FREE_INDE
{
	THREAD_MAX_NUM = 10,
	NUM_MAX_COUNT = 10000,
};

void Crash()
{
	int* p = nullptr;
	*p = 0;
}

MemoryPool<Test> gMemoryPool(0);
Test* gNode[THREAD_MAX_NUM * NUM_MAX_COUNT];
long gCount = 0;

int Thread()
{
    Test* node[NUM_MAX_COUNT];
    int count = 0;

    while ( (count = InterlockedIncrement(&gCount)) < 500 )
    {
        for (int i = 0; i < NUM_MAX_COUNT; i++)
        {
            node[i] = gMemoryPool.Alloc();
        }

        for (int i = 0; i < NUM_MAX_COUNT; i++)
        {
            if (node[i]->testChar != 20)
                Crash();
            if (node[i]->testInt != 15)
                Crash();
        }

        for (int i = 0; i < NUM_MAX_COUNT; i++)
        {
            InterlockedIncrement((long*)&node[i]->testInt);
            InterlockedIncrement((long*)&node[i]->testChar);
        }
        for (int i = 0; i < NUM_MAX_COUNT; i++)
        {
            gMemoryPool.Free(node[i]);
        }
        for (int i = 0; i < NUM_MAX_COUNT; i++)
        {
            node[i] = gMemoryPool.Alloc();
        }

        for (int i = 0; i < NUM_MAX_COUNT; i++)
        {
            if (node[i]->testChar != 21)
                Crash();
            if (node[i]->testInt != 16)
                Crash();
        }

        for (int i = 0; i < NUM_MAX_COUNT; i++)
        {
            InterlockedDecrement((long*)&node[i]->testInt);
            InterlockedDecrement((long*)&node[i]->testChar);
        }

        for (int i = 0; i < NUM_MAX_COUNT; i++)
        {
            gMemoryPool.Free(node[i]);
        }
    }

    return 0;
}

int main()
{
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

    vector<thread> allocThreads;
    chrono::system_clock::time_point startTime;
    chrono::system_clock::time_point endTime;
    chrono::milliseconds elapsedTime;

	for (int i = 0; i < NUM_MAX_COUNT * THREAD_MAX_NUM; i++)
	{
		gNode[i] = gMemoryPool.Alloc();
	}
	for (int i = 0; i < NUM_MAX_COUNT * THREAD_MAX_NUM; i++)
	{
		gMemoryPool.Free(gNode[i]);
	}

    for (int i = 0; i < THREAD_MAX_NUM; i++)
    {
        allocThreads.emplace_back(Thread);
    }

    //while (true)
    //{
    //    cout << "allockCount: " << gMemoryPool.GetAllockCount() << " useCount: " << gMemoryPool.GetUseCount() << endl;
    //}

    startTime = std::chrono::system_clock::now();

    for (thread& allocThread : allocThreads)
    {
        allocThread.join();
    }

    endTime = chrono::system_clock::now();
    elapsedTime = chrono::duration_cast<chrono::milliseconds>(endTime - startTime);
    cout << "Thread 수행 시간: " << elapsedTime.count() << " AllocCount: " << gMemoryPool.GetAllockCount() << " UseCount: " << gMemoryPool.GetUseCount() << endl;
    return 0;
}