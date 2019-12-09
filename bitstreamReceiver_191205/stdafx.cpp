
// stdafx.cpp : 표준 포함 파일만 들어 있는 소스 파일입니다.
// bitstreamReceiver.pch는 미리 컴파일된 헤더가 됩니다.
// stdafx.obj에는 미리 컴파일된 형식 정보가 포함됩니다.

#include "stdafx.h"


#pragma comment(lib, "winmm.lib")

CTnmThread::CTnmThread(void) {
	m_bExit = false;
	m_hThread = INVALID_HANDLE_VALUE;

	m_eState = mtReady;
}

CTnmThread::~CTnmThread(void) {
	Terminate();
}

void CTnmThread::Start() {
	UINT uiThreadID;

	m_bExit = false;
	m_hThread = (HANDLE)_beginthreadex(NULL, 0, _thread, (void*)this, 0, &uiThreadID);
	//> 170328 - 쓰레드 우선순위 상향
	SetThreadPriority(m_hThread, THREAD_PRIORITY_HIGHEST);
}

void CTnmThread::Terminate() {
	m_bExit = true;
	if (m_hThread != INVALID_HANDLE_VALUE) {
		WaitForSingleObject(m_hThread, INFINITE);
	}
}

UINT CTnmThread::_thread(LPVOID lParam) {
	CTnmThread *pthis = reinterpret_cast<CTnmThread*>(lParam);

	pthis->SetState(mtRunning);
	pthis->Run();
	pthis->SetState(mtTerminated);

	return 0;
}

bool CTnmThread::IsTerminated() {
	if (m_eState == mtTerminated) {
		return true;
	}

	return false;
}

bool CTnmThread::IsRunning() {
	if (m_eState == mtRunning) {
		return true;
	}

	return false;
}