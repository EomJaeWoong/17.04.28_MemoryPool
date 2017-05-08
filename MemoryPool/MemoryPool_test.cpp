#include <windows.h>
#include <stdio.h>
#include <process.h>
#include <time.h>
#include <stdlib.h>

#include "MemoryPool.h"
#include "MemoryPool_test.h"

/*
�׽�Ʈ ����
- ���� ������ ������ ���� ������ ������ ��ġ Ȯ��
- �����͸� �־��ٰ� ���� �ڿ� �� �����͸� �ٸ��̰� �޸𸮸� ����ϴ��� Ȯ�� (2������ �������� Ȯ��)
struct st_TEST_DATA
{
volatile LONG64	lData;
volatile LONG64	lCount;
};
//�� ����ü�� �����͸� �ٷ�� ���� ���� ����.
//CLockfreeStack<st_TEST_DATA *> g_Stack();
*/

CMemoryPool<st_TEST_DATA *> g_Mempool;

LONG64 lPushTPS = 0;
LONG64 lPopTPS = 0;

LONG64 lPushCounter = 0;
LONG64 lPopCounter = 0;

unsigned __stdcall StackThread(void *pParam);

void main()
{
	HANDLE hThread[dfTHREAD_MAX];
	DWORD dwThreadID;



	for (int iCnt = 0; iCnt < dfTHREAD_MAX; iCnt++)
	{
		hThread[iCnt] = (HANDLE)_beginthreadex(
			NULL,
			0,
			StackThread,
			(LPVOID)1000,
			0,
			(unsigned int *)&dwThreadID
			);
	}

	while (1)
	{
		lPushTPS = lPushCounter;
		lPopTPS = lPopCounter;

		lPushCounter = 0;
		lPopCounter = 0;

		wprintf(L"------------------------------------------------\n");
		wprintf(L"Push TPS : %d\n", lPushTPS);
		wprintf(L"Pop TPS : %d\n", lPopTPS);
		wprintf(L"------------------------------------------------\n\n");

		Sleep(999);
	}
}

/*------------------------------------------------------------------*/
// 0. �� �����忡�� st_QUEUE_DATA �����͸� ���� ��ġ (10000��) ����		
// 0. ������ ����(Ȯ��)
// 1. iData = 0x0000000055555555 ����
// 1. lCount = 0 ����
// 2. ���ÿ� ����

// 3. �ణ���  Sleep (0 ~ 3)
// 4. ���� ���� ������ �� ��ŭ ���� 
// 4. - �̶� �����°� ���� ���� �������� ����, �ٸ� �����尡 ���� �������� ���� ����
// 5. ���� ��ü �����Ͱ� �ʱⰪ�� �´��� Ȯ��. (�����͸� ���� ����ϴ��� Ȯ��)
// 6. ���� ��ü �����Ϳ� ���� lCount Interlock + 1
// 6. ���� ��ü �����Ϳ� ���� iData Interlock + 1
// 7. �ణ���
// 8. + 1 �� �����Ͱ� ��ȿ���� Ȯ�� (���� �����͸� ���� ����ϴ��� Ȯ��)
// 9. ������ �ʱ�ȭ (0x0000000055555555, 0)
// 10. ���� �� ��ŭ ���ÿ� �ٽ� ����
//  3�� ���� �ݺ�.
/*------------------------------------------------------------------*/
unsigned __stdcall StackThread(void *pParam)
{
	srand(time(NULL) + (int)pParam);

	int iRand, iCnt;
	st_TEST_DATA *pData;
	st_TEST_DATA *pDataArray[dfTHREAD_ALLOC];

	iRand = rand() % dfTHREAD_ALLOC;

	for (iCnt = 0; iCnt < dfTHREAD_ALLOC; iCnt++)
	{
		pDataArray[iCnt] = new st_TEST_DATA;
		pDataArray[iCnt]->lData = 0x0000000055555555;
		pDataArray[iCnt]->lCount = 0;
	}

	for (iCnt = 0; iCnt < iRand; iCnt++)
	{
		g_Mempool.Free(pDataArray[iCnt]);
		InterlockedIncrement64((LONG64 *)&lPushCounter);
	}

	while (1){
		Sleep(0);

		for (int iCnt = 0; iCnt < iRand; iCnt++)
		{
			g_Stack.Pop(&pData);
			InterlockedIncrement64((LONG64 *)&lPopCounter);

			if ((pData->lData != 0x0000000055555555) || (pData->lCount != 0))
				printf("pDataArray[%d] is using in stack\n", iCnt);

			InterlockedIncrement64((LONG64 *)&pData->lCount);
			InterlockedIncrement64((LONG64 *)&pData->lData);

			pDataArray[iCnt] = pData;
		}

		Sleep(0);

		for (int iCnt = 0; iCnt < iRand; iCnt++)
		{
			if ((pDataArray[iCnt]->lCount != 1) || (pDataArray[iCnt]->lData != 0x0000000055555556))
				printf("pDataArray[%d] is using\n", iCnt);
		}

		for (int iCnt = 0; iCnt < iRand; iCnt++)
		{
			InterlockedDecrement64((LONG64 *)&pDataArray[iCnt]->lCount);
			InterlockedDecrement64((LONG64 *)&pDataArray[iCnt]->lData);
		}

		for (int iCnt = 0; iCnt < iRand; iCnt++)
		{
			g_Stack.Push(pDataArray[iCnt]);
			InterlockedIncrement64((LONG64 *)&lPushCounter);
		}
	}
}