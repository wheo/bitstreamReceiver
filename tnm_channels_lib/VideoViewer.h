#pragma once
class CVideoViewer : public CWnd
{
public:
	CVideoViewer();
	~CVideoViewer();

	bool CreateViewer(const RECT &rect, CWnd *pParentWnd, UINT nID);
	void DeleteViewer();
	void ResizeViewer();
	
	void SetTargetInfo(int nChannel, detection_Info_s *pDetectInfo);

	void set_cur_connected_ip(CString strIP);
	void set_recv_frame_count(int nFPS);

	void ReDraw();
	void DrawYUV420(int nSrcWidth, int nSrcHeight, uint8_t* pData);
	void ImageDraw(int nWidth, int nHeight, byte *pData);

	void get_cur_mouse_pos(int &nx, int &ny);

	DECLARE_MESSAGE_MAP()
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);

private:
	bool CreateViewer();
	void Draw();

	void CreateD2D();
	void DeleteD2D();

	void DrawString(CString strData, CRect *rc, IDWriteTextFormat *pText, ID2D1SolidColorBrush *pBrush);
protected:
	CRect m_rcViewer;

	char* m_pRGB32;
	
	SwsContext	*m_pSwsCtx;

	AVPixelFormat m_fmtSrc;

	//CCriticalSection m_csDraw;

	double m_fWidthRatio;
	double m_fHeightRatio;
	
	int m_nSrcWidth;
	int m_nSrcHeight;

	int m_cur_mouse_x;
	int m_cur_mouse_y;

	//> overlay
	detection_Info_s m_detect_info;
	CString m_str_connected_ip;
	int m_recv_frame_count;

	/* 2d2 */	
	ID2D1HwndRenderTarget *m_ipRenderTarget;
	ID2D1Bitmap *m_pBmpImage;

	IDWriteTextFormat*		m_pD2DText;
	ID2D1SolidColorBrush*	m_pSolidBrushB;
	ID2D1SolidColorBrush*	m_pSolidBrushR;

	ID2D1SolidColorBrush	*m_pBrushRect;

	bool m_bExpand;
	bool m_bMouseClicked;	//> 드래그 표시를 위해 클릭상태여부 체크	
	CPoint m_ptClickDown;
	CPoint m_ptClickUp;
	
};

