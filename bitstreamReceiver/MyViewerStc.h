#pragma once
typedef struct tagOverlay
{
	//int nClickChannel;
	//bool bIsShowRecvInfo;
	//bool bIsShowDetectionInfo;

	//bool bIsChSave[MAX_CHANNEL];
	//SightStatus	SSI[MAX_ROBOT];
	//WeaponStatus WSI[MAX_ROBOT];
	//int nLRF[MAX_ROBOT];
	//traceInfo TraceInfo[MAX_ROBOT];
	//int nCamZOOM[MAX_ROBOT][2];		//0: IR, 1: CCD;
} overlay_info_s;

class CMyViewerStc : public CWnd
{
public:
	CMyViewerStc();
	~CMyViewerStc();

	void CreateViewer(const RECT &rect, CWnd *pParentWnd, UINT nID);
	void DeleteViewer();
	void ResizeViewer();

	void DrawVideoFrame(AVFrame *pFrame);
	void Draw();

	void DrawData(int nSrcWidth, int nSrcHeight, uint8_t* pData);

	void DrawYUV420(int nSrcWidth, int nSrcHeight, uint8_t* pData);
	void DrawGRAY8(int nSrcWidth, int nSrcHeight, uint8_t* pData);

	//	virtual void DrawOverlay(ID2D1HwndRenderTarget *pTarget) = 0;	

	//void ShowDetectionInfo(bool bOverlay);
	void SetTargetInfo(int nChannel, detection_Info_s *pDetectInfo);
	
protected:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);	

protected:
	void CreateRenderer();
	void DeleteRenderer();

	void DrawString(CString strData, CRect *rc, IDWriteTextFormat *pText, ID2D1SolidColorBrush *pBrush);
	void DrawDetectionInfo();
	
protected:
	int m_nIndex;

	char* m_pRGB32;
	int m_nViewerHeight;
	int m_nViewerWidth;
	CRect m_rcViewer;

public:
	
private:
	ID2D1HwndRenderTarget *m_ipRenderTarget;
	ID2D1Bitmap *m_pBmpImage;
	IDWriteTextFormat*		m_pD2DText;

	ID2D1SolidColorBrush*	m_pSolidBrushB;
	ID2D1SolidColorBrush*	m_pSolidBrushR;	

	SwsContext	*m_pSwsCtx;

	AVPixelFormat m_fmtSrc;
	
	int m_nSrcWidth;
	int m_nSrcHeight;

	bool m_bMouseClicked;	//> 드래그 표시를 위해 클릭상태여부 체크	
	CPoint m_ptClickDown;
	CPoint m_ptClickUp;

	ID2D1SolidColorBrush	*m_pBrushRect;	
	bool m_bExpand;	

	
	CRenderTarget* m_pRenderTarget;

	CCriticalSection m_csDraw;


	detection_Info_s m_detect_info;
	double m_fWriteRatio;
	double m_fHeightRatio;
public:
	
};

