// CDlgChannel.cpp: 구현 파일
//

#include "stdafx.h"
#include "bitstreamReceiver.h"
#include "CDlgChannel.h"
#include "afxdialogex.h"


// CDlgChannel 대화 상자

IMPLEMENT_DYNAMIC(CDlgChannel, CDialogEx)
#define MAX_RESULUTION	4
static int g_nPercent[MAX_RESULUTION] = { 25, 50, 75, 100 };

CDlgChannel::CDlgChannel(CWnd* pParent /*=nullptr*/, int nIndex)
	: CDialogEx(IDD_SUB_CHANNEL_DIALOG, pParent)
{	
	m_nIndex = nIndex;		

	m_pParent = pParent;
		
	memset(&m_ch_cfg, 0, sizeof(channel_cfg_s));
}

CDlgChannel::~CDlgChannel()
{

}

void CDlgChannel::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CHANNEL_EDIT_IP, m_cEditIP);
	for (int i = 0; i < 2; i++) {
		DDX_Control(pDX, IDC_CHANNEL_BUTTON_CONNECT + i, m_cBtnConnect[i]);
	}
	
	DDX_Control(pDX, IDC_CHANNEL_BUTTON_START, m_cBtnPlay);
	DDX_Control(pDX, IDC_CHANNEL_BUTTON_PAUSE, m_cBtnPause);
	
	DDX_Control(pDX, IDC_COMBO_RESOLUTION, m_cComboResolurion);
}

BEGIN_MESSAGE_MAP(CDlgChannel, CDialogEx)
	ON_WM_SIZE()
	ON_BN_CLICKED(IDC_CHANNEL_BUTTON_CONNECT, &CDlgChannel::OnBnClickedChannelButtonConnect)
	ON_BN_CLICKED(IDC_CHANNEL_BUTTON_CONNECT2, &CDlgChannel::OnBnClickedChannelButtonConnect2)
	ON_BN_CLICKED(IDC_CHANNEL_BUTTON_START, &CDlgChannel::OnBnClickedChannelButtonStart)
	ON_BN_CLICKED(IDC_CHANNEL_BUTTON_PAUSE, &CDlgChannel::OnBnClickedChannelButtonPause)	
	ON_BN_CLICKED(IDC_RADIO_CHANNEL1, &CDlgChannel::OnBnClickedRadioChannel1)
	ON_BN_CLICKED(IDC_RADIO_CHANNEL2, &CDlgChannel::OnBnClickedRadioChannel2)
	ON_WM_TIMER()
	ON_WM_DESTROY()
	ON_CBN_SELCHANGE(IDC_COMBO_RESOLUTION, &CDlgChannel::OnSelchangeComboResolution)
END_MESSAGE_MAP()


// CDlgChannel 메시지 처리기
void CDlgChannel::set_channel_info(channel_cfg_s *pCfg)
{
	memcpy(&m_ch_cfg, pCfg, sizeof(channel_cfg_s));
}

BOOL CDlgChannel::OnInitDialog()
{
	CDialogEx::OnInitDialog();		

	m_cEditIP.SetWindowTextA(g_strChannelName[m_nIndex]);
		
	m_cBtnConnect[1].EnableWindow(FALSE);

	m_cBtnPause.EnableWindow(FALSE);
	m_cBtnPlay.EnableWindow(FALSE);
	m_cBtnPause.ShowWindow(SW_SHOW);
	m_cBtnPlay.ShowWindow(SW_HIDE);
	
	for (int i = 0; i < MAX_RESULUTION; i++) {
		CString strResolution;
		strResolution.Format(_T("%d%%"), g_nPercent[i]);
		m_cComboResolurion.AddString(strResolution);		
	}
	m_cComboResolurion.SetCurSel(MAX_RESULUTION-1);

//	SetTimer(1, 100, NULL);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}

void CDlgChannel::set_connected_status(bool IsConnected)
{
	if (IsConnected) {
		m_cBtnConnect[0].EnableWindow(FALSE);
		m_cBtnConnect[1].EnableWindow(TRUE);

		m_cBtnPause.EnableWindow();
		m_cBtnPlay.EnableWindow();
	}
	else {
		m_cBtnConnect[1].EnableWindow(FALSE);
		m_cBtnConnect[0].EnableWindow(TRUE);

		m_cBtnPause.EnableWindow(FALSE);
		m_cBtnPlay.EnableWindow(FALSE);
	}
}

void CDlgChannel::OnBnClickedChannelButtonConnect()
{
	if (m_pParent) {
		m_pParent->PostMessageA(TNM_CHANNEL_CONNECT, m_nIndex);
	}	
}

void CDlgChannel::OnBnClickedChannelButtonConnect2()
{
	if (m_pParent) {
		m_pParent->PostMessageA(TNM_CHANNEL_DISCONNECT, m_nIndex);
	}
}

void CDlgChannel::OnBnClickedChannelButtonStart()
{
	if (m_pParent) {
		m_pParent->PostMessageA(TNM_CHANNEL_PLAY, m_nIndex);
	}
	
	m_cBtnPause.ShowWindow(SW_SHOW);
	m_cBtnPlay.ShowWindow(SW_HIDE);
}

void CDlgChannel::OnBnClickedChannelButtonPause()
{
	if (m_pParent) {
		m_pParent->PostMessageA(TNM_CHANNEL_STOP, m_nIndex);
	}
	
	m_cBtnPause.ShowWindow(SW_HIDE);
	m_cBtnPlay.ShowWindow(SW_SHOW);
}

void CDlgChannel::OnBnClickedRadioChannel1()
{
	if (m_pParent) {
		m_pParent->PostMessageA(TNM_CHANNEL_AUDIO, m_nIndex, 1);
	}
}
void CDlgChannel::OnBnClickedRadioChannel2()
{
	if (m_pParent) {
		m_pParent->PostMessageA(TNM_CHANNEL_AUDIO, m_nIndex, 2);
	}
}

void CDlgChannel::OnSelchangeComboResolution()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	int nIndex = m_cComboResolurion.GetCurSel();
	if (m_pParent) {
		int nPercent = g_nPercent[nIndex];
		m_pParent->PostMessageA(TNM_CHANNEL_RESOLUTION, m_nIndex, nPercent);
	}
}

void CDlgChannel::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);

	CRect rc, rcc;

	GetClientRect(&rc);			
}


LRESULT CDlgChannel::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.

	return CDialogEx::WindowProc(message, wParam, lParam);
}

BOOL CDlgChannel::PreTranslateMessage(MSG* pMsg)
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.

	return CDialogEx::PreTranslateMessage(pMsg);
}

void CDlgChannel::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent == 1) {
		int nx, ny;
		CString strPos;

		nx = ny = 0;
		/*if (mp_tnm_channel) {						
			mp_tnm_channel->get_cur_mouse_pos(nx, ny);			
		}*/

		strPos.Format(_T("%d, %d"), nx, ny);
	//	m_cEdit_cur_pos.SetWindowTextA(strPos);
	}

	CDialogEx::OnTimer(nIDEvent);
}


void CDlgChannel::OnDestroy()
{
	CDialogEx::OnDestroy();

//	KillTimer(1);
}




