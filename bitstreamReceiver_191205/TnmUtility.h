#pragma once
#include <chrono>
using namespace std;
using namespace chrono;

enum
{
	BLACK = 0,
	DARK_BLUE,
	DARK_GREEN,
	DARK_SKY_BLUE,
	DARK_RED,
	DARK_VIOLET,
	DARK_YELLO,
	GRAY,
	DARK_GRAY,
	BLUE,
	GREEN,
	SKY_BLUE,
	RED,
	VIOLET,
	YELLOW,
	WHITE
};

int ConsolePrintF(LPCTSTR tszFormat, ...);

#define _d(...)	ConsolePrintF(__VA_ARGS__)
#define _dy(...)	{ SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), YELLOW); ConsolePrintF(__VA_ARGS__);  SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), GRAY); }
#define _dr(...)	{ SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), RED); ConsolePrintF(__VA_ARGS__);  /*SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), GRAY);*/ }
#define _dv(...)	{ SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), VIOLET); ConsolePrintF(__VA_ARGS__);  SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), GRAY); }
#define _db(...)	{ SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), DARK_SKY_BLUE); ConsolePrintF(__VA_ARGS__);  SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), GRAY); }
#define _dg(...)	{ SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), GREEN); ConsolePrintF(__VA_ARGS__);  SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), GRAY); }
#define _dw(...) { SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), WHITE); ConsolePrintF(__VA_ARGS__);  SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), GRAY); }

inline bool TnmQueryPerformanceCounter(unsigned long long *counter)
{
	*counter = 0;
	LARGE_INTEGER lcounter;
	if (!QueryPerformanceCounter(&lcounter)) {
		return false;
	}
	*counter = lcounter.QuadPart;
	return true;
}

inline bool NvQueryPerformanceFrequency(unsigned long long *freq)
{
	*freq = 0;
	LARGE_INTEGER lfreq;
	if (!QueryPerformanceFrequency(&lfreq)) {
		return false;
	}
	*freq = lfreq.QuadPart;
	return true;
}

extern void WideToMulti(wchar_t* strSrc, char* strDst);


class CMyWarkingTime
{
public:
	CMyWarkingTime(void);
	~CMyWarkingTime(void);

	void SetStart();
	void SetEnd();
	int GetDuration();

protected:
	system_clock::time_point m_start;
	system_clock::time_point m_end;
};