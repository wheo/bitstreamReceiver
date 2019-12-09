
// stdafx.h : 자주 사용하지만 자주 변경되지는 않는
// 표준 시스템 포함 파일 및 프로젝트 관련 포함 파일이 
// 들어 있는 포함 파일입니다.

#pragma once

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN            // 거의 사용되지 않는 내용은 Windows 헤더에서 제외합니다.
#endif

#include "targetver.h"

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS      // 일부 CString 생성자는 명시적으로 선언됩니다.

// MFC의 공통 부분과 무시 가능한 경고 메시지에 대한 숨기기를 해제합니다.
#define _AFX_ALL_WARNINGS

#include <afxwin.h>         // MFC 핵심 및 표준 구성 요소입니다.
#include <afxext.h>         // MFC 확장입니다.


#include <afxdisp.h>        // MFC 자동화 클래스입니다.



#ifndef _AFX_NO_OLE_SUPPORT
#include <afxdtctl.h>           // Internet Explorer 4 공용 컨트롤에 대한 MFC 지원입니다.
#endif
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>             // Windows 공용 컨트롤에 대한 MFC 지원입니다.
#endif // _AFX_NO_AFXCMN_SUPPORT

#include <afxcontrolbars.h>     // MFC의 리본 및 컨트롤 막대 지원









#ifdef _UNICODE
#if defined _M_IX86
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif
#endif

#define TNM_CHANNEL_CONNECT		WM_USER + 1
#define TNM_CHANNEL_DISCONNECT	WM_USER + 2
#define TNM_CHANNEL_PLAY		WM_USER + 3
#define TNM_CHANNEL_STOP		WM_USER + 4
#define TNM_CHANNEL_CONTRAST	WM_USER	+ 5
#define TNM_CHANNEL_BRIGHTNESS	WM_USER + 6
#define TNM_CHANNEL_SETURATION	WM_USER + 7
#define TNM_CHANNEL_AUDIO		WM_USER + 8
#define TNM_CHANNEL_CROP		WM_USER + 9
#define TNM_CHANNEL_RESOLUTION	WM_USER + 10
#define WM_TNM_INOUT_THUMB_POS_CHANGED	WM_USER + 11

#include "jthread.h"

#include "TnmUtility.h"

#include "secondThread.h"

#include "MemDC.h"

using namespace jthread;

#include "mmsystem.h"
#include <afxcontrolbars.h>

#include "TnmGlobal.h"
#include <afxcontrolbars.h>

#define ENABLE_HW	0

#define MAX_WIDTH	1920
#define MAX_HEIGHT	1080

#define CHANNEL_WIDTH	338
#define CHANNEL_HEIGHT	150

#define MAX_CHANNEL_COUNT	6

#define MAX_DETECTION_COUNT	50

#define TNM_CODEC_ID_H264	0
#define TNM_CODEC_ID_HEVC	1
#define TNM_CODEC_ID_JPEG	2

#define SAFE_DELETE(x) if (x) {delete x; }; x = NULL;

static TCHAR g_strChannelName[][100] = { _T("Analog1"), _T("Analog2"), _T("Analog3"), _T("Analog4"), _T("Panorama"), _T("Focus") };

enum MT_STATE {
	mtReady, mtRunning, mtTerminated, mtZombie, mtAborted
};
class CTnmThread {
public:
	CTnmThread(void);
	~CTnmThread(void);

	void Start();
	void Terminate();

	bool IsTerminated();
	bool IsRunning();

	void SetState(MT_STATE st) { m_eState = st; };
	HANDLE GetHandle() { return m_hThread; };

protected:

	virtual void Run() = 0;

	static unsigned __stdcall _thread(LPVOID lParam);

protected:

	bool m_bExit; ///> 쓰레드 종료 플래그
	bool m_bPause;

private:

	HANDLE m_hThread; ///> 쓰레드 핸들
	MT_STATE m_eState; ///> 쓰레드 상태 변수
};

struct save_file_meta_s
{
	TCHAR strFileName[200];
	byte enable_ch;
	int nDuration;
};
