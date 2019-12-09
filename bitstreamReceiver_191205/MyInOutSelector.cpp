// MyInOutSelector.cpp : implementation file
//

#include "stdafx.h"
#include "bitstreamReceiver.h"
#include "MyInOutSelector.h"


// CMyInOutSelector

IMPLEMENT_DYNAMIC(CMyInOutSelector, CMyWnd)

CMyInOutSelector::CMyInOutSelector()
{
	m_nMax = 0;
	m_nRatio = 0;

	m_nPos = 0;

	m_nInPos = -1;
	m_nOutPos = -1;

	m_flagInOut = 0;

	m_rcThumb.left = 0;
	m_rcThumb.right = 0;

	m_bIsOver = false;
	m_bIsTracking = false;

	m_rcThumb.left = 0;
	m_rcThumb.right = m_rcThumb.left + 9;
	m_rcThumb.top = 0;
	m_rcThumb.bottom = 18;

	m_nInSel = -1;
	m_nOutSel = -1;
}

CMyInOutSelector::~CMyInOutSelector()
{
}


BEGIN_MESSAGE_MAP(CMyInOutSelector, CMyWnd)
	ON_WM_MOUSEMOVE()
	ON_WM_MOUSELEAVE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
END_MESSAGE_MAP()



// CMyInOutSelector message handlers
void CMyInOutSelector::Create(const RECT &rect, CWnd *pParentWnd, UINT nID, CCreateContext *pContext) {

	DWORD dwStyle = WS_CHILD |WS_VISIBLE |PBS_SMOOTH;

	BITMAP bmp;
	m_hBmp = LoadBitmap(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDB_BTN_INOUT_THUMB));
	
	GetObject(m_hBmp, sizeof(BITMAP), &bmp);

	m_imgList.Create(bmp.bmWidth/3, bmp.bmHeight, ILC_COLOR24 | ILC_MASK, 0, 1);
	m_imgList.SetBkColor(RGB(0,0,255));
	m_imgList.Add(CBitmap::FromHandle(m_hBmp), RGB(0, 0, 255));

	CMyWnd::Create(NULL, _T(""), dwStyle, rect, pParentWnd, nID, pContext);
}

void CMyInOutSelector::OnMyDraw(CMyMemDC *pDC)
{
	CRect rc, rcInner;
	GetClientRect(&rc);

	CBrush brFrame(RGB(151, 151, 151));

	pDC->FillSolidRect(&rc, RGB(60, 58, 56));

	rcInner = rc;
	rcInner.DeflateRect(4, 2, 4, 1);

	/** 내부 */
	if (m_nRatio) {
		pDC->FillSolidRect(&rcInner, RGB(57, 57, 57));
		if (m_nRatio != 100) {
			rcInner.bottom = 6;

			int nLeft = (rcInner.Width()*m_nRatio)/100;
			int nRight = rcInner.Width() - nLeft + 1;

			rcInner.right = nLeft;
			pDC->FillSolidRect(&rcInner, RGB(220, 220, 20));
		}
	} else {
		pDC->FillSolidRect(&rcInner, RGB(40, 40, 40));
	}
	
	/** 테두리 */
	rcInner = rc;
	rcInner.DeflateRect(3, 1, 3, 0);

	pDC->FrameRect(&rcInner, &brFrame);


	if (m_nInSel >= 0 && m_nOutSel >= 0) {
		CRect rcInOut;

		rcInner = rc;
		rcInner.DeflateRect(4, 2, 4, 1);

		rcInOut = rcInner;

		if (!(m_flagInOut & 0x2)) {
			m_nOutPos = m_nMax;
		}

		int nMaxThumb = (rcInner.Width()*m_nRatio)/100;
		rcInOut.left = 4 + (nMaxThumb*m_nInSel)/m_nMax;
		rcInOut.right = 4 + (nMaxThumb*m_nOutSel)/m_nMax;

		pDC->FillSolidRect(&rcInOut, RGB(253, 77, 72));
	}
	
	if (m_nInPos >= 0 && m_nOutPos >= 0) {
		CRect rcInOut;

		rcInner = rc;
		rcInner.DeflateRect(4, 2, 4, 1);

		rcInOut = rcInner;

		if (!(m_flagInOut & 0x2)) {
			m_nOutPos = m_nMax;
		}

		int nMaxThumb = (rcInner.Width()*m_nRatio)/100;
		rcInOut.left = 4 + (nMaxThumb*m_nInPos)/m_nMax;
		rcInOut.right = 4 + (nMaxThumb*m_nOutPos)/m_nMax;

		pDC->FillSolidRect(&rcInOut, RGB(242,112,34));
	}
	
	if (m_nMax) {
		int nIndex = m_bIsOver ? 1 : 0;
		if (m_bIsTracking) {
			nIndex = 2;
		}
		// 좀더 부드럽게 움직일 수 있는 방법을 찾아볼 것
		m_imgList.Draw(pDC, nIndex, CPoint(m_rcThumb.left, 0), ILD_TRANSPARENT); 
	}
}

void CMyInOutSelector::SetRange(int nMax)
{
	m_nMax = nMax;
}

void CMyInOutSelector::SetRatio(int nRatio)
{
	CRect rc, rcInner;
	GetClientRect(&rc);

	rcInner = rc;
	rcInner.DeflateRect(1, 1, 1, 1);

	if (nRatio != m_nRatio) {
		if (m_nRatio == 0 || nRatio == 100) {
			m_nRatio = nRatio;
			Invalidate();
		} else {
			int nOld = (rcInner.Width()*m_nRatio)/100;
			m_nRatio = nRatio;
			int nNew = (rcInner.Width()*m_nRatio)/100;

			rcInner.left = nOld-4;
			rcInner.right = nNew+4;

			InvalidateRect(&rcInner);
		}
	}
}

void CMyInOutSelector::OnMouseMove(UINT nFlags, CPoint point)
{
	TRACKMOUSEEVENT tme;

	tme.cbSize = sizeof(tme);
	tme.hwndTrack = m_hWnd;
	tme.dwFlags = TME_LEAVE|TME_HOVER;
	tme.dwHoverTime = 1;
	_TrackMouseEvent(&tme);

	if (m_bIsTracking) {
		CRect rc, rcInner;
		GetClientRect(&rc);

		rcInner = rc;
		rcInner.DeflateRect(4, 2, 4, 1);

		int nMaxThumb = (rcInner.Width()*m_nRatio)/100;

		int x = point.x;
		int left = x;
		if (left < rcInner.left) {
			left = rcInner.left;
		}
		if (left >= rcInner.right) {
			left = rcInner.right-1;
		}

		CRect rcOld = m_rcThumb;

		m_rcThumb.left = left - 4;
		m_rcThumb.right = m_rcThumb.left + 9;

		if (rcOld != m_rcThumb) {
			InvalidateRect(&rcOld);
			InvalidateRect(&m_rcThumb);

			GetParent()->SendMessage(WM_TNM_INOUT_THUMB_POS_CHANGED, NULL, NULL);
		}
		
	} else {
		if (m_rcThumb.PtInRect(point)) {
			if (!m_bIsOver) {
				m_bIsOver = true;

				InvalidateRect(&m_rcThumb);
			}
		} else {
			if (m_bIsOver) {
				m_bIsOver = false;

				InvalidateRect(&m_rcThumb);
			}
		}
	}

	CMyWnd::OnMouseMove(nFlags, point);
}


void CMyInOutSelector::OnMouseLeave()
{
	if (m_bIsOver) {
		m_bIsOver = false;

		InvalidateRect(&m_rcThumb);
	}

	CMyWnd::OnMouseLeave();
}


void CMyInOutSelector::OnLButtonDown(UINT nFlags, CPoint point)
{
	if (m_bIsOver) {
		if (m_bIsTracking == false) {
			m_bIsTracking = true;

			m_nClickPos = point.x - m_rcThumb.left;
			SetCapture();

			Invalidate();
		}
	}
	CMyWnd::OnLButtonDown(nFlags, point);
}


void CMyInOutSelector::OnLButtonUp(UINT nFlags, CPoint point)
{
	m_bIsOver = false;
	if (m_bIsTracking) {
		m_bIsTracking = false;
		ReleaseCapture();
	} else {
		CRect rc, rcInner;
		GetClientRect(&rc);

		rcInner = rc;
		rcInner.DeflateRect(4, 2, 4, 1);

		int nMaxThumb = (rcInner.Width()*m_nRatio)/100;

		int x = point.x;
		int left = x;
		if (left < rcInner.left) {
			left = rcInner.left;
		}
		if (left >= rcInner.right) {
			left = rcInner.right-1;
		}

		CRect rcOld = m_rcThumb;

		m_rcThumb.left = left - 4;
		m_rcThumb.right = m_rcThumb.left + 9;

		if (rcOld != m_rcThumb) {
			InvalidateRect(&rcOld);
			InvalidateRect(&m_rcThumb);

			GetParent()->SendMessage(WM_TNM_INOUT_THUMB_POS_CHANGED, NULL, NULL);
		}
	}

	if (m_rcThumb.PtInRect(point)) {
		m_bIsOver = true;
	} else {
		m_bIsOver = false;
	}
	
	Invalidate();

	CMyWnd::OnLButtonUp(nFlags, point);
}

void CMyInOutSelector::SetIn(int nPos)
{
	if (m_nOutPos >= 0) {
		if (m_nOutPos < nPos) {
			m_nInPos = m_nOutPos;
			m_nOutPos = nPos;

			m_flagInOut |= 0x3;
		} else {
			m_nInPos = nPos;

			m_flagInOut |= 0x1;
		}
	} else {
		m_nInPos = nPos;
		m_nOutPos = m_nMax;

		//> 170214 - 하나만 설정해도 화면에 표시는 되는데 좌표 설정이 안된다. 처리
	//	m_flagInOut |= 0x1;
		m_flagInOut |= 0x3;
	}

	Invalidate();
}

void CMyInOutSelector::SetOut(int nPos)
{
	if (m_nInPos >= 0) {
		if (m_nInPos > nPos) {
			m_nOutPos = m_nInPos;
			m_nInPos = nPos;

			m_flagInOut |= 0x3;
		} else {
			m_nOutPos = nPos;

			m_flagInOut |= 0x2;
		}
	} else {
		m_nInPos = 0;
		m_nOutPos = nPos;

		//> 170214 - 하나만 설정해도 화면에 표시는 되는데 좌표 설정이 안된다. 처리
	//	m_flagInOut |= 0x2;
		m_flagInOut |= 0x3;
	}

	Invalidate();
}

void CMyInOutSelector::SetIn()
{
	SetIn(m_nPos);	
}

void CMyInOutSelector::SetOut()
{
	SetOut(m_nPos);
}

