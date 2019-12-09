
// bitstreamReceiverDlg.h : 헤더 파일
//

#pragma once

#include "CDlgChannel.h"
#include "CDlgViewer.h"
#include "MyInOutSelector.h"
#include "TnmComm.h"

#define ENABLE_MAX_CHANNEL 7

// CbitstreamReceiverDlg 대화 상자
class CbitstreamReceiverDlg : public CDialogEx
{
// 생성입니다.
public:
	CbitstreamReceiverDlg(CWnd* pParent = NULL);	// 표준 생성자입니다.

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_BITSTREAMRECEIVER_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 지원입니다.


// 구현입니다.
protected:
	HICON m_hIcon;

	// 생성된 메시지 맵 함수
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();		
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnDestroy();
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	afx_msg void OnBnClickedButtonCtrl1();
	afx_msg void OnBnClickedButtonCtrl2();
	afx_msg void OnBnClickedButtonCtrl3();
	afx_msg void OnBnClickedButtonCtrl4();
	afx_msg void OnBnClickedButtonCtrl5();
	afx_msg void OnBnClickedButtonCtrl6();
	afx_msg void OnBnClickedButtonCtrl7();
	afx_msg void OnBnClickedButtonSave1();
	afx_msg void OnBnClickedButtonStop1();
	afx_msg void OnBnClickedButtonSave2();
	afx_msg void OnBnClickedButtonStop2();
	afx_msg void OnClickListSaveFile(NMHDR *pNMHDR, LRESULT *pResult);
	DECLARE_MESSAGE_MAP()

private:
	void LoadConfigure();

	void DefaultChInfo(int nChannel, channel_cfg_s *pChCfg);

	void add_file_list(save_file_meta_s meta);

	void select_file(save_file_meta_s meta);

	void move_file_pos(int nDstSec);

private:
	CDlgChannel *m_pChannel[MAX_CHANNEL_COUNT];
	CDlgViewer *m_pViewer[MAX_CHANNEL_COUNT];

	CComboBox m_cComboDst;
	CComboBox m_cComboFWD;
	CButton m_cBtnCtrl[6];	

	CButton m_cCheckCh1[ENABLE_MAX_CHANNEL];
	CButton m_cCheckCh2[ENABLE_MAX_CHANNEL];

	CEdit m_cEditEvtName;

	CListCtrl m_cListFile;
	CButton m_cCheck_file[ENABLE_MAX_CHANNEL];

	CEdit m_cEditSelectFile;

	CMyInOutSelector m_cInOut;
	
private:
	conversion_cfg_s m_conversion_cfg[2];

	CTnmComm *mp_udp[2];

	int m_ncodec;

	mux_cfg_s m_mux_cfg[MAX_VIDEO_CHANNEL_COUNT];	

	save_file_meta_s m_selected_file_meta;
};
