
// bitstreamReceiver.h : PROJECT_NAME ���� ���α׷��� ���� �� ��� �����Դϴ�.
//

#pragma once

#ifndef __AFXWIN_H__
	#error "PCH�� ���� �� ������ �����ϱ� ���� 'stdafx.h'�� �����մϴ�."
#endif

#include "resource.h"		// �� ��ȣ�Դϴ�.


// CbitstreamReceiverApp:
// �� Ŭ������ ������ ���ؼ��� bitstreamReceiver.cpp�� �����Ͻʽÿ�.
//

class CbitstreamReceiverApp : public CWinApp
{
public:
	CbitstreamReceiverApp();

// �������Դϴ�.
public:
	virtual BOOL InitInstance();

// �����Դϴ�.

	DECLARE_MESSAGE_MAP()
};

extern CbitstreamReceiverApp theApp;