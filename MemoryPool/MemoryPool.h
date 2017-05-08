/*---------------------------------------------------------------

	procademy MemoryPool.

	�޸� Ǯ Ŭ����.
	Ư�� ����Ÿ(����ü,Ŭ����,����)�� ������ �Ҵ� �� ��������.

	- ����.

	procademy::CMemoryPool<DATA> MemPool(300, FALSE);
	DATA *pData = MemPool.Alloc();

	pData ���

	MemPool.Free(pData);


	!.	���� ���� ���Ǿ� �ӵ��� ������ �� �޸𸮶�� �����ڿ���
		Lock �÷��׸� �־� ����¡ ���Ϸ� ���縦 ���� �� �ִ�.
		���� �߿��� ��찡 �ƴ��̻� ��� ����.

		
		
		���ǻ��� :	�ܼ��� �޸� ������� ����Ͽ� �޸𸮸� �Ҵ��� �޸� ����� �����Ͽ� �ش�.
					Ŭ������ ����ϴ� ��� Ŭ������ ������ ȣ�� �� Ŭ�������� �Ҵ��� ���� ���Ѵ�.
					Ŭ������ �����Լ�, ��Ӱ��谡 ���� �̷����� �ʴ´�.
					VirtualAlloc ���� �޸� �Ҵ� �� memset ���� �ʱ�ȭ�� �ϹǷ� Ŭ���������� ���� ����.
		
				
----------------------------------------------------------------*/
#ifndef  __MEMORYPOOL__H__
#define  __MEMORYPOOL__H__s
#include <assert.h>
#include <new.h>


	template <class DATA>
	class CMemoryPool
	{
	private:

		/* **************************************************************** */
		// �� �� �տ� ���� ��� ����ü.
		/* **************************************************************** */
		struct st_BLOCK_NODE
		{
			st_BLOCK_NODE()
			{
				stpNextBlock = NULL;
			}
			st_BLOCK_NODE *stpNextBlock;
		};

		/* **************************************************************** */
		// ������ �޸� Ǯ�� ž ���
		/* **************************************************************** */
		struct st_TOP_NODE
		{
			st_BLOCK_NODE *pNode;
			__int64 iUniqueNum;
		};

	public:

		//////////////////////////////////////////////////////////////////////////
		// ������, �ı���.
		//
		// Parameters:	(int) �ִ� �� ����.
		//				(bool) �޸� Lock �÷��� - �߿��ϰ� �ӵ��� �ʿ�� �Ѵٸ� Lock.
		// Return:
		//////////////////////////////////////////////////////////////////////////
		CMemoryPool(int iBlockNum, bool bLockFlag = false)
		{
			////////////////////////////////////////////////////////////////
			// TOP ��� �Ҵ�
			////////////////////////////////////////////////////////////////
			_pTopNode = (st_TOP_NODE *)_aligned_malloc(sizeof(st_TOP_NODE), 16);
			_pTopNode->pNode = NULL;
			_pTopNode->iUniqueNum = 0;

			_iUniqueNum = 0;

			////////////////////////////////////////////////////////////////
			// �޸� Ǯ ũ�� ����
			////////////////////////////////////////////////////////////////
			m_iBlockCount = iBlockNum;

			if (iBlockNum < 0)	return;

			else if (iBlockNum == 0)
			{
				m_bStoreFlag = true;
				m_stBlockHeader = NULL;
			}

			////////////////////////////////////////////////////////////////
			// DATA * ���� ũ�⸸ ŭ �޸� �Ҵ�
			////////////////////////////////////////////////////////////////
			m_stBlockHeader = new char[(sizeof(DATA) + sizeof(st_BLOCK_NODE)) * m_iBlockCount];

			_pTopNode = (st_BLOCK_NODE *)m_stBlockHeader;
			char *pBlock = (char *)m_stpTop;
			st_BLOCK_NODE *stpNode = m_stpTop;

			////////////////////////////////////////////////////////////////
			// BLOCK ����
			////////////////////////////////////////////////////////////////
			for (int iCnt = 0; iCnt < m_iBlockCount - 1; iCnt++)
			{
				pBlock += sizeof(DATA) + sizeof(st_BLOCK_NODE);
				stpNode->stpNextBlock = (st_BLOCK_NODE *)pBlock;
				stpNode = stpNode->stpNextBlock;
			}

			stpNode->stpNextBlock = NULL;
		}

		virtual	~CMemoryPool()
		{
			delete []m_stBlockHeader;
		}

		//////////////////////////////////////////////////////////////////////////
		// �� �ϳ��� �Ҵ�޴´�.
		//
		// Parameters: ����.
		// Return: (DATA *) ����Ÿ �� ������.
		//////////////////////////////////////////////////////////////////////////
		DATA	*Alloc(bool bPlacementNew = true)
		{
			st_BLOCK_NODE *stpBlock;
			st_TOP_NODE pPreTopNode;
			st_BLOCK_NODE *pNode, *pNewTopNode;

			if (m_iBlockCount <= m_iAllocCount)
			{
				if (m_bStoreFlag)
				{
					stpBlock = (st_BLOCK_NODE *)new char[(sizeof(st_BLOCK_NODE) + sizeof(DATA))];
					InterlockedIncrement64((LONG64 *)&m_iBlockCount);
				}

				else
					return nullptr;
			}

			else
			{
				st_TOP_NODE pPreTopNode;
				st_BLOCK_NODE *pNode;
				__int64 iUniqueNum = InterlockedIncrement64(&_iUniqueNum);

				do
				{
					pPreTopNode.iUniqueNum = _pTop->iUniqueNum;
					pPreTopNode.pTopNode = _pTop->pTopNode;

					pNode = _pTop->pTopNode;
				} while (!InterlockedCompareExchange128((volatile LONG64 *)_pTop, iUniqueNum, (LONG64)_pTop->pTopNode->pNext, (LONG64 *)&pPreTopNode));
				*pOutData = pPreTopNode.pTopNode->Data;
				delete pNode;
		
				stpBlock = m_stpTop;
				m_stpTop = stpBlock->stpNextBlock;
			}

			InterlockedIncrement64((LONG64 *)&m_iAllocCount);
			return (DATA *)(stpBlock + 1);
		}

		//////////////////////////////////////////////////////////////////////////
		// ������̴� ���� �����Ѵ�.
		//
		// Parameters: (DATA *) �� ������.
		// Return: (BOOL) TRUE, FALSE.
		//////////////////////////////////////////////////////////////////////////
		bool	Free(DATA *pData)
		{
			st_BLOCK_NODE *stpBlock;
			st_TOP_NODE pPreTopNode;

			stpBlock = ((st_BLOCK_NODE *)pData - 1);
			stpBlock->stpNextBlock = _pTopNode;

			__int64 iUniqueNum = InterlockedIncrement64(&_iUniqueNum);

			do {
				pPreTopNode.pTopNode = _pTop->pTopNode;
				pPreTopNode.iUniqueNum = _pTop->iUniqueNum;

				pNode->Data = Data;
				pNode->pNext = _pTop->pTopNode;
			} while (!InterlockedCompareExchange128((volatile LONG64 *)_pTop, iUniqueNum, (LONG64)pNode, (LONG64 *)&pPreTopNode));

			InterlockedDecrement64((LONG64 *)&m_iAllocCount);
			return false;
		}


		//////////////////////////////////////////////////////////////////////////
		// ���� ������� �� ������ ��´�.
		//
		// Parameters: ����.
		// Return: (int) ������� �� ����.
		//////////////////////////////////////////////////////////////////////////
		int		GetAllocCount(void) { return m_iAllocCount; }

	private:
		//////////////////////////////////////////////////////////////////////////
		// ��� ������ ž
		//////////////////////////////////////////////////////////////////////////
		st_TOP_NODE *_pTopNode;

		//////////////////////////////////////////////////////////////////////////
		// ž�� Unique Number
		//////////////////////////////////////////////////////////////////////////
		__int64 _iUniqueNum;

		//////////////////////////////////////////////////////////////////////////
		// ��� ����ü ���
		//////////////////////////////////////////////////////////////////////////
		char *m_stBlockHeader;

		//////////////////////////////////////////////////////////////////////////
		// �޸� Lock �÷���
		//////////////////////////////////////////////////////////////////////////
		bool m_bLockFlag;

		//////////////////////////////////////////////////////////////////////////
		// �޸� ���� �÷���, true�� ������ �����Ҵ� ��
		//////////////////////////////////////////////////////////////////////////
		bool m_bStoreFlag;

		//////////////////////////////////////////////////////////////////////////
		// ���� ������� �� ����
		//////////////////////////////////////////////////////////////////////////
		int m_iAllocCount;

		//////////////////////////////////////////////////////////////////////////
		// ��ü �� ����
		//////////////////////////////////////////////////////////////////////////
		int m_iBlockCount;
	};

#endif