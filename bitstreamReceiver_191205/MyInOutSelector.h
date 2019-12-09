#pragma once

#include "MyWnd.h"
// CMyInOutSelector

class CMyInOutSelector : public CMyWnd
{
	DECLARE_DYNAMIC(CMyInOutSelector)

public:
	CMyInOutSelector();
	virtual ~CMyInOutSelector();

	void OnMyDraw(CMyMemDC *pDC);

	void Clear() {
		m_nPos = 0;
		m_nMax = 0;
		m_nRatio = 0;

		m_rcThumb.left = 0;
		m_rcThumb.right = 9;

		ClearInOut();
		ClearSelection();
	};

	void ClearInOut() {
		m_nInPos = -1;
		m_nOutPos = -1;

		m_flagInOut = 0;

		Invalidate();
	}

	void Create(const RECT &rect, CWnd *pParentWnd, UINT nID, CCreateContext *pContext = NULL);

	/** SetRange
	@desc : ���ð����� ������ �����Ѵ�.
	@param : nMax - �ִ� ��Ʈ�� ������ ���� */
	void SetRange(int nMax);

	int GetRange() { return m_nMax; };

	/** SetRatio
	@desc : �ִ� ��Ʈ�� �������� ��ü���� ����������� ����
	@param : nRatio - ����, ��) 50% -> nMax�� ���� 10�̸� ��ü�� 20�̶� ���� ����
			 ���� 100%�̸� �ִ�=max */
	void SetRatio(int nRatio);

	void SetPos(int nPos) {
		CRect rc, rcOld, rcInner;
		GetClientRect(&rc);

		m_nPos = nPos;

		if (m_nMax <= 0) {
			return;
		}

		rcInner = rc;
		rcInner.DeflateRect(4, 2, 4, 1);
		
		int nMaxThumb = (rcInner.Width()*m_nRatio)/100;
		int nThumbPos = 4 + (nMaxThumb*m_nPos)/m_nMax;
		
		rcOld = m_rcThumb;

		m_rcThumb.left = nThumbPos - 4;
		m_rcThumb.right = m_rcThumb.left + 9;
		m_rcThumb.top = 0;
		m_rcThumb.bottom = 18;

		InvalidateRect(&rcOld);
		InvalidateRect(&m_rcThumb);
	};

	int GetPos() {
		CRect rc, rcInner;
		GetClientRect(&rc);

		rcInner = rc;
		rcInner.DeflateRect(4, 2, 4, 1);
		
		int nMaxThumb = (rcInner.Width()*m_nRatio)/100;
		if (nMaxThumb) {
			m_nPos = (m_rcThumb.left*m_nMax)/nMaxThumb;
			return m_nPos;
		}

		return 0;
	};

	void SetIn();
	void SetOut();

	void SetIn(int nPos);
	void SetOut(int nPos);

	bool GetInOut(int &nIn, int &nOut) {
		if (m_flagInOut & 0x1) {
			nIn = m_nInPos;
		} else {
			nIn = -1;
		}
		if (m_flagInOut & 0x2) {
			nOut = m_nOutPos;
		} else {
			nOut = -1;
		}
		if (nIn >= 0 && nOut >= 0) {
			return true;
		}
		return false;
	}

	void SetSelection(int nIn, int nOut) {
		m_nInSel = nIn;
		m_nOutSel = nOut;
		Invalidate();
	};
	void ClearSelection() {
		m_nInSel = -1;
		m_nOutSel = -1;
		Invalidate();
	};

protected:
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnMouseLeave();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	DECLARE_MESSAGE_MAP()

	int m_nMax;
	int m_nRatio;

	int m_nPos;

	int m_nInPos;
	int m_nOutPos;

	int m_nInSel;
	int m_nOutSel;
	
	byte m_flagInOut;

	bool m_bIsOver;
	bool m_bIsTracking;

	int m_nClickPos;

	HBITMAP m_hBmp;

	CRect m_rcThumb;
	CImageList m_imgList;

public:
	
};


