
// bitstreamReceiverDlg.cpp : ���� ����
//

#include "stdafx.h"
#include "bitstreamReceiver.h"
#include "bitstreamReceiverDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// ���� ���α׷� ������ ���Ǵ� CAboutDlg ��ȭ �����Դϴ�.

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// ��ȭ ���� �������Դϴ�.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �����Դϴ�.

// �����Դϴ�.
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CbitstreamReceiverDlg ��ȭ ����



CbitstreamReceiverDlg::CbitstreamReceiverDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_BITSTREAMRECEIVER_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	for (int i = 0; i < MAX_CHANNEL_COUNT; i++) {
		m_pChannel[i] = NULL;
		m_pViewer[i] = NULL;		
	}
	for (int i = 0; i < 2; i++) {
		mp_udp[i] = nullptr;
	}
}

void CbitstreamReceiverDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_COMBO_DST_IP, m_cComboDst);
	DDX_Control(pDX, IDC_COMBO_FWD, m_cComboFWD);
	for (int i = 0; i < 6; i++) {
		DDX_Control(pDX, IDC_BUTTON_CTRL1 + i, m_cBtnCtrl[i]);
	}
	DDX_Control(pDX, IDC_EDIT_EVENT_NAME, m_cEditEvtName);
	for (int i = 0; i < ENABLE_MAX_CHANNEL; i++) {
		DDX_Control(pDX, IDC_CHECK_CHANNEL1_1 + i, m_cCheckCh1[i]);
		DDX_Control(pDX, IDC_CHECK_CHANNEL2_1 + i, m_cCheckCh2[i]);
		DDX_Control(pDX, IDC_CHECK_CHANNEL3_1 + i, m_cCheck_file[i]);
	}
	DDX_Control(pDX, IDC_LIST_SAVE_FILE, m_cListFile);

	DDX_Control(pDX, IDC_EDIT_SELECT_FILE, m_cEditSelectFile);
}

BEGIN_MESSAGE_MAP(CbitstreamReceiverDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()	
	ON_WM_SIZE()
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_BUTTON_CTRL1, &CbitstreamReceiverDlg::OnBnClickedButtonCtrl1)
	ON_BN_CLICKED(IDC_BUTTON_CTRL2, &CbitstreamReceiverDlg::OnBnClickedButtonCtrl2)
	ON_BN_CLICKED(IDC_BUTTON_CTRL3, &CbitstreamReceiverDlg::OnBnClickedButtonCtrl3)
	ON_BN_CLICKED(IDC_BUTTON_CTRL4, &CbitstreamReceiverDlg::OnBnClickedButtonCtrl4)
	ON_BN_CLICKED(IDC_BUTTON_CTRL5, &CbitstreamReceiverDlg::OnBnClickedButtonCtrl5)
	ON_BN_CLICKED(IDC_BUTTON_CTRL6, &CbitstreamReceiverDlg::OnBnClickedButtonCtrl6)
	ON_BN_CLICKED(IDC_BUTTON_SAVE1, &CbitstreamReceiverDlg::OnBnClickedButtonSave1)
	ON_BN_CLICKED(IDC_BUTTON_STOP1, &CbitstreamReceiverDlg::OnBnClickedButtonStop1)
	ON_BN_CLICKED(IDC_BUTTON_SAVE2, &CbitstreamReceiverDlg::OnBnClickedButtonSave2)
	ON_BN_CLICKED(IDC_BUTTON_STOP2, &CbitstreamReceiverDlg::OnBnClickedButtonStop2)
	ON_NOTIFY(NM_CLICK, IDC_LIST_SAVE_FILE, &CbitstreamReceiverDlg::OnClickListSaveFile)
	ON_BN_CLICKED(IDC_BUTTON_CTRL7, &CbitstreamReceiverDlg::OnBnClickedButtonCtrl7)
END_MESSAGE_MAP()


// CbitstreamReceiverDlg �޽��� ó����
void CbitstreamReceiverDlg::OnDestroy()
{
	CDialogEx::OnDestroy();

	for (int i = 0; i < MAX_CHANNEL_COUNT; i++) {		
		SAFE_DELETE(m_pChannel[i]);
		SAFE_DELETE(m_pViewer[i]);	
	}

	for (int i = 0; i < 2; i++) {
		SAFE_DELETE(mp_udp[i]);
	}
	WSACleanup();
}

BOOL CbitstreamReceiverDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// �ý��� �޴��� "����..." �޴� �׸��� �߰��մϴ�.

	// IDM_ABOUTBOX�� �ý��� ��� ������ �־�� �մϴ�.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// �� ��ȭ ������ �������� �����մϴ�.  ���� ���α׷��� �� â�� ��ȭ ���ڰ� �ƴ� ��쿡��
	//  �����ӿ�ũ�� �� �۾��� �ڵ����� �����մϴ�.
	SetIcon(m_hIcon, TRUE);			// ū �������� �����մϴ�.
	SetIcon(m_hIcon, FALSE);		// ���� �������� �����մϴ�.
			
	// network
	WSADATA wsaData;
	int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		//	printf("WSAStartup failed with error: %d\n", iResult);
		_d(_T("WSAStartup failed with error: %d\n"), iResult);
		exit(-1);
	}

	m_ncodec = 0;	

	for (int i = 0; i < MAX_VIDEO_CHANNEL_COUNT; i++) {
		memset(&m_mux_cfg[i], 0, sizeof(mux_cfg_s));
		m_mux_cfg[i].output = 1;
		if (i >= 0 && i <= 3) {
			m_mux_cfg[i].vid.is_interlace = true;
		}
	}

	LoadConfigure();
	
	for (int i = 0; i < MAX_CHANNEL_COUNT; i++) {	
		channel_cfg_s ch_cfg;
		DefaultChInfo(i, &ch_cfg);

		memcpy(&ch_cfg.mux, &m_mux_cfg[i], sizeof(mux_cfg_s));

		m_pChannel[i] = new CDlgChannel(this, i);
		m_pChannel[i]->set_channel_info(&ch_cfg);
		m_pChannel[i]->Create(IDD_SUB_CHANNEL_DIALOG, this);	
		m_pChannel[i]->ShowWindow(SW_SHOW);

		m_pViewer[i] = new CDlgViewer(this, i);
		m_pViewer[i]->set_channel_info(&ch_cfg);
		m_pViewer[i]->Create(IDD_SUB_VIEWER, this);
		m_pViewer[i]->ShowWindow(SW_SHOW);
	}	

	m_cComboDst.AddString(m_conversion_cfg[0].ctr_channel.strIP);
	m_cComboDst.AddString(m_conversion_cfg[1].ctr_channel.strIP);
	m_cComboDst.SetCurSel(0);

	m_cComboFWD.AddString(_T("x2"));
	m_cComboFWD.AddString(_T("x3"));
	m_cComboFWD.AddString(_T("x4"));
	m_cComboFWD.AddString(_T("x5"));
	m_cComboFWD.SetCurSel(0);

	for (int i = 0; i < 2; i++) {
		mp_udp[i] = new CTnmComm();
		mp_udp[i]->StartRecv(m_conversion_cfg[i].ctr_channel.strIP, m_conversion_cfg[i].ctr_channel.nPort, m_conversion_cfg[i].nRecvPort);
	}

	m_cInOut.Create(CRect(10, 10, 20, 20), this, IDC_EDITOR_STC_INOUT);

	m_cListFile.SetExtendedStyle(LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);
	//> ���� �׸�� ��� �߰�
	m_cListFile.InsertColumn(0, _T("Num"), LVCFMT_CENTER, 40);
	m_cListFile.InsertColumn(1, _T("Name"), LVCFMT_LEFT, 400);
	m_cListFile.InsertColumn(2, _T("ch"), LVCFMT_CENTER, 0);
	m_cListFile.InsertColumn(3, _T("Duration"), LVCFMT_LEFT, 80);
		
	MoveWindow(0, 0, 1920, 950); 

	return TRUE;  // ��Ŀ���� ��Ʈ�ѿ� �������� ������ TRUE�� ��ȯ�մϴ�.
}

void CbitstreamReceiverDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// ��ȭ ���ڿ� �ּ�ȭ ���߸� �߰��� ��� �������� �׸�����
//  �Ʒ� �ڵ尡 �ʿ��մϴ�.  ����/�� ���� ����ϴ� MFC ���� ���α׷��� ��쿡��
//  �����ӿ�ũ���� �� �۾��� �ڵ����� �����մϴ�.

void CbitstreamReceiverDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // �׸��⸦ ���� ����̽� ���ؽ�Ʈ�Դϴ�.

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Ŭ���̾�Ʈ �簢������ �������� ����� ����ϴ�.
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// �������� �׸��ϴ�.
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// ����ڰ� �ּ�ȭ�� â�� ���� ���ȿ� Ŀ���� ǥ�õǵ��� �ý��ۿ���
//  �� �Լ��� ȣ���մϴ�.
HCURSOR CbitstreamReceiverDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CbitstreamReceiverDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);

	CRect rc, rcc, rcc2;
	GetClientRect(&rc);

	rcc.left = 5;
	rcc.top = 0;
	rcc.right = rcc.left + CHANNEL_WIDTH;
	rcc.bottom = rcc.top + CHANNEL_HEIGHT;

	rcc2.left = 5;
	rcc2.top = CHANNEL_HEIGHT + 1;
	rcc2.right = rcc2.left + CHANNEL_WIDTH;
	rcc2.bottom = rcc2.top + CHANNEL_HEIGHT;
	for (int i = 0; i < MAX_CHANNEL_COUNT; i++) {
		if (i <= 3) {
			if (m_pChannel[i]) {
				m_pChannel[i]->MoveWindow(&rcc);
			}
			rcc.OffsetRect(CHANNEL_WIDTH+1, 0);
		}
		else {
			if (m_pChannel[i]) {
				m_pChannel[i]->MoveWindow(&rcc2);
			}
			rcc2.OffsetRect(CHANNEL_WIDTH+1, 0);
		}				
	}	

	rcc.left = 5;
	rcc.top = 440;
	rcc.right = rcc.left + 850;
	rcc.bottom = rc.bottom - 5;
	if (m_cListFile) {
		m_cListFile.MoveWindow(&rcc);
	}

	rcc.left = 5;
	rcc.top = 375;
	rcc.right = rcc.left + 850;
	rcc.bottom = rcc.top + 15;
	if (m_cInOut) {
		m_cInOut.MoveWindow(&rcc);
	}
}

void CbitstreamReceiverDlg::DefaultChInfo(int nChannel, channel_cfg_s *pChCfg)
{
	memset(pChCfg, 0, sizeof(channel_cfg_s));
	pChCfg->mux.output = 1;

	pChCfg->mux.vid.codec = m_ncodec;
	pChCfg->channel_count = 1;

	if (nChannel >= 0 && nChannel <= 3) {
		//> 4Ch
		pChCfg->mux.vid.bitrate = 5000000;
		pChCfg->mux.vid.fps = 29.97;
		pChCfg->mux.vid.height = 240;
		pChCfg->mux.vid.width = 720;

		pChCfg->mux.vid.is_interlace = true;
	}
	else if (nChannel == 4) {
		//> panorama
		pChCfg->mux.vid.bitrate = 5000000;
		pChCfg->mux.vid.fps = 4;
		pChCfg->mux.vid.height = 1280;
		pChCfg->mux.vid.width = 1024;

		pChCfg->mux.vid.is_interlace = false;
	}
	else if (nChannel == 5) {
		//> focus
		pChCfg->mux.vid.bitrate = 5000000;
		pChCfg->mux.vid.fps = 4;
		pChCfg->mux.vid.height = 1024;
		pChCfg->mux.vid.width = 1024;

		pChCfg->mux.vid.is_interlace = false;
	}

	_stprintf(pChCfg->vid_ch[0].strIP, _T("%s"), m_conversion_cfg[0].vid_channel[nChannel].strIP);
	pChCfg->vid_ch[0].nPort = m_conversion_cfg[0].vid_channel[nChannel].nPort;

	_stprintf(pChCfg->vid_ch[1].strIP, _T("%s"), m_conversion_cfg[1].vid_channel[nChannel].strIP);
	pChCfg->vid_ch[1].nPort = m_conversion_cfg[1].vid_channel[nChannel].nPort;

	_stprintf(pChCfg->aud_ch[0].strIP, _T("%s"), m_conversion_cfg[0].aud_channel[0].strIP);
	pChCfg->aud_ch[0].nPort = m_conversion_cfg[0].aud_channel[0].nPort;

	_stprintf(pChCfg->aud_ch[1].strIP, _T("%s"), m_conversion_cfg[0].aud_channel[1].strIP);
	pChCfg->aud_ch[1].nPort = m_conversion_cfg[0].aud_channel[1].nPort;
}

void ReadConfigLine(FILE *fp, char *pCfg)
{
	int nCnt;
	bool bIsStart = false;

	while (1) {
		char c = fgetc(fp);
		if (c == '[') {
			nCnt = 0;
			bIsStart = true;
		}
		else if (c == ']') {
			pCfg[nCnt] = 0;
			//	fseek(fp, 1, SEEK_CUR);
			break;
		}
		else if (bIsStart) {
			pCfg[nCnt++] = c;
		}
	}
}
void CbitstreamReceiverDlg::LoadConfigure()
{
	CString strPath = _T("tnm_channel.ini");

	FILE* fp = _tfopen(strPath, _T("r"));
	if (fp) {
		char temp[256];

		_d(_T("[main] load config\n"));
		ReadConfigLine(fp, temp);
		m_ncodec = _ttoi(temp);

		for (int j = 0; j < 2; j++) {
			ReadConfigLine(fp, temp);
			_stprintf(m_conversion_cfg[j].ctr_channel.strIP, _T("%s"), temp);
			ReadConfigLine(fp, temp);
			m_conversion_cfg[j].ctr_channel.nPort = _ttoi(temp);
			ReadConfigLine(fp, temp);
			m_conversion_cfg[j].nRecvPort = _ttoi(temp);
			
			for (int i = 0; i < MAX_VIDEO_CHANNEL_COUNT; i++) {
				ReadConfigLine(fp, temp);
				_stprintf(m_conversion_cfg[j].vid_channel[i].strIP, _T("%s"), temp);
				ReadConfigLine(fp, temp);
				m_conversion_cfg[j].vid_channel[i].nPort = _ttoi(temp);
			}

			for (int i = 0; i < MAX_AUDIO_CHANNEL_COUNT; i++) {
				ReadConfigLine(fp, temp);
				_stprintf(m_conversion_cfg[j].aud_channel[i].strIP, _T("%s"), temp);
				ReadConfigLine(fp, temp);
				m_conversion_cfg[j].aud_channel[i].nPort = _ttoi(temp);
			}
		}

		for (int i = 0; i < MAX_VIDEO_CHANNEL_COUNT; i++) {
			int nTemp;
			ReadConfigLine(fp, temp);
			m_mux_cfg[i].vid.codec = _ttoi(temp);

			ReadConfigLine(fp, temp);
			nTemp = _ttoi(temp);
			m_mux_cfg[i].vid.bitrate = nTemp * 1024;

			ReadConfigLine(fp, temp);
			m_mux_cfg[i].vid.fps = _ttof(temp);

			ReadConfigLine(fp, temp);
			m_mux_cfg[i].vid.width = _ttoi(temp);

			ReadConfigLine(fp, temp);
			m_mux_cfg[i].vid.height = _ttoi(temp);
		}

		fclose(fp);
	}
	else {
		_stprintf(m_conversion_cfg[0].ctr_channel.strIP, _T("192.168.2.153"));
		m_conversion_cfg[0].ctr_channel.nPort = 2550;

		CString strIP;
		int nIP = 91;
		int nPort = 19262;
		for (int i = 0; i < MAX_VIDEO_CHANNEL_COUNT; i++) {
			_stprintf(m_conversion_cfg[0].vid_channel[i].strIP, _T("228.67.43.%d"), nIP);
			m_conversion_cfg[0].vid_channel[i].nPort = nPort;

			nPort++;
			nIP++;
		}

		nIP = 97;
		nPort = 19268;
		for (int i = 0; i < MAX_AUDIO_CHANNEL_COUNT; i++) {
			_stprintf(m_conversion_cfg[0].aud_channel[i].strIP, _T("228.67.43.%d"), nIP);
			m_conversion_cfg[0].aud_channel[i].nPort = nPort;

			nPort++;
			nIP++;
		}

		nIP = 91;
		nPort = 19262;
		for (int i = 0; i < MAX_VIDEO_CHANNEL_COUNT; i++) {
			_stprintf(m_conversion_cfg[1].vid_channel[i].strIP, _T("228.67.44.%d"), nIP);
			m_conversion_cfg[1].vid_channel[i].nPort = nPort;

			nPort++;
			nIP++;
		}

		nIP = 97;
		nPort = 19268;
		for (int i = 0; i < MAX_AUDIO_CHANNEL_COUNT; i++) {
			_stprintf(m_conversion_cfg[1].aud_channel[i].strIP, _T("228.67.44.%d"), nIP);
			m_conversion_cfg[1].aud_channel[i].nPort = nPort;

			nPort++;
			nIP++;
		}
	}
}


LRESULT CbitstreamReceiverDlg::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	int nChannel = (int)wParam;

	if (message == TNM_CHANNEL_CONNECT) {
		if (m_pViewer[nChannel]) {
			if (m_pViewer[nChannel]->connect_channel()) {
				m_pChannel[nChannel]->set_connected_status(true);
			}
			else {
				m_pChannel[nChannel]->set_connected_status(false);
			}
		}
	} 
	else if (message == TNM_CHANNEL_DISCONNECT) {
		if (m_pViewer[nChannel]) {
			m_pViewer[nChannel]->disconnect_channel();
			m_pChannel[nChannel]->set_connected_status(false);			
		}
	}
	else if (message == TNM_CHANNEL_PLAY) {
		if (m_pViewer[nChannel]) {
			m_pViewer[nChannel]->display_status(true);			
		}
	}
	else if (message == TNM_CHANNEL_STOP) {
		if (m_pViewer[nChannel]) {
			m_pViewer[nChannel]->display_status(false);
		}
	}
	else if (message == TNM_CHANNEL_CONTRAST) {
		int nPos = (int)lParam;
		if (m_pViewer[nChannel]) {
			m_pViewer[nChannel]->set_contrast(nPos);
		}
	}
	else if (message == TNM_CHANNEL_BRIGHTNESS) {
		int nPos = (int)lParam;
		if (m_pViewer[nChannel]) {
			m_pViewer[nChannel]->set_brightness(nPos);
		}
	}
	else if (message == TNM_CHANNEL_SETURATION) {
		int nPos = (int)lParam;
		if (m_pViewer[nChannel]) {
			m_pViewer[nChannel]->set_saturation(nPos);
		}
	}
	else if (message == TNM_CHANNEL_AUDIO) {
		int nStream = (int)lParam;
		if (m_pViewer[nChannel]) {
			m_pViewer[nChannel]->enable_audio(nStream);
		}
	}	
	else if (message == TNM_CHANNEL_RESOLUTION) {
		int nPercent = (int)lParam;
		if (m_pViewer[nChannel]) {			
			m_pViewer[nChannel]->set_viewer_resolution(nPercent);
		}
	}
	else if (message == WM_TNM_INOUT_THUMB_POS_CHANGED) {
		int nPos = m_cInOut.GetPos();
		move_file_pos(nPos);
	}

	return CDialogEx::WindowProc(message, wParam, lParam);
}

void CbitstreamReceiverDlg::add_file_list(save_file_meta_s meta)
{
	CString strIndex;
	//> ���� ��ϵ� ���� üũ
	int nIndex = m_cListFile.GetItemCount();

	//> ������ �߰�
	strIndex.Format(_T("%d"), nIndex);
	m_cListFile.InsertItem(nIndex, strIndex);

	//> �� ���鿡 �� �߰�
	strIndex.Format(_T("%s"), meta.strFileName);
	m_cListFile.SetItemText(nIndex, 1, strIndex);

	strIndex.Format(_T("%d"), meta.enable_ch);
	m_cListFile.SetItemText(nIndex, 2, strIndex);

	strIndex.Format(_T("%d"), meta.nDuration);
	m_cListFile.SetItemText(nIndex, 3, strIndex);
}

void CbitstreamReceiverDlg::select_file(save_file_meta_s meta)
{
	CString strText;

	strText.Format(_T("%s"), meta.strFileName);
	m_cEditSelectFile.SetWindowTextA(strText);

	for (int i = 0; i < ENABLE_MAX_CHANNEL; i++) {
		m_cCheck_file[i].SetCheck(0);

		if (meta.enable_ch & (0x01 << i)) {
			m_cCheck_file[i].SetCheck(1);
		}
	}
	
	//> ���õ� ������ ��ü �෹�̼� ������ Ÿ�ӹ��� �ִ� ��ġ������
	m_cInOut.SetRatio(100);
	m_cInOut.SetRange(meta.nDuration);
	m_cInOut.SetPos(0);


	memcpy(&m_selected_file_meta, &meta, sizeof(save_file_meta_s));
}

void CbitstreamReceiverDlg::OnBnClickedButtonCtrl1()
{
	// open
	//> 1. ����Ʈ �ʱ�ȭ
	m_cListFile.DeleteAllItems();

	//> 2. ������ �����κ��� ���ϸ���Ʈ ����. ������ ������, ��ȭ ���۽ð�, ����ð�, �෹�̼�, ��ȭ�� ä���������� ��Ÿ�� ����־����

	//> 3. ���� ������ŭ ����Ʈ�� �߰�
	save_file_meta_s meta;

	//> �׽�Ʈ�� ������ �ӽ÷� ����
	for (int i = 0; i < 20; i++) {
		memset(&meta, 0, sizeof(save_file_meta_s));
		CString strText;
		strText.Format(_T("test_file_%d"), i + 1);
		_stprintf(meta.strFileName, _T("%s"), strText);

		meta.enable_ch = 0x06;

		meta.nDuration = 200;

		add_file_list(meta);
	}
}
void CbitstreamReceiverDlg::OnBnClickedButtonCtrl2()
{
	// close
}
void CbitstreamReceiverDlg::OnBnClickedButtonCtrl3()
{
	// <<
}
void CbitstreamReceiverDlg::OnBnClickedButtonCtrl4()
{
	// play
}
void CbitstreamReceiverDlg::OnBnClickedButtonCtrl5()
{
	// stop
}
void CbitstreamReceiverDlg::OnBnClickedButtonCtrl6()
{
	// >>
}
void CbitstreamReceiverDlg::OnBnClickedButtonCtrl7()
{
	// delete
}
void CbitstreamReceiverDlg::move_file_pos(int nDstSec)
{
	//> ���� �̵���Ű��

	//> ��Ʈ�� ����
	m_cInOut.SetPos(nDstSec);
}
void CbitstreamReceiverDlg::OnBnClickedButtonSave1()
{
	// ��ñ�� ����
	int nIndex = m_cComboDst.GetCurSel();
	if (mp_udp[nIndex]) {
		byte enable = 0;
		for (int i = 0; i < ENABLE_MAX_CHANNEL; i++) {
			if (m_cCheckCh1[i].GetCheck()) {
				enable |= (0x01 << i);
			}
		}
		mp_udp[nIndex]->set_save_start(0, enable, m_mux_cfg, _T(""));
	}
}
void CbitstreamReceiverDlg::OnBnClickedButtonStop1()
{
	// ��ñ�� ����
	int nIndex = m_cComboDst.GetCurSel();
	if (mp_udp[nIndex]) {
		mp_udp[nIndex]->set_save_stop(0);
	}
}
void CbitstreamReceiverDlg::OnBnClickedButtonSave2()
{
	// �̺�Ʈ��� ����
	int nIndex = m_cComboDst.GetCurSel();
	if (mp_udp[nIndex]) {
		byte enable = 0;
		char strName[FILENAME_MAX];
		CString strTemp;

		m_cEditEvtName.GetWindowTextA(strTemp);
		_stprintf(strName, _T("%s"), strTemp);

		for (int i = 0; i < ENABLE_MAX_CHANNEL; i++) {
			if (m_cCheckCh2[i].GetCheck()) {
				enable |= (0x01 << i);
			}
		}
		mp_udp[nIndex]->set_save_start(1, enable, m_mux_cfg, strName);
	}
}
void CbitstreamReceiverDlg::OnBnClickedButtonStop2()
{
	// �̺�Ʈ ��� ����
	int nIndex = m_cComboDst.GetCurSel();
	if (mp_udp[nIndex]) {
		mp_udp[nIndex]->set_save_stop(1);
	}
}


void CbitstreamReceiverDlg::OnClickListSaveFile(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	int nItem = m_cListFile.GetItemCount();
	int nSelect = pNMItemActivate->iItem;

	if (nSelect >= 0 && nItem > nSelect) {
		//> ���õ� ������ ������ ä���
		CString strText;
		save_file_meta_s meta;
		memset(&meta, 0, sizeof(save_file_meta_s));

		_stprintf(meta.strFileName, _T("%s"), m_cListFile.GetItemText(nSelect, 1));
		
		strText.Format(_T("%s"), m_cListFile.GetItemText(nSelect, 2));
		meta.enable_ch = _ttoi(strText);

		strText.Format(_T("%s"), m_cListFile.GetItemText(nSelect, 3));
		meta.nDuration = _ttoi(strText);

		//> ������ ���Ϸ� ó��
		select_file(meta);
	}
	*pResult = 0;
}



