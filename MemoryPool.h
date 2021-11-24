#pragma once

template <class Object>
class MemoryPool
{
public:
	struct BlockNodeInfo
	{
	public:
		bool AllocBlockNode(const bool isPlacementNew)
		{
			if (isPlacementNew == true)
			{
				data = (Object *)malloc(sizeof(Object));
				_ASSERT(data != nullptr);
				return true;
			}
			else
			{
				data = new Object;
				_ASSERT(data != nullptr);
				return true;
			}

		}
		void Release(const bool isPlacementNew)
		{
			if (isPlacementNew == true)
			{
				if (data)
				{
					free(data);
					data = nullptr;
				}
			}
			else
			{
				if (data)
				{
					delete data;
					data = nullptr;
				}
			}
		}

	public:
		Object* data = nullptr;
		BlockNodeInfo* next = nullptr;
	};

public:
	explicit MemoryPool(int blockNum, bool isPlacementNew = false)
	{
		mIsPlacementNew = isPlacementNew;

		for (int i = 0; i < blockNum; ++i)
		{
			BlockNodeInfo* newNode = new BlockNodeInfo;
			_ASSERT(newNode != nullptr);
	
			if (newNode->AllocBlockNode(mIsPlacementNew) == true)
			{
				newNode->next = mTopNode;
				mTopNode = newNode;
				// �Ҵ��� ���� üũ
				++mAllocCount;
			}
		}
	}
	virtual ~MemoryPool()
	{
		BlockNodeInfo* deleteNode = mTopNode;
		BlockNodeInfo* tempNode = nullptr;

		while (deleteNode != nullptr)
		{
			tempNode = deleteNode;
			deleteNode = deleteNode->next;

			// ������Ʈ ����
			tempNode->Release(mIsPlacementNew);
			// ��� ����
			if(tempNode)
			{
				delete tempNode;
				tempNode = nullptr;
			}
			// �Ҵ��� ���� ����
			--mAllocCount;
		}
	}
	
public:
	// �� �Ҵ� �Լ�
	BlockNodeInfo* Alloc(void)
	{
		BlockNodeInfo* newNode = nullptr;

		if (mTopNode == nullptr)
		{
			newNode = new BlockNodeInfo;
			_ASSERT(newNode != nullptr);

			// freelist�� ��쿡�� �⺻ ������ ȣ�� ����
			if (newNode->AllocBlockNode(mIsPlacementNew) == true)
			{
				newNode->next = mTopNode;
				mTopNode = newNode;
				// �Ҵ��� ���� üũ
				++mAllocCount;
			}
			else
				return nullptr;
		}

		mFreeNode = mTopNode;
		mTopNode = mFreeNode->next;

		// ������� ���� üũ
		++mUseCount;
		_ASSERT(mFreeNode != nullptr);

		// placement boolean ���� ���� ��쿡�� ������ ȣ��
		if(mIsPlacementNew == true)
			new (mFreeNode->data) Object();

		return mFreeNode;
	};
	// �� ��ȯ ���
	bool Free(BlockNodeInfo* blockNode)
	{
		_ASSERT(blockNode != nullptr);

		blockNode->next = mTopNode;
		mTopNode = blockNode;

		// placement boolean ���� ���� ��쿡�� �Ҹ��� ȣ��
		if (mIsPlacementNew == true)
			blockNode->data->~Object();

		// ������� ���� ����
		--mUseCount;

		return true;
	};

public:
	int GetAllockCount() const { return mAllocCount; }
	int GetUseCount() const { return mUseCount; }

private:
	BlockNodeInfo* mTopNode = nullptr;
	BlockNodeInfo* mFreeNode = nullptr;
	int mAllocCount = 0;
	int mUseCount = 0;
	bool mIsPlacementNew = false;
};

