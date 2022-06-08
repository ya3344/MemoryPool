#pragma once
// �����͸� �����ֱ� ������ ���������� ���� �� �� �ִ�.

template <class Object>
class MemoryPool
{
public:
	enum LOCK_FREE_INDE
	{
		MEMORY_LOG_MAX_INDEX = 10000,
		ALLOC_NULLPTR = 0,
		ALLOC_BEFORE,
		ALLOC_BEFORE_AFTER,
		ALLOC_AFTER,
		FREE,
	};

public:
	struct NodeInfo
	{
		Object data;
		NodeInfo* next = nullptr;
	};
	struct __declspec(align(16)) BlockNodeInfo
	{
	public:
		NodeInfo* node = nullptr;
		__int64 popCount = 0;
	};

public: // Debug ��
	struct MemoryLogInfo
	{
		int* topAddress = nullptr;
		int* topAddressNext = nullptr;
		unsigned int threadID = 0;
		char funcName = -1;
		long threadCount = 0;
	};

public:
	MemoryPool()
	{
		mIsPlacementNew = false;
	}
	explicit MemoryPool(int blockNum, bool isPlacementNew = false)
	{
		mIsPlacementNew = isPlacementNew;
		NodeInfo* newNode = nullptr;

		for (int i = 0; i < blockNum; ++i)
		{
			if (mIsPlacementNew == true)
				newNode = (NodeInfo*)malloc(sizeof(NodeInfo));
			else
				newNode = new NodeInfo;

			_ASSERT(newNode != nullptr);
	
			newNode->next = mTopNode.node;
			mTopNode.node = newNode;

			// �Ҵ��� ���� üũ
			++mAllocCount;
		}
	}
	virtual ~MemoryPool()
	{
		NodeInfo* deleteNode = mTopNode.node;
		NodeInfo* tempNode = nullptr;

		while (deleteNode != nullptr)
		{
			tempNode = deleteNode;
			deleteNode = deleteNode->next;

			// ��� ����
			if(tempNode)
			{
				if (mIsPlacementNew == true)
					free(tempNode);
				else
					delete tempNode;

				tempNode = nullptr;
			}
			// �Ҵ��� ���� ����
			--mAllocCount;
		}
	}
	
public:
	void Initialize(int blockNum, bool isPlacementNew = false)
	{
		mIsPlacementNew = isPlacementNew;
		NodeInfo* newNode = nullptr;

		for (int i = 0; i < blockNum; ++i)
		{
			if (mIsPlacementNew == true)
				newNode = (NodeInfo*)malloc(sizeof(NodeInfo));
			else
				newNode = new NodeInfo;

			_ASSERT(newNode != nullptr);

			newNode->next = mTopNode.node;
			mTopNode.node = newNode;

			// �Ҵ��� ���� üũ
			++mAllocCount;
		}
	}
public: // debug �� �Լ�
	void MemoryLog(const char funcName, const unsigned int threadID, int* topAddress, int* topAddressNext)
	{
		long memoryLogIndex = 0;
		memoryLogIndex = InterlockedIncrement(&mMemoryLogIndex);
		memoryLogIndex %= MEMORY_LOG_MAX_INDEX;

		mMemoryLog[memoryLogIndex].funcName = funcName;
		mMemoryLog[memoryLogIndex].threadID = threadID;
		mMemoryLog[memoryLogIndex].topAddress = topAddress;
		mMemoryLog[memoryLogIndex].topAddressNext = topAddressNext;
		mMemoryLog[memoryLogIndex].threadCount = InterlockedIncrement(&mThreadCount);
	}

public:
	// �� �Ҵ� �Լ�
	Object* Alloc(void)
	{
		NodeInfo* newNode = nullptr;
		BlockNodeInfo oldNode;
		BlockNodeInfo nextNode;

		do
		{
			oldNode = mTopNode;
			if (oldNode.node == nullptr)
			{
				NodeInfo* newNode = nullptr;

				if (mIsPlacementNew == true)
					newNode = (NodeInfo*)malloc(sizeof(NodeInfo));
				else
					newNode = new NodeInfo;
				
				_ASSERT(newNode != nullptr);

				// �Ҵ��� ���� üũ
				InterlockedIncrement((long*)&mAllocCount);
				// ��� ���� üũ
				InterlockedIncrement((long*)&mUseCount);
				MemoryLog(ALLOC_NULLPTR, GetCurrentThreadId(), (int*)newNode, nullptr);

				// placement boolean ���� ���� ��쿡�� ������ ȣ��
				if (mIsPlacementNew == true)
					new (&(newNode->data)) Object();

				return (Object*)newNode;
			}
			MemoryLog(ALLOC_BEFORE, GetCurrentThreadId(), (int*)oldNode.node, (int*)oldNode.node->next);
			nextNode.node = oldNode.node->next;
			nextNode.popCount = oldNode.popCount;
			InterlockedIncrement64(&nextNode.popCount);
			MemoryLog(ALLOC_BEFORE_AFTER, GetCurrentThreadId(), (int*)oldNode.node, (int*)oldNode.node->next);
		} while (InterlockedCompareExchange128((LONG64*)&mTopNode, (LONG64)nextNode.popCount, 
			(LONG64)nextNode.node, (LONG64*)&oldNode) == FALSE);

		// placement boolean ���� ���� ��쿡�� ������ ȣ��
		if (mIsPlacementNew == true)
			new (&(oldNode.node->data)) Object();

		// ������� ���� üũ
		_InterlockedIncrement((long*)&mUseCount);

		return (Object*)oldNode.node;
	};
	// �� ��ȯ �Լ�
	bool Free(Object* data)
	{
		_ASSERT(data != nullptr);

		BlockNodeInfo oldNode;
		BlockNodeInfo nextNode;

		do
		{
			oldNode = mTopNode;
			nextNode.node = (NodeInfo*)data;
			nextNode.node->next = mTopNode.node;
		} while (InterlockedCompareExchange128((LONG64*)&mTopNode, (LONG64)nextNode.popCount, 
			(LONG64)nextNode.node, (LONG64*)&oldNode) == FALSE);
		
		// placement boolean ���� ���� ��쿡�� �Ҹ��� ȣ��
		if (mIsPlacementNew == true)
			mTopNode.node->data.~Object();

		// ������� ���� ����
		InterlockedDecrement((long*)&mUseCount);

		return true;
	};

public:
	inline int GetAllockCount() const { return mAllocCount; }
	inline int GetUseCount() const { return mUseCount; }

private:
	BlockNodeInfo mTopNode;
	__declspec(align(64)) int mAllocCount = 0;
	__declspec(align(64)) int mUseCount = 0;
	bool mIsPlacementNew = false;

private: // Debug �� ����
	MemoryLogInfo mMemoryLog[MEMORY_LOG_MAX_INDEX];
	long mMemoryLogIndex = -1;
	long mThreadCount = 0;
};

