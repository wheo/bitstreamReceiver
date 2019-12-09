#include "stdafx.h"

int ConsolePrintF(LPCTSTR tszFormat, ...)
{
	int cnt = 0;

	va_list argptr;
	TCHAR buf[1024];

	va_start(argptr, tszFormat);
	cnt = _vsntprintf(buf, sizeof(buf) / sizeof(TCHAR), tszFormat, argptr);
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