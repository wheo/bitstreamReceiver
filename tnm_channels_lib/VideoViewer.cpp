#include "stdafx.h"
#include "VideoViewer.h"

CVideoViewer::CVideoViewer()
{
	m_ipRenderTarget = NULL;
	m_pBmpImage = NULL;
	
	m_pBrushRect = NULL;

	m_pRGB32 = NULL;
	m_pSwsCtx = NULL;

	m_pD2DText = NULL;
	m_pSolidBrushB = NULL;
	m_pSolidBrushR = NULL;

	m_nSrcHeight = -1;
	m_nSrcWidth = -1;

	m_fWidthRatio = 1.;
	m_fHeightRatio = 1.;

	m_cur_mouse_x = 0;
	m_cur_mouse_y = 0;

	m_recv_frame_count = 0;

	m_bMouseClicked = false;
	m_bExpand = false;	

	m_str_connected_ip = _T("");

	CreateD2D();
}

CVideoViewer::~CVideoViewer()
{
	DeleteViewer();

	DeleteD2D();	
}

BEGIN_MESSAGE_MAP(CVideoViewer, CWnd)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()

void CVideoViewer::SetTargetInfo(int nChannel, detection_Info_s *pDetectInfo)
{
	memcpy(&m_detect_info, pDetectInfo, sizeof(detection_Info_s));
}

void CVideoViewer::set_cur_connected_ip(CString strIP)
{
	m_str_connected_ip.Format(_T("%s"), strIP);
}

void CVideoViewer::set_recv_frame_count(int nFPS)
{
	m_recv_frame_count = nFPS;
}

void CVideoViewer::ResizeViewer()
{
	DeleteViewer();
	CreateViewer();
}

bool CVideoViewer::CreateViewer()
{
	HRESULT hr = E_FAIL;
	CRect rc;

//	m_csDraw.Lock();

	GetClientRect(&rc);

	m_rcViewer.left = 0;
	m_rcViewer.top = 0;
	m_rcViewer.right = rc.Width();
	m_rcViewer.bottom = rc.Height();

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

	D2D1_SIZE_U size = D2D1::SizeU(m_rcViewer.Width(), m_rcViewer.Height());
	//D2D1_SIZE_U size = D2D1::SizeU(m_nDstWidth, m_nDstHeight);

	D2D1_HWND_RENDER_TARGET_PROPERTIES hWndRenderTargetProps = {
		m_hWnd, size, D2D1_PRESENT_OPTIONS_IMMEDIATELY };

	hr = gp_D2DFactory->CreateHwndRenderTarget(renderTargetProps, hWndRenderTargetProps, &m_ipRenderTarget);
	if (hr != S_OK) {
		_d(_T("[ADVIEW] Couldn't create Hardware Render target"));
//		m_csDraw.Unlock();
		return false;
	}

	FLOAT dpiX, dpiY;
	gp_D2DFactory->GetDesktopDpi(&dpiX, &dpiY);

	D2D1_BITMAP_PROPERTIES bitmapProps = {
		pixelFormat,
		dpiX,
		dpiY
	};

	D2D1_SIZE_U bitmapSize = {
		m_rcViewer.Width(),
		m_rcViewer.Height()
	};

	m_ipRenderTarget->CreateBitmap(bitmapSize, bitmapProps, &m_pBmpImage);

	gp_WriteFactory->CreateTextFormat(L"Courier New", NULL,
		DWRITE_FONT_WEIGHT_BLACK,
		DWRITE_FONT_STYLE_NORMAL,
		DWRITE_FONT_STRETCH_NORMAL,
		15.0f, L"", &m_pD2DText);

	m_ipRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black), &m_pSolidBrushB);
	m_ipRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Red), &m_pSolidBrushR);

	_d(_T("[viewer] Create Viewer. %d, %d, %d, %d\n"), m_rcViewer.top, m_rcViewer.left, m_rcViewer.bottom, m_rcViewer.right);
//	m_csDraw.Unlock();

	return true;
}
bool CVideoViewer::CreateViewer(const RECT &rect, CWnd *pParentWnd, UINT nID)
{
	DeleteViewer();

	CWnd::Create(NULL, _T(""), WS_CHILD | WS_VISIBLE, rect, pParentWnd, nID);
	
	return CreateViewer();
}

void CVideoViewer::get_cur_mouse_pos(int &nx, int &ny)
{
	nx = m_cur_mouse_x;
	ny = m_cur_mouse_y;
}

void CVideoViewer::DeleteViewer()
{
//	m_csDraw.Lock();

	if (m_ipRenderTarget) {
		m_ipRenderTarget->Release();		
	}
	m_ipRenderTarget = NULL;

	if (m_pBmpImage) {
		m_pBmpImage->Release();
	}
	m_pBmpImage = NULL;

	if (m_pSwsCtx) {
		sws_freeContext(m_pSwsCtx);
	}
	m_pSwsCtx = NULL;
	
	if (m_pRGB32) {
		delete[] m_pRGB32;
	}
	m_pRGB32 = NULL;

	if (m_pBrushRect) {
		m_pBrushRect->Release();
	}
	m_pBrushRect = NULL;

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

//	m_csDraw.Unlock();
	
	_d(_T("[viewer] Delete Viewer.\n"));	
}

void CVideoViewer::DrawYUV420(int nSrcWidth, int nSrcHeight, uint8_t* pData)
{
	if (!m_pBmpImage) {
		return;
	}

//	m_csDraw.Lock();
	
	if (m_nSrcWidth != nSrcWidth || m_nSrcHeight != nSrcHeight) {
		m_nSrcWidth = nSrcWidth;
		m_nSrcHeight = nSrcHeight;

		if (m_pRGB32) {
			delete[] m_pRGB32;
		}
		m_pRGB32 = NULL;		

		if (m_pSwsCtx) {
			sws_freeContext(m_pSwsCtx);
		}
		m_pSwsCtx = NULL;
	}

	if (!m_pSwsCtx) {
		m_pSwsCtx = sws_getContext(nSrcWidth, nSrcHeight, AV_PIX_FMT_YUV420P, m_rcViewer.Width(), m_rcViewer.Height(), AV_PIX_FMT_RGB32, SWS_FAST_BILINEAR, NULL, NULL, NULL);
		m_fmtSrc = AV_PIX_FMT_YUV420P;

		m_fWidthRatio = (double)m_rcViewer.Width() / nSrcWidth;
		m_fHeightRatio = (double)m_rcViewer.Height() / nSrcHeight;
	}

	if (!m_pRGB32) {
		m_pRGB32 = new char[m_rcViewer.Width() * m_rcViewer.Height() * 4];
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

		dst_line[0] = m_rcViewer.Width() * 4;
		dst_line[1] = 0;
		dst_line[2] = 0;
		dst_line[3] = 0;

		sws_scale(m_pSwsCtx, src_data, src_line, 0, m_nSrcHeight, dst_data, dst_line);

		if (m_pBmpImage) {
			m_pBmpImage->CopyFromMemory(NULL, m_pRGB32, m_rcViewer.Width() * 4);
		}
	}	
//	m_csDraw.Unlock();

	Draw();
}

void CVideoViewer::ImageDraw(int nWidth, int nHeight, byte *pRGB32)
{
	if ((nWidth > 0) && (nHeight > 0) && m_pBmpImage) {
		m_nSrcWidth = nWidth;
		m_nSrcHeight = nHeight;

		m_pBmpImage->CopyFromMemory(NULL, pRGB32, m_nSrcWidth * 4);

		Draw();
	}
}

void CVideoViewer::ReDraw()
{
	Draw();
}

void CVideoViewer::Draw()
{
	if (!m_ipRenderTarget) {
		return;
	}
	if (!m_pBmpImage) {
		return;
	}

//	m_csDraw.Lock();
	if (!m_pBrushRect) {
		m_ipRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Red), &m_pBrushRect);
	}

	if (!(m_ipRenderTarget->CheckWindowState() & D2D1_WINDOW_STATE_OCCLUDED)) {
		HRESULT hr;
		D2D1_SIZE_F size = m_ipRenderTarget->GetSize();
		if ((size.width > 0.f) && (size.height > 0.f)) {			
			m_ipRenderTarget->BeginDraw();

			m_ipRenderTarget->SetTransform(D2D1::Matrix3x2F::Identity());
			m_ipRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::Black));

//			D2D1_RECT_F src_rc = D2D1::RectF(0.f, 0.f, m_nSrcWidth, m_nSrcHeight);
			D2D1_RECT_F src_rc = D2D1::RectF(0.f, 0.f, size.width, size.height);
			D2D1_RECT_F dst_rc = D2D1::RectF(0.f, 0.f, size.width, size.height);

			if (m_bExpand) {
				src_rc = D2D1::RectF(m_ptClickDown.x, m_ptClickDown.y, m_ptClickUp.x, m_ptClickUp.y);
			}

			m_ipRenderTarget->DrawBitmap(m_pBmpImage, &dst_rc, 1.0f, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, &src_rc);
			//m_ipRenderTarget->DrawBitmap(m_pBmpImage, &dst_rc, 1.0f, D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR, &src_rc);

			if (m_bMouseClicked) {
				D2D1_RECT_F rect;
				rect = D2D1::RectF(m_ptClickDown.x, m_ptClickDown.y, m_ptClickUp.x, m_ptClickUp.y);
				m_ipRenderTarget->DrawRectangle(&rect, m_pBrushRect, 3.f);
			}

			CString strText;
			//> 접속정보 오버레이.
			CRect rcText;
			rcText.top = 5;
			rcText.left = 5;
			rcText.bottom = rcText.top + 25;
			rcText.right = rcText.left + 150;
			DrawString(m_str_connected_ip, &rcText, m_pD2DText, m_pSolidBrushR);

			//> FPS 정보 오버레이.
			strText.Format(_T("%d"), m_recv_frame_count);
			rcText.top = size.height - 30;
			rcText.left = 5;
			rcText.bottom = rcText.top + 25;
			rcText.right = rcText.left + 50;

			DrawString(strText, &rcText, m_pD2DText, m_pSolidBrushR);

			//> 마우스 좌표.
			strText.Format(_T("x: %03d, y: %03d"), m_cur_mouse_x, m_cur_mouse_y);
			rcText.top = size.height - 30;
			rcText.left = size.width - 150;
			rcText.bottom = rcText.top + 25;
			rcText.right = rcText.left + 140;

			DrawString(strText, &rcText, m_pD2DText, m_pSolidBrushR);
			
			hr = m_ipRenderTarget->EndDraw();
			if (hr == D2DERR_RECREATE_TARGET) {
				_d(_T("[Viewer] Draw 실패. 타겟 재생성\n"));				
			}			
		}
	}

//	m_csDraw.Unlock();
}

void CVideoViewer::DrawString(CString strData, CRect *rc, IDWriteTextFormat *pText, ID2D1SolidColorBrush *pBrush)
{
	int nLen = 0;
	wchar_t wc_ch[128] = { 0, };

	memset(wc_ch, 0, sizeof(wc_ch));
	nLen = MultiByteToWideChar(CP_ACP, 0, strData, strlen(strData), NULL, NULL);
	MultiByteToWideChar(CP_ACP, 0, strData, strlen(strData), wc_ch, nLen);

	D2D1_RECT_F rectCh;
	rectCh = D2D1::RectF((FLOAT)rc->left, (FLOAT)rc->top, (FLOAT)rc->right, (FLOAT)rc->bottom);
	m_ipRenderTarget->DrawTextA(wc_ch, wcslen(wc_ch), pText, rectCh, m_pSolidBrushB);

	rc->OffsetRect(-1, -1);
	rectCh = D2D1::RectF((FLOAT)rc->left, (FLOAT)rc->top, (FLOAT)rc->right, (FLOAT)rc->bottom);
	m_ipRenderTarget->DrawTextA(wc_ch, wcslen(wc_ch), pText, rectCh, pBrush);
}

void CVideoViewer::CreateD2D() 
{
	HRESULT hr = E_FAIL;
	if (!gp_D2DFactory) {
		CoInitialize(0);

		hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_MULTI_THREADED, &gp_D2DFactory);
		if (hr != S_OK) {
			_d(_T("[D2D] Couldn't create ID2D1Factory\n"));
		}			
	}

	if (!gp_WriteFactory) {
		hr = ::DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), reinterpret_cast<IUnknown**>(&gp_WriteFactory));
		if (hr != S_OK)
		{
			_d("[D2D] Couldn't create IWriteFactory\n");
		}
	}
}

void CVideoViewer::DeleteD2D()
{
	/*if (g_ipWICFactory)
		g_ipWICFactory->Release();
	g_ipWICFactory = NULL;

	*/

	if (gp_WriteFactory) {
		gp_WriteFactory->Release();
		gp_WriteFactory = NULL;
	}	

	if (gp_D2DFactory) {
		gp_D2DFactory->Release();
		gp_D2DFactory = NULL;

		CoUninitialize();
	}
}


void CVideoViewer::OnLButtonDown(UINT nFlags, CPoint point)
{
	m_bExpand = false;
	m_bMouseClicked = true;
	m_ptClickDown.SetPoint(point.x, point.y);
	m_ptClickUp.SetPoint(point.x, point.y);

	SetCapture();

	CWnd::OnLButtonDown(nFlags, point);
}


void CVideoViewer::OnLButtonUp(UINT nFlags, CPoint point)
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


void CVideoViewer::OnMouseMove(UINT nFlags, CPoint point)
{
	CPoint movepos;
	GetCursorPos(&movepos);	//> 커서 포인트 수집
	ScreenToClient(&movepos);	//> 스트린 좌표에서 클라이언트 좌표 기준으로 변경

	m_cur_mouse_x = movepos.x;
	m_cur_mouse_y = movepos.y;

#if 0
	_d(_T("[viewer] mouse pos: (%d, %d), pos: (%d, %d)\n"), point.x, point.y, movepos.x, movepos.y);
#endif

	if (m_bMouseClicked) {
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



#if 0
if (m_bCrop) {
	if (m_nCropLeft > 0) {
		temp.left = src_rc.left + (m_nCropLeft*m_fWidthRatio);
	}
	if (m_nCropRight > 0) {
		temp.right = src_rc.right - (m_nCropRight*m_fWidthRatio);
	}
	if (m_nCropTop > 0) {
		temp.top = src_rc.top + (m_nCropTop*m_fHeightRatio);
	}
	if (m_nCropBottom > 0) {
		temp.bottom = src_rc.bottom - (m_nCropBottom*m_fHeightRatio);
	}

	if (temp.left >= 0 && temp.right <= size.width && temp.top >= 0 && temp.bottom <= size.height) {
		if (temp.right > temp.left && temp.bottom > temp.top) {
			bCrop = true;
		}
	}
}
if (bCrop) {
	if (m_bExpand) {
		double fWidthRatio = (temp.right - temp.left) / size.width;
		double fHeightRatio = (temp.bottom - temp.top) / size.height;

		src_rc = D2D1::RectF((m_ptClickDown.x*fWidthRatio), (m_ptClickDown.y*fHeightRatio), (m_ptClickUp.x*fWidthRatio), (m_ptClickUp.y*fHeightRatio));
	}
	else {
		src_rc = temp;
	}
}
else {
	if (m_bExpand) {
		src_rc = D2D1::RectF(m_ptClickDown.x, m_ptClickDown.y, m_ptClickUp.x, m_ptClickUp.y);
	}
}
#endif

BOOL CVideoViewer::OnEraseBkgnd(CDC* pDC)
{	
	Draw();

	return CWnd::OnEraseBkgnd(pDC);
}
