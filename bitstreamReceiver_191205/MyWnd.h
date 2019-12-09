#pragma once
// CMyWnd

class CMyWnd : public CWnd
{
	DECLARE_DYNAMIC(CMyWnd)

public:
	CMyWnd();
	virtual ~CMyWnd();

	virtual void OnMyDraw(CMyMemDC *pDC) = 0;

protected:
	afx_msg void OnPaint();
	DECLARE_MESSAGE_MAP()
};


