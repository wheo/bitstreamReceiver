#include "stdafx.h"
#include "MyViewerStc.h"

#define BACKGROUND_COLOR            RGB(200, 200, 200)

CMyViewerStc::CMyViewerStc()
{
	m_ipRenderTarget = NULL;
	m_pBmpImage = NULL;
	m_pSwsCtx = NULL;
	m_pBrushRect = NULL;
	
	m_pRGB32 = NULL;

	m_bMouseClicked = false;
	m_bExpand = false;	
		
	m_pRenderTarget = NULL;

	m_pSolidBrushB = NULL;
	m_pSolidBrushR = NULL;

	m_pD2DText = NULL;

	m_nViewerHeight = -1;
	m_nViewerWidth = -1;

	m_nSrcHeight = -1;
	m_nSrcWidth = -1;

	m_fWriteRatio = 1.;
	m_fHeightRatio = 1.;

	memset(&m_detect_info, 0, sizeof(detection_Info_s));	
}

CMyViewerStc::~CMyViewerStc()
{
	DeleteRenderer();
}

BEGIN_MESSAGE_MAP(CMyViewerStc, CWnd)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()	
END_MESSAGE_MAP()

void CMyViewerStc::SetTargetInfo(int nChannel, detection_Info_s *pDetectInfo)
{
	memcpy(&m_detect_info, pDetectInfo, sizeof(detection_Info_s));
#if 0
	m_detectionInfo[nChannel].nIndex = pDetectInfo->nIndex;
	m_detectionInfo[nChannel].nTargetCount = pDetectInfo->nTargetCount;
	for (int i = 0; i < m_detectionInfo[nChannel].nTargetCount; i++) {
		m_detectionInfo[nChannel].nx[i] = pDetectInfo->nx[i];
		m_detectionInfo[nChannel].ny[i] = pDetectInfo->ny[i];
		m_detectionInfo[nChannel].nWidth[i] = pDetectInfo->nWidth[i];
		m_detectionInfo[nChannel].nheight[i] = pDetectInfo->nheight[i];
	}
#endif
}

void CMyViewerStc::DrawData(int nSrcWidth, int nSrcHeight, uint8_t* pData)
{
	if (m_nSrcWidth != nSrcWidth || m_nSrcHeight != nSrcHeight) {
		m_nSrcWidth = nSrcWidth;
		m_nSrcHeight = nSrcHeight;

		if (m_pRGB32) {
			delete[] m_pRGB32;
		}
		
		m_pRGB32 = new char[m_nSrcWidth * m_nSrcHeight * 4];
	}

	/*int nSize = m_nSrcWidth * m_nSrcHeight * 4;
	memcpy(m_pRGB32, pData, nSize);*/

	//m_pBmpImage->CopyFromMemory(NULL, pData, m_nSrcWidth * 4);

	Draw();
}

void CMyViewerStc::DrawGRAY8(int nSrcWidth, int nSrcHeight, uint8_t* pData)
{
	if (!m_pBmpImage) {
		return;
	}

	m_csDraw.Lock();

	if (m_nSrcWidth != nSrcWidth || m_nSrcHeight != nSrcHeight) {
		m_nSrcWidth = nSrcWidth;
		m_nSrcHeight = nSrcHeight;

		if (m_pRGB32) {
			delete[] m_pRGB32;
		}

		m_pRGB32 = new char[m_nViewerWidth * m_nViewerWidth * 4];

		if (m_pSwsCtx) {
			sws_freeContext(m_pSwsCtx);			
		}
		m_pSwsCtx = NULL;		
	}

	if (!m_pSwsCtx) {
		m_pSwsCtx = sws_getContext(nSrcWidth, nSrcHeight, AV_PIX_FMT_GRAY8, m_nViewerWidth, m_nViewerHeight, AV_PIX_FMT_RGB32, SWS_FAST_BILINEAR, NULL, NULL, NULL);
		m_fmtSrc = AV_PIX_FMT_GRAY8;

		m_fWriteRatio = (double)m_nViewerWidth / nSrcWidth;
		m_fHeightRatio = (double)m_nViewerHeight / nSrcHeight;
	}

	if (m_pSwsCtx) {
		uint8_t *src_data[4], *dst_data[4];
		int src_line[4], dst_line[4];

		src_data[0] = pData;
		src_data[1] = pData + (m_nSrcWidth * m_nSrcHeight);
		src_data[2] = pData + (m_nSrcWidth * m_nSrcHeight) + (m_nSrcWidth * m_nSrcHeight / 4);
		src_data[3] = 0;

		src_line[0] = m_nSrcWidth;
		src_line[1] = m_nSrcWidth / 2;
		src_line[2] = m_nSrcWidth / 2;
		src_line[3] = 0;

		dst_data[0] = (uint8_t*)m_pRGB32;
		dst_data[1] = 0;
		dst_data[2] = 0;
		dst_data[3] = 0;

		dst_line[0] = m_nViewerWidth * 4;
		dst_line[1] = 0;
		dst_line[2] = 0;
		dst_line[3] = 0;

		sws_scale(m_pSwsCtx, src_data, src_line, 0, m_nSrcHeight, dst_data, dst_line);

		if (m_pBmpImage) {
			m_pBmpImage->CopyFromMemory(NULL, m_pRGB32, m_nViewerWidth * 4);
		}
	}
	m_csDraw.Unlock();

	Draw();
}

void CMyViewerStc::DrawYUV420(int nSrcWidth, int nSrcHeight, uint8_t* pData)
{
	if (!m_pBmpImage) {
		return;
	}

	m_csDraw.Lock();

	if (m_nSrcWidth != nSrcWidth || m_nSrcHeight != nSrcHeight) {
		m_nSrcWidth = nSrcWidth;
		m_nSrcHeight = nSrcHeight;

		if (m_pRGB32) {
			delete[] m_pRGB32;
		}

		m_pRGB32 = new char[m_nViewerWidth * m_nViewerWidth * 4];
			   		
		if (m_pSwsCtx) {
			sws_freeContext(m_pSwsCtx);			
		}
		m_pSwsCtx = NULL;		
	}

	if (!m_pSwsCtx) {
		m_pSwsCtx = sws_getContext(nSrcWidth, nSrcHeight, AV_PIX_FMT_YUV420P, m_nViewerWidth, m_nViewerHeight, AV_PIX_FMT_RGB32, SWS_FAST_BILINEAR, NULL, NULL, NULL);
		m_fmtSrc = AV_PIX_FMT_YUV420P;

		m_fWriteRatio = (double)m_nViewerWidth / nSrcWidth;
		m_fHeightRatio = (double)m_nViewerHeight / nSrcHeight;
	}

	if (m_pSwsCtx) {
		uint8_t *src_data[4], *dst_data[4];
		int src_line[4], dst_line[4];

		src_data[0] = pData;
		src_data[1] = pData + (m_nSrcWidth * m_nSrcHeight);
		src_data[2] = pData + (m_nSrcWidth * m_nSrcHeight) + (m_nSrcWidth * m_nSrcHeight / 4);
		src_data[3] = 0;

		src_line[0] = m_nSrcWidth;
		src_line[1] = m_nSrcWidth / 2;
		src_line[2] = m_nSrcWidth / 2;
		src_line[3] = 0;

		dst_data[0] = (uint8_t*)m_pRGB32;
		dst_data[1] = 0;
		dst_data[2] = 0;
		dst_data[3] = 0;

		dst_line[0] = m_nViewerWidth * 4;
		dst_line[1] = 0;
		dst_line[2] = 0;
		dst_line[3] = 0;

		sws_scale(m_pSwsCtx, src_data, src_line, 0, m_nSrcHeight, dst_data, dst_line);
		
		
		if (m_pBmpImage) {
			m_pBmpImage->CopyFromMemory(NULL, m_pRGB32, m_nViewerWidth * 4);
		}
		
	}
	m_csDraw.Unlock();

	Draw();
}

void CMyViewerStc::DrawVideoFrame(AVFrame *pFrame)
{
	if (!m_pRGB32) {
		m_pRGB32 = new char[m_nViewerHeight * m_nViewerWidth * 4];
	}
	if (pFrame) {
		uint8_t *src_data[4], *dst_data[4];
		int src_line[4], dst_line[4];

		src_data[0] = (uint8_t*)pFrame->data[0];
		src_data[1] = (uint8_t*)pFrame->data[1];
		src_data[2] = (uint8_t*)pFrame->data[2];
		src_data[3] = (uint8_t*)pFrame->data[3];

		src_line[0] = pFrame->linesize[0];
		src_line[1] = pFrame->linesize[1];
		src_line[2] = pFrame->linesize[2];
		src_line[3] = pFrame->linesize[3];

		dst_data[0] = (uint8_t*)m_pRGB32;
		dst_data[1] = 0;
		dst_data[2] = 0;
		dst_data[3] = 0;

		dst_line[0] = m_nViewerWidth * 4;
		dst_line[1] = 0;
		dst_line[2] = 0;
		dst_line[3] = 0;

		if (m_fmtSrc != pFrame->format || pFrame->width != m_nSrcWidth || pFrame->height != m_nSrcHeight) {
			if (m_pSwsCtx) {
				sws_freeContext(m_pSwsCtx);
				m_pSwsCtx = NULL;
			}
		}
		if (!m_pSwsCtx) {
			m_pSwsCtx = sws_getContext(pFrame->width, pFrame->height, (AVPixelFormat)pFrame->format, m_nViewerWidth, m_nViewerHeight, AV_PIX_FMT_RGB32, SWS_FAST_BILINEAR, NULL, NULL, NULL);
			m_nSrcWidth = pFrame->width;
			m_nSrcHeight = pFrame->height;

			m_fmtSrc = (AVPixelFormat)pFrame->format;
		}
				
		sws_scale(m_pSwsCtx, src_data, src_line, 0, pFrame->height, dst_data, dst_line);

		m_pBmpImage->CopyFromMemory(NULL, m_pRGB32, m_nViewerWidth * 4);

//		memcpy(&m_fLastPTS, &pFrame->pts, sizeof(double));

		Draw();		
	}
}

void CMyViewerStc::Draw()
{
	HRESULT hr;	
	
	m_csDraw.Lock();
	if (!m_pBrushRect) {
		m_ipRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Red), &m_pBrushRect);
	}

	if (m_ipRenderTarget) {
		if (!(m_ipRenderTarget->CheckWindowState() & D2D1_WINDOW_STATE_OCCLUDED)) {
			D2D1_SIZE_F size = m_ipRenderTarget->GetSize();
			if ((size.width > 0.f) && (size.height > 0.f)) {

				m_ipRenderTarget->BeginDraw();

				m_ipRenderTarget->SetTransform(D2D1::Matrix3x2F::Identity());
				m_ipRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::Black));

				D2D1_RECT_F src_rc = D2D1::RectF(0.f, 0.f, size.width, size.height);
				D2D1_RECT_F dst_rc = D2D1::RectF(0.f, 0.f, size.width, size.height);

				src_rc = D2D1::RectF(0.f, 0.f, m_nViewerWidth, m_nViewerHeight);
				dst_rc = D2D1::RectF(0.f, 0.f, m_nViewerWidth, m_nViewerHeight);
				if (m_bExpand) {
					src_rc = D2D1::RectF(m_ptClickDown.x, m_ptClickDown.y, m_ptClickUp.x, m_ptClickUp.y);
				}
				m_ipRenderTarget->DrawBitmap(m_pBmpImage, &dst_rc, 1.0f, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, &src_rc);
				//m_ipRenderTarget->DrawBitmap(m_pBmpImage, &dst_rc, 1.0f, D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR, &src_rc);
								
				if (m_bExpand == false) {
					DrawDetectionInfo();
				}

				if (m_bMouseClicked) {
					D2D1_RECT_F rect;
					rect = D2D1::RectF(m_ptClickDown.x, m_ptClickDown.y, m_ptClickUp.x, m_ptClickUp.y);
					m_ipRenderTarget->DrawRectangle(&rect, m_pBrushRect, 3.f);
				}

				hr = m_ipRenderTarget->EndDraw();
				if (hr == D2DERR_RECREATE_TARGET) {
					_d(L"[Preview] Draw 실패. 타겟 재생성\n");
				}
			}
		}
	}
	m_csDraw.Unlock();
}

void CMyViewerStc::DrawDetectionInfo()
{
	CString strCh;
	CRect rc;

	D2D1_SIZE_F size = m_ipRenderTarget->GetSize();

	//> 탐지정보
	int nTarget = m_detect_info.nTargetCount;
	if (nTarget > 0) {		
		CPoint pt1, pt2;
		for (int i = 0; i < nTarget; i++) {
			pt1.x = m_detect_info.nx[i] * m_fWriteRatio;
			pt1.y = m_detect_info.ny[i] * m_fHeightRatio;

			pt2.x = pt1.x + (m_detect_info.nWidth[i] * m_fWriteRatio);
			pt2.y = pt1.y + (m_detect_info.nHeight[i] * m_fHeightRatio);

			m_ipRenderTarget->DrawLine(D2D1::Point2F(pt1.x, pt1.y), D2D1::Point2F(pt2.x, pt1.y), m_pSolidBrushR, 2);
			m_ipRenderTarget->DrawLine(D2D1::Point2F(pt1.x, pt1.y), D2D1::Point2F(pt1.x, pt2.y), m_pSolidBrushR, 2);
			m_ipRenderTarget->DrawLine(D2D1::Point2F(pt2.x, pt1.y), D2D1::Point2F(pt2.x, pt2.y), m_pSolidBrushR, 2);
			m_ipRenderTarget->DrawLine(D2D1::Point2F(pt1.x, pt2.y), D2D1::Point2F(pt2.x, pt2.y), m_pSolidBrushR, 2);

			strCh.Format(L"%d", m_detect_info.nx[i]);
			rc.left = pt2.x + 5;
			rc.top = pt1.y;
			rc.right = rc.left + 80;
			rc.bottom = rc.top + 20;

			DrawString(strCh, &rc, m_pD2DText, m_pSolidBrushR);
		}	
	}
}

void CMyViewerStc::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.	
//	GetParent()->PostMessage(WN_TNM_VIEWER_CLICKED, m_nIndex);
	m_bExpand = false;
	m_bMouseClicked = true;
	m_ptClickDown.SetPoint(point.x, point.y);
	m_ptClickUp.SetPoint(point.x, point.y);

	SetCapture();
	
	CWnd::OnLButtonDown(nFlags, point);
}


void CMyViewerStc::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	m_bMouseClicked = false;
	m_bExpand = false;

//	_d(L"[viewer] double click\n");
	CWnd::OnLButtonDblClk(nFlags, point);
}


void CMyViewerStc::OnLButtonUp(UINT nFlags, CPoint point)
{	
	CRect rc;
	if (m_bMouseClicked) {
		if ((m_ptClickDown.x != m_ptClickUp.x) && (m_ptClickDown.y != m_ptClickUp.y)) {
			m_bExpand = true;
		}
	}
	m_bMouseClicked = false;

	::ReleaseCapture();

	CWnd::OnLButtonUp(nFlags, point);
}


void CMyViewerStc::OnMouseMove(UINT nFlags, CPoint point)
{
	if (m_bMouseClicked) {
#if 0
		CPoint movepos;
		GetCursorPos(&movepos);	//> 커서 포인트 수집
		ScreenToClient(&movepos);	//> 스트린 좌표에서 클라이언트 좌표 기준으로 변경
		_d(L"[Priview] Click move. point: (%d, %d), pos: (%d, %d)\n", point.x, point.y, movepos.x, movepos.y);
#endif
		m_ptClickUp.SetPoint(point.x, point.y);

		if (!m_rcViewer.PtInRect(point)) {
			//		_d(L"[Priview] 이탈\n");
			//> 새로운 선택영역 라벨 생성
	//		m_rcTempLabel.SetRect(m_ptClickDown.x, m_ptClickDown.y, point.x, point.y);

			m_bMouseClicked = false;
			m_bExpand = false;
			::ReleaseCapture();
		}

		Draw();
	}

	CWnd::OnMouseMove(nFlags, point);
}

void CMyViewerStc::DrawString(CString strData, CRect *rc, IDWriteTextFormat *pText, ID2D1SolidColorBrush *pBrush)
{
	int nLen = 0;
	wchar_t wc_ch[128] = { 0, };

	//memset(wc_ch, 0, sizeof(wc_ch));
	memset(wc_ch, 0, 128);
//	nLen = MultiByteToWideChar(CP_ACP, 0, strData, strlen(strData), NULL, NULL);
//	MultiByteToWideChar(CP_ACP, 0, strData, strlen(strData), wc_ch, nLen);

	_stprintf(wc_ch, L"%s", strData);

	D2D1_RECT_F rectCh;
	rectCh = D2D1::RectF((FLOAT)rc->left, (FLOAT)rc->top, (FLOAT)rc->right, (FLOAT)rc->bottom);
	m_ipRenderTarget->DrawText(wc_ch, wcslen(wc_ch), pText, rectCh, m_pSolidBrushB);

	rc->OffsetRect(-1, -1);
	rectCh = D2D1::RectF((FLOAT)rc->left, (FLOAT)rc->top, (FLOAT)rc->right, (FLOAT)rc->bottom);
	m_ipRenderTarget->DrawText(wc_ch, wcslen(wc_ch), pText, rectCh, pBrush);
}

void CMyViewerStc::ResizeViewer()
{
	DeleteRenderer();
	CreateRenderer();
}
void CMyViewerStc::DeleteViewer()
{
	DeleteRenderer();
}

void CMyViewerStc::CreateViewer(const RECT &rect, CWnd *pParentWnd, UINT nID)
{
	CWnd::Create(NULL, L"", WS_CHILD | WS_VISIBLE, rect, pParentWnd, nID);

	CreateRenderer();
}

void CMyViewerStc::CreateRenderer()
{
	CRect rc;

	HRESULT hr = E_FAIL;
	D2D1_PIXEL_FORMAT pixelFormat = {
		DXGI_FORMAT_B8G8R8A8_UNORM,
		D2D1_ALPHA_MODE_PREMULTIPLIED
	};

	D2D1_RENDER_TARGET_PROPERTIES renderTargetProps = {
		D2D1_RENDER_TARGET_TYPE_DEFAULT,
		pixelFormat,
		0,
		0,
		D2D1_RENDER_TARGET_USAGE_NONE,
		D2D1_FEATURE_LEVEL_DEFAULT
	};

	if (m_ipRenderTarget) {
		return;
	}

	m_csDraw.Lock();

	GetClientRect(&rc);

	m_nViewerHeight = rc.Height();
	m_nViewerWidth = rc.Width();
	m_rcViewer.CopyRect(&rc);

	D2D1_SIZE_U size = D2D1::SizeU(m_nViewerWidth, m_nViewerHeight);

	D2D1_HWND_RENDER_TARGET_PROPERTIES hWndRenderTargetProps = {
		m_hWnd, size, D2D1_PRESENT_OPTIONS_IMMEDIATELY };

	hr = ::g_ipD2DFactory->CreateHwndRenderTarget(renderTargetProps, hWndRenderTargetProps, &m_ipRenderTarget);
	if (hr != S_OK) {
		_d(L"[ADVIEW] Couldn't create Hardware Render target");
	}

	FLOAT dpiX, dpiY;
	g_ipD2DFactory->GetDesktopDpi(&dpiX, &dpiY);

	D2D1_BITMAP_PROPERTIES bitmapProps = {
		pixelFormat,
		dpiX,
		dpiY
	};
#if 0
	D2D1_SIZE_U bitmapSize = {
		MAX_WIDTH,
		MAX_HEIGHT
};
#else
	D2D1_SIZE_U bitmapSize = {
		m_nViewerWidth,
		m_nViewerHeight
	};
#endif

	g_ipWriteFactory->CreateTextFormat(L"Courier New", NULL,
		DWRITE_FONT_WEIGHT_BLACK,
		DWRITE_FONT_STYLE_NORMAL,
		DWRITE_FONT_STRETCH_NORMAL,
		15.0f, L"", &m_pD2DText);

	m_nSrcHeight = -1;
	m_nSrcWidth = -1;

	m_ipRenderTarget->CreateBitmap(bitmapSize, bitmapProps, &m_pBmpImage);

	m_ipRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black), &m_pSolidBrushB);
	m_ipRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Red), &m_pSolidBrushR);

	m_csDraw.Unlock();

	//	m_pRenderTarget = new CHwndRenderTarget;	
	//	m_pRenderTarget->Attach(m_ipRenderTarget);
	////	GetRenderTarget()->Attach(m_ipRenderTarget);
	//
	//	//Create Direct2D resources	
	//	m_pBitmap = new CD2DBitmap();
	//	HRESULT hr = m_pBitmap->Create(pRenderTarget);
	//	if (FAILED(hr))
	//		return -1;
}

void CMyViewerStc::DeleteRenderer()
{
	m_csDraw.Lock();

	if (m_ipRenderTarget) {
		if (!(m_ipRenderTarget->CheckWindowState() & D2D1_WINDOW_STATE_OCCLUDED)) {
			m_ipRenderTarget->BeginDraw();

			m_ipRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::Black));
			m_ipRenderTarget->EndDraw();
		}

		if (m_pRenderTarget) {
			m_pRenderTarget->Detach();
		}
		m_ipRenderTarget->Release();
		m_ipRenderTarget = NULL;
	}

	SAFE_DELETE(m_pRenderTarget);

	if (m_pBmpImage) {
		m_pBmpImage->Release();
	}
	m_pBmpImage = NULL;

	if (m_pSwsCtx) {
		sws_freeContext(m_pSwsCtx);
	}
	m_pSwsCtx = NULL;

	if (m_pBrushRect) {
		m_pBrushRect->Release();
	}
	m_pBrushRect = NULL;

	if (m_pRGB32) {
		delete[] m_pRGB32;
		m_pRGB32 = NULL;
	}

	if (m_pSolidBrushB) {
		m_pSolidBrushB->Release();
	}
	m_pSolidBrushB = NULL;

	if (m_pSolidBrushR) {
		m_pSolidBrushR->Release();
	}
	m_pSolidBrushR = NULL;

	if (m_pD2DText) {
		m_pD2DText->Release();
	}
	m_pD2DText = NULL;

	m_csDraw.Unlock();
}

#if 0
for (int a = 0; a < g_nCaseMaxCh[m_nCaseNum]; a++) {
	int nCamNum = m_ImgInfo[a].nCam;
	int nRobotNum = m_ImgInfo[a].nRobot;

	if (nRobotNum < 0) {
		continue;
	}

	//> 주행보조선
	if ((nCamNum == CAMID_FRONT_CCD) || (nCamNum == CAMID_FRONT_IR)) {
		DrawGuideLine(a);
	}
	if (nCamNum == CAMID_REAR) {
		DrawRearGuidLine(a);
	}

	if (nCamNum == CAMID_DETECT_IR) {
		bool bPanoramic = false;
		if ((m_nCaseNum == 2) || (m_nCaseNum == 3)) {
			if (a == 1) {
				bPanoramic = true;
			}
		}

		if (bPanoramic) {
			//> 탐지정보
			DrawTargetInfo(a);
		}
		else {
			//> 자동추적창
			if (m_pOverlay->TraceInfo[nRobotNum].nCtrl != 0) {
				DrawTraceLine(a);
			}
		}
	}

	//>  오버레이 버튼
	DrawOverlayBtn(a);

	if ((nCamNum == CAMID_DETECT_CCD) || (nCamNum == CAMID_DETECT_IR)) {
		//> 차선정보 표시			
		DrawTargetLine(a);

		//> 고각
		strCh.Format("%.02f", m_pOverlay->SSI[nRobotNum].usAltitude * 0.01);

		rc.left = g_rcChannel[m_nCaseNum][a].right - 100;
		rc.top = g_rcChannel[m_nCaseNum][a].top + 20;
		rc.right = rc.left + 80;
		rc.bottom = rc.top + 20;

		DrawString(strCh, &rc, m_pD2DText, m_pSolidBrushW);

		//> 방위각			
		strCh.Format("%.02f", m_pOverlay->SSI[nRobotNum].usAzimuth * 0.01);

		rc.left = g_rcChannel[m_nCaseNum][a].right - 100;
		rc.top = g_rcChannel[m_nCaseNum][a].top + 40;
		rc.right = rc.left + 80;
		rc.bottom = rc.top + 20;

		DrawString(strCh, &rc, m_pD2DText, m_pSolidBrushW);

		//> 배율						
		if (nCamNum == CAMID_DETECT_IR) {
			strCh.Format("X %d", m_pOverlay->SSI[nRobotNum].ucIR_Zoom);
		}
		else {
			strCh.Format("X %d", m_pOverlay->SSI[nRobotNum].ucCCD_Zoom);
		}

		rc.left = g_rcChannel[m_nCaseNum][a].right - 100;
		rc.top = g_rcChannel[m_nCaseNum][a].top + 60;
		rc.right = rc.left + 80;
		rc.bottom = rc.top + 20;

		DrawString(strCh, &rc, m_pD2DText, m_pSolidBrushW);

		//> 수직확장								
		strCh.Format("%d cm", m_pOverlay->SSI[nRobotNum].usExpansionHeight);

		rc.left = g_rcChannel[m_nCaseNum][a].right - 100;
		rc.top = g_rcChannel[m_nCaseNum][a].top + 80;
		rc.right = rc.left + 80;
		rc.bottom = rc.top + 20;

		DrawString(strCh, &rc, m_pD2DText, m_pSolidBrushW);

		//> LRF						
		strCh.Format("%d m", m_pOverlay->nLRF[nRobotNum]);

		rc.left = g_rcChannel[m_nCaseNum][a].right - 100;
		rc.top = g_rcChannel[m_nCaseNum][a].top + 100;
		rc.right = rc.left + 80;
		rc.bottom = rc.top + 20;

		DrawString(strCh, &rc, m_pD2DText, m_pSolidBrushW);

		//> 무장정보				
		strCh.Format("%s", g_strWeaponName[m_pOverlay->WSI[nRobotNum].ucType]);

		rc.left = g_rcChannel[m_nCaseNum][a].left + 120;
		rc.top = g_rcChannel[m_nCaseNum][a].top + 10;
		rc.right = rc.left + 80;
		rc.bottom = rc.top + 20;

		DrawString(strCh, &rc, m_pD2DText, m_pSolidBrushW);
}
	}
#endif



#if 0
/* brightness. 밝기
- WHITE-POINT. default 1.0f, 1.0f
- BLACK-POINT. default 0.0f, 0.0f
ComPtr<ID2D1Effect> brightnessEffect;
m_d2dContext->CreateEffect(CLSID_D2D1Brightness, &brightnessEffect);

brightnessEffect->SetValue(D2D1_BRIGHTNESS_PROP_BLACK_POINT, D2D1::Vector2F(0.0f, 0.2f));

m_d2dContext->BeginDraw();
m_d2dContext->DrawImage(brightnessEffect.Get());
m_d2dContext->EndDraw();
*/
#if 0
// Obtain hwndRenderTarget's deviceContext
ID2D1DeviceContext *deviceContext;
m_ipRenderTarget->QueryInterface(&deviceContext);

// create Gaussian blur effect
CComPtr<ID2D1Effect> spEffect;
HRESULT hr = deviceContext->CreateEffect(CLSID_D2D1GaussianBlur, &spEffect);
ATLASSERT(SUCCEEDED(hr));

// set "StandardDeviation" property value (default value is 3.0f)
hr = spEffect->SetValue(D2D1_GAUSSIANBLUR_PROP_STANDARD_DEVIATION, 2.0f);
ATLASSERT(SUCCEEDED(hr));
#endif
#if 0
CComPtr<ID2D1Effect> brightnessEffect;
hr = deviceContext->CreateEffect(CLSID_D2D1Brightness, &brightnessEffect);
ATLASSERT(SUCCEEDED(hr));

hr = brightnessEffect->SetValue(D2D1_BRIGHTNESS_PROP_BLACK_POINT, D2D1::Vector2F(0.9f, 0.9f));
ATLASSERT(SUCCEEDED(hr));

// set the input image
brightnessEffect->SetInput(0, m_pBmpImage);
#endif
// Obtain hwndRenderTarget's deviceContext
ID2D1DeviceContext *deviceContext;
m_ipRenderTarget->QueryInterface(&deviceContext);

// create Gaussian blur effect
CComPtr<ID2D1Effect> spEffect;
HRESULT hr = deviceContext->CreateEffect(CLSID_D2D1GaussianBlur, &spEffect);
ATLASSERT(SUCCEEDED(hr));

// set "StandardDeviation" property value (default value is 3.0f)
hr = spEffect->SetValue(D2D1_GAUSSIANBLUR_PROP_STANDARD_DEVIATION, 2.0f);
ATLASSERT(SUCCEEDED(hr));

// set the input image
spEffect->SetInput(0, m_pBmpImage);

// finally, call ID2D1DeviceContext::DrawImage
deviceContext->BeginDraw();
deviceContext->DrawImage(spEffect);

DrawOverlay((ID2D1HwndRenderTarget*)deviceContext);

if (m_bMouseClicked) {
	D2D1_RECT_F rect;
	rect = D2D1::RectF(m_ptClickDown.x, m_ptClickDown.y, m_ptClickUp.x, m_ptClickUp.y);
	deviceContext->DrawRectangle(&rect, m_pBrushRect, 3.f);
}
deviceContext->EndDraw();
#endif