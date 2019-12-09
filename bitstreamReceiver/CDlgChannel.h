#pragma once
// CDlgChannel 대화 상자

class CDlgChannel : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgChannel)

public:
	CDlgChannel(CWnd* pParent = nullptr, int nIndex = 1);   // 표준 생성자입니다.
	virtual ~CDlgChannel();

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_SUB_CHANNEL_DIALOG };
#endif

	void set_channel_info(channel_cfg_s *pCfg);

	void set_connected_status(bool IsConnected);


protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.	
	virtual BOOL OnInitDialog();
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnBnClickedChannelButtonConnect();
	afx_msg void OnBnClickedChannelButtonConnect2();
	afx_msg void OnBnClickedChannelButtonStart();
	afx_msg void OnBnClickedChannelButtonPause();	
	afx_msg void OnBnClickedRadioChannel1();
	afx_msg void OnBnClickedRadioChannel2();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnDestroy();
	afx_msg void OnSelchangeComboResolution();

	DECLARE_MESSAGE_MAP()


	
private:	
	int m_nIndex;
	CWnd* m_pParent;
	
	channel_cfg_s m_ch_cfg;
		
private:
	CEdit m_cEditIP;	
	CButton m_cBtnConnect[2];
	
	
	CButton m_cBtnPlay;	
	CButton m_cBtnPause;	

	CComboBox m_cComboResolurion;
	
};
