// MyWnd.cpp : implementation file
//

#include "stdafx.h"
//#include "ClippingExpress.h"
#include "MyWnd.h"


// CMyWnd

IMPLEMENT_DYNAMIC(CMyWnd, CWnd)

CMyWnd::CMyWnd()
{

}

CMyWnd::~CMyWnd()
{
}


BEGIN_MESSAGE_MAP(CMyWnd, CWnd)
	ON_WM_PAINT()
END_MESSAGE_MAP()



// CMyWnd message handlers
void CMyWnd::OnPaint()
{
	CRect rc;
	GetClientRect(&rc);

	CPaintDC dc(this);
	CMyMemDC md(&dc, &rc);

	OnMyDraw(&md);
}


