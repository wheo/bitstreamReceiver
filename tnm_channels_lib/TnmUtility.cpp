#include "stdafx.h"

#pragma comment(lib, "swscale.lib")
#pragma comment(lib, "swresample.lib")
#pragma comment(lib, "avfilter.lib")
#pragma comment(lib, "avformat.lib")
#pragma comment(lib, "avcodec.lib")
#pragma comment(lib, "avutil.lib")

int ConsolePrintF(LPCTSTR tszFormat, ...)
{
	int cnt = 0;

	va_list argptr;
	TCHAR buf[1024];

	va_start(argptr, tszFormat);
	cnt = _vsntprintf(buf, sizeof(buf) / sizeof(TCHAR), tszFormat, argptr);
//	cnt = _vsnwprintf(buf, sizeof(buf) / sizeof(TCHAR), tszFormat, argptr);
	if (cnt < 0)	// overflow
	{
		cnt = sizeof(buf) / sizeof(TCHAR) - 1;
		buf[cnt] = 0;
	}

	static HANDLE hConsole = NULL;
	if (!hConsole)
	{
		AllocConsole();
		hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

		// enlarge console buffer
		CONSOLE_SCREEN_BUFFER_INFO csbi;
		GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
		if (csbi.dwSize.X < 120)	 csbi.dwSize.X = 120;
		if (csbi.dwSize.Y < 9999) csbi.dwSize.Y = 9999;
		SetConsoleScreenBufferSize(GetStdHandle(STD_OUTPUT_HANDLE), csbi.dwSize);
	}

	DWORD dwNumberOfCharsWritten;
	WriteConsole(hConsole, buf, cnt, &dwNumberOfCharsWritten, NULL);

	va_end(argptr);

	return cnt;
}

void WideToMulti(wchar_t* strSrc, char* strDst)
{
	/// 유니코드 => 멀티바이트	
	int len = WideCharToMultiByte(CP_ACP, 0, strSrc, -1, NULL, 0, NULL, NULL);
	WideCharToMultiByte(CP_ACP, 0, strSrc, -1, strDst, len, NULL, NULL);
}



CMyWarkingTime::CMyWarkingTime(void)
{

}

CMyWarkingTime::~CMyWarkingTime(void)
{

}
void CMyWarkingTime::SetStart()
{
	m_start = system_clock::now();
}
void CMyWarkingTime::SetEnd()
{
	m_end = system_clock::now();
}

//typedef duration<long long, milli> milliseconds;
int CMyWarkingTime::GetDuration()
{
	milliseconds sec = duration_cast<milliseconds>(m_end - m_start);

	long long lTemp = sec.count();
	int nTemp = (int)lTemp;

	return nTemp;
}

CVideoThread::CVideoThread(void) {
	m_bExit = false;
	m_hThread = INVALID_HANDLE_VALUE;

	m_eState = mtReady;
}

CVideoThread::~CVideoThread(void) {
	Terminate();
}

void CVideoThread::SetThreadMAXPriority()
{
	SetThreadPriority(m_hThread, THREAD_PRIORITY_TIME_CRITICAL);
}

void CVideoThread::Start() {
	UINT uiThreadID;

	m_bExit = false;
	m_hThread = (HANDLE)_beginthreadex(NULL, 0, _thread, (void*)this, 0, &uiThreadID);
	//> 170328 - 쓰레드 우선순위 상향
	SetThreadPriority(m_hThread, THREAD_PRIORITY_HIGHEST);
}

void CVideoThread::Terminate() {
	m_bExit = true;
	if (m_hThread != INVALID_HANDLE_VALUE) {
		WaitForSingleObject(m_hThread, INFINITE);
	}
}

UINT CVideoThread::_thread(LPVOID lParam) {
	CVideoThread *pthis = reinterpret_cast<CVideoThread*>(lParam);

	pthis->SetState(mtRunning);
	pthis->Run();
	pthis->SetState(mtTerminated);

	return 0;
}

bool CVideoThread::IsTerminated() {
	if (m_eState == mtTerminated) {
		return true;
	}

	return false;
}

bool CVideoThread::IsRunning() {
	if (m_eState == mtRunning) {
		return true;
	}

	return false;
}

CVideoDoubleThread::CVideoDoubleThread(void) {
	m_bExit = false;
	m_hThread[0] = m_hThread[1] = INVALID_HANDLE_VALUE;
}

CVideoDoubleThread::~CVideoDoubleThread(void) {
	Terminate();
}

void CVideoDoubleThread::Start() {
	UINT uiThreadID;

	m_bExit = false;

	m_hThread[0] = (HANDLE)_beginthreadex(NULL, 0, _thread1, (void*)this, 0, &uiThreadID);
	m_hThread[1] = (HANDLE)_beginthreadex(NULL, 0, _thread2, (void*)this, 0, &uiThreadID);
}

void CVideoDoubleThread::Terminate() {
	m_bExit = true;
	if (m_hThread[0] != INVALID_HANDLE_VALUE && m_hThread[1] != INVALID_HANDLE_VALUE) {
		WaitForMultipleObjects(2, m_hThread, true, INFINITE);
	}
}

UINT CVideoDoubleThread::_thread1(LPVOID lParam) {
	CVideoDoubleThread *pthis = reinterpret_cast<CVideoDoubleThread*>(lParam);

	pthis->Run1();
	return 0;
}

UINT CVideoDoubleThread::_thread2(LPVOID lParam) {
	CVideoDoubleThread *pthis = reinterpret_cast<CVideoDoubleThread*>(lParam);

	pthis->Run2();
	return 0;
}

CMyRing::CMyRing(int nMaxSize, int nChannel)
{
	m_pBase = new byte[nMaxSize];

	m_nRP = 0;
	m_nWP = 0;

	m_nWritten = 0;
	m_nRemain = nMaxSize;
	m_nTotal = nMaxSize;

	m_bExit = false;
	m_nChannel = nChannel;
}

CMyRing::~CMyRing()
{
	if (m_pBase) {
		delete[] m_pBase;
	}
}

int CMyRing::Read(uint8_t *buf, int buf_size, bool bBlock)
{
	while (1) {
		m_cs.Lock();
		if (m_nWritten >= buf_size) {
			m_cs.Unlock();
			break;
		}
		m_cs.Unlock();

		if (bBlock == false) {
			return -1;
		}

		Sleep(2);	// 읽을 크기보다 적을 경우 대기
		if (m_bExit) {
			return 0;
		}
	}

	int nS1, nS2;
	int nRP = m_nRP, nWP = m_nWP;

	if (nRP >= nWP) {
		// 0=======W------R=======m_nTotal 의 경우(1)
		nS1 = m_nTotal - nRP;
		nS2 = nWP;
	}
	else {
		// 0-------R======W-------m_nTotal 의 경우(2)
		nS1 = nWP - nRP;
		nS2 = 0;
	}

	if (nS1 >= buf_size) {
		// 한쪽만으로도 충분한 데이터가 있는 경우(1,2)
		memcpy(buf, &m_pBase[nRP], buf_size);

		nRP += buf_size;
		if (nRP >= m_nTotal) {
			if (nRP > m_nTotal) {
				_d(_T("Ring-Read read error msg\n"));
			}
			nRP = 0;
		}
	}
	else {
		// 양쪽을 다 써야 하는 경우(1)
		//	_d("Ring-Read wrap case occur(written %d, read %d, size1 %d)\n", m_nWritten, buf_size, nS1);

		memcpy(buf, &m_pBase[nRP], nS1);
		memcpy(&buf[nS1], &m_pBase[0], buf_size - nS1);

		nRP = buf_size - nS1;
	}

	// 정보 업데이트
	// !!! 주의 !!! m_nRemain은 적절한 동기화를 위해 제일 나중에 바뀌어야 함
	m_cs.Lock();
	m_nRP = nRP;
	m_nWritten -= buf_size;
	m_nRemain += buf_size;
	m_cs.Unlock();

	if (m_nChannel == 1) {
		if (m_nWP == m_nRP) {
			_d(_T("Ring-Read error remain: %d, write: %d\n"), m_nRemain, m_nWritten);
		}
	}

	return buf_size;
}

int CMyRing::Write(uint8_t *buf, int buf_size)
{
	while (1) {
		m_cs.Lock();
		if (m_nRemain >= buf_size) {
			m_cs.Unlock();
			break;
		}
		m_cs.Unlock();

		Sleep(2);	// 남은 크기가 부족할 경우 대기
		if (m_bExit) {
			return 0;
		}

		_d(_T("[CMyRing] 남은 크기가 부족할 경우 대기\n"));
	}

	int nSize1, nSize2;
	int nRP = m_nRP, nWP = m_nWP;

	if (nWP >= nRP) {
		// ------R=======W------- 의 경우(1)
		nSize1 = m_nTotal - nWP;
		nSize2 = nRP;
	}
	else {
		// ======W-------R======= 의 경우(2)
		nSize1 = nRP - nWP;
		nSize2 = 0;
	}

	if (nSize1 >= buf_size) {
		// 한쪽만으로도 충분(1,2)
		memcpy(&m_pBase[nWP], buf, buf_size);

		nWP += buf_size;
		if (nWP >= m_nTotal) {
			if (nWP > m_nTotal) {
				_d(_T("[CMyRing] Write error \n"));
			}
			nWP = 0;
		}
	}
	else {
		// 양쪽이 다 필요(2)
		//	_d("Ring-Write wrap case occur(remain %d, write %d, size1 %d)\n", m_nRemain, buf_size, nSize1);

		memcpy(&m_pBase[nWP], buf, nSize1);
		memcpy(&m_pBase[0], &buf[nSize1], buf_size - nSize1);

		nWP = buf_size - nSize1;
	}

	// 업데이트
	// !!! 주의 !!! m_nWritten은 적절한 동기화를 위해 제일 나중에 바뀌어야 함
	m_cs.Lock();
	m_nWP = nWP;
	m_nRemain -= buf_size;
	m_nWritten += buf_size;
	m_cs.Unlock();

	if (m_nChannel == 1) {
		if (m_nWP == m_nRP) {
			_d(_T("[CMyRing] Write error. remain: %d, write: %d\n"), m_nRemain, m_nWritten);
		}
	}
	return buf_size;
}