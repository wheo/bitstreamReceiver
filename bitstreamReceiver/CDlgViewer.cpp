// CDlgViewer.cpp: 구현 파일
//

#include "stdafx.h"
#include "bitstreamReceiver.h"
#include "CDlgViewer.h"
#include "afxdialogex.h"


// CDlgViewer 대화 상자

IMPLEMENT_DYNAMIC(CDlgViewer, CDialogEx)

CDlgViewer::CDlgViewer(CWnd* pParent /*=nullptr*/, int nIndex)
	: CDialogEx(IDD_SUB_VIEWER, pParent)
{
	m_nIndex = nIndex;

	memset(&m_ch_cfg, 0, sizeof(channel_cfg_s));

	mp_tnm_channel = NULL;
}

CDlgViewer::~CDlgViewer()
{
}

void CDlgViewer::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CDlgViewer, CDialogEx)
	ON_WM_DESTROY()
	ON_WM_ERASEBKGND()
	ON_WM_SIZE()
END_MESSAGE_MAP()


bool CDlgViewer::connect_channel()
{
	if (!mp_tnm_channel) {
		return false;
	}
	
	return mp_tnm_channel->start_channel(m_ch_cfg);
}
void CDlgViewer::set_channel_info(channel_cfg_s *pCfg)
{
	memcpy(&m_ch_cfg, pCfg, sizeof(channel_cfg_s));
}

void CDlgViewer::display_status(bool bPlay)
{
	if (!mp_tnm_channel) {
		return;
	}

	mp_tnm_channel->set_display(bPlay);
}

void CDlgViewer::disconnect_channel()
{
	if (mp_tnm_channel) {
		mp_tnm_channel->stop_channel();
	}
}

void CDlgViewer::set_contrast(int nPos)
{
	if (!mp_tnm_channel) {
		return;
	}
	mp_tnm_channel->set_contrast(nPos);
}
void CDlgViewer::set_brightness(int nPos)
{
	if (!mp_tnm_channel) {
		return;
	}
	mp_tnm_channel->set_brightness(nPos);
}
void CDlgViewer::set_saturation(int nPos)
{
	if (!mp_tnm_channel) {
		return;
	}
	mp_tnm_channel->set_saturation(nPos);
}

void CDlgViewer::enable_audio(int nStream)
{
	if (!mp_tnm_channel) {
		return;
	}
	mp_tnm_channel->enable_audio(nStream);
}


void CDlgViewer::set_crop_value(int nLeft, int nTop, int nRight, int nBottom)
{
	if (!mp_tnm_channel) {
		return;
	}
	mp_tnm_channel->set_crop_value(nLeft, nTop, nRight, nBottom);
}

void CDlgViewer::set_viewer_resolution(int nPercent)
{
	if (!mp_tnm_channel) {
		return;
	}
//	_d(_T("[viewer] resolution percent: %d\n"), nPercent);
	resize_viewer(nPercent);
}

BOOL CDlgViewer::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	::SetWindowText(this->m_hWnd, g_strChannelName[m_nIndex]);

	mp_tnm_channel = new Ctnm_channel(m_nIndex);
	mp_tnm_channel->create_viewer(this, CRect(10, 10, 10, 10), IDC_CHANNEL_VIEWER + m_nIndex);

	resize_viewer(100);	

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}

void CDlgViewer::resize_viewer(int percent)
{
	int nWidth = m_ch_cfg.mux.vid.width;
	int nHeight = m_ch_cfg.mux.vid.height;

	if (m_ch_cfg.mux.vid.is_interlace) {
	//	nHeight *= 2;
	}

	nWidth *= percent;
	nWidth /= 100;
	nHeight *= percent;
	nHeight /= 100;

	CRect rc, rcc;

	GetClientRect(&rcc);
	GetWindowRect(&rc);
	   	 
	::MoveWindow(this->m_hWnd, rc.left, rc.top, nWidth + 16 + 2, nHeight + 39 + 2, TRUE);
}


void CDlgViewer::OnDestroy()
{
	CDialogEx::OnDestroy();

	disconnect_channel();

	SAFE_DELETE(mp_tnm_channel);
}


BOOL CDlgViewer::OnEraseBkgnd(CDC* pDC)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.

	return CDialogEx::OnEraseBkgnd(pDC);
}


void CDlgViewer::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);
	
	CRect rc, rcc;

	GetClientRect(&rcc);
		
	rc.top = 1;
	rc.left = 1;
	rc.right = rcc.right - 1;
	rc.bottom = rcc.bottom - 1;
	
	if (mp_tnm_channel) {
		mp_tnm_channel->resize_viewer(rc);
	}
}

LRESULT CDlgViewer::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.

	return CDialogEx::WindowProc(message, wParam, lParam);
}
