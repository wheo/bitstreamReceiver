
// stdafx.h : ���� ��������� ���� ��������� �ʴ�
// ǥ�� �ý��� ���� ���� �� ������Ʈ ���� ���� ������ 
// ��� �ִ� ���� �����Դϴ�.

#pragma once

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN            // ���� ������ �ʴ� ������ Windows ������� �����մϴ�.
#endif

#include "targetver.h"

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS      // �Ϻ� CString �����ڴ� ��������� ����˴ϴ�.

// MFC�� ���� �κа� ���� ������ ��� �޽����� ���� ����⸦ �����մϴ�.
#define _AFX_ALL_WARNINGS

#include <afxwin.h>         // MFC �ٽ� �� ǥ�� ���� ����Դϴ�.
#include <afxext.h>         // MFC Ȯ���Դϴ�.


#include <afxdisp.h>        // MFC �ڵ�ȭ Ŭ�����Դϴ�.



#ifndef _AFX_NO_OLE_SUPPORT
#include <afxdtctl.h>           // Internet Explorer 4 ���� ��Ʈ�ѿ� ���� MFC �����Դϴ�.
#endif
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>             // Windows ���� ��Ʈ�ѿ� ���� MFC �����Դϴ�.
#endif // _AFX_NO_AFXCMN_SUPPORT

#include <afxcontrolbars.h>     // MFC�� ���� �� ��Ʈ�� ���� ����









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

	bool m_bExit; ///> ������ ���� �÷���
	bool m_bPause;

private:

	HANDLE m_hThread; ///> ������ �ڵ�
	MT_STATE m_eState; ///> ������ ���� ����
};

struct save_file_meta_s
{
	TCHAR strFileName[200];
	byte enable_ch;
	int nDuration;
};
