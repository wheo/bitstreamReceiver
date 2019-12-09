#pragma once
#include <chrono>
#include <afxmt.h>

/** FFMPEG */
extern "C"
{
#include "libswscale\swscale.h"
#include "libswresample\swresample.h"
#include "libavfilter\avfilter.h"
#include "libavcodec\avcodec.h"
#include "libavformat\avformat.h"
#include "libavutil\avutil.h"
#include "libavutil/opt.h"
#include "libavfilter/buffersink.h"
#include "libavfilter/buffersrc.h"
	//#include "libavutil/random_seed.h"
}

using namespace std;
using namespace chrono;

#define SAFE_DELETE(x) if (x) {delete x; }; x = NULL;

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


/** Thread class */
enum MT_STATE {
	mtReady, mtRunning, mtTerminated, mtZombie, mtAborted
};

class CVideoThread {
public:
	CVideoThread(void);
	~CVideoThread(void);

	void Start();
	void Terminate();

	bool IsTerminated();
	bool IsRunning();

	void SetState(MT_STATE st) { m_eState = st; };
	HANDLE GetHandle() { return m_hThread; };

	void SetThreadMAXPriority();
protected:

	virtual void Run() = 0;

	static unsigned __stdcall _thread(LPVOID lParam);

protected:

	bool m_bExit; ///> ������ ���� �÷���
	bool m_bPause;

private:

	HANDLE m_hThread; ///> ������ �ڵ�
	MT_STATE m_eState; ///> ������ ���� ����

};

class CVideoDoubleThread {
public:
	CVideoDoubleThread(void);
	~CVideoDoubleThread(void);

	void Start();
	void Terminate();

protected:

	virtual void Run1() = 0;
	virtual void Run2() = 0;

	static unsigned __stdcall _thread1(LPVOID lParam);
	static unsigned __stdcall _thread2(LPVOID lParam);

protected:

	bool m_bExit;

	HANDLE m_hThread[2];
};

/** ���� ������ RingBuffer Ŭ���� */
class CMyRing
{
public:
	/**������
	@param nMaxSize : �ִ� ������ ������*/
	CMyRing::CMyRing(int nMaxSize, int nChannel = 0);
	CMyRing::~CMyRing();

	/**����带 �����Ѵ�.(�����忡�� ������ ���Ḧ ����) */
	void SetExit() { m_bExit = true; };

	/**�����ۿ� �ִ´�
	@param buf : ���� �����Ͱ� ����ִ� ���� ������
	@param buf_size : ���� �������� ������
	@return ���� �� ũ��*/
	int Write(uint8_t *buf, int buf_size);

	/**�����ۿ��� �д´�.
	@param buf : �о�� ���� ������
	@param buf_size : �о�� ������
	@return ���� ���� ũ��*/
	int Read(uint8_t *buf, int buf_size, bool bBlock = true);
protected:
	int		m_nChannel; ////> ä�ι�ȣ

	byte	*m_pBase;	//> �������� ���� �ּ�

	bool	m_bExit;	//> �����÷���(���������)

	int		m_nRP;		//> Read Position
	int		m_nWP;		//> Write Position

	int		m_nRemain;	//> �������� ���� ũ��
	int		m_nWritten;	//> �����ۿ� �� ũ��
	int		m_nTotal;	//> ��ü ũ��

	CCriticalSection m_cs; ///> ũ��Ƽ�ü���
};