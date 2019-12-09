#include "stdafx.h"
#include "secondThread.h"


secondThread::secondThread()
{
	running = 0;
	threadid = 0;
	threadhandle = 0;
}


secondThread::~secondThread()
{
	Kill2();
}

int secondThread::Start2()
{
	threadhandle = (HANDLE)_beginthreadex(NULL, 0, TheThread, this, 0, &threadid);
	if (threadhandle == NULL)
	{
		return -1;
	}

	return 0;
}

int secondThread::Kill2()
{
	if (!running)
	{
		return -1;
	}

	TerminateThread(threadhandle, 0);
	CloseHandle(threadhandle);
	threadhandle = 0;
	running = false;


	return 0;
}

bool secondThread::IsRunning2()
{
	return running;
}

UINT secondThread::TheThread(void * param)
{
	secondThread* pThis;
	void* ret;

	pThis = (secondThread*)param;

	pThis->running = true;

	ret = pThis->Thread2();

	CloseHandle(pThis->threadhandle);

	return 0;
}
