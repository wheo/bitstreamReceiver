#pragma once
#include "Ctnm_channel.h"

// CDlgViewer 대화 상자

class CDlgViewer : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgViewer)

public:
	CDlgViewer(CWnd* pParent = nullptr, int nIndex = 0);   // 표준 생성자입니다.
	virtual ~CDlgViewer();

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_SUB_VIEWER };
#endif

	void set_channel_info(channel_cfg_s *pCfg);

	void display_status(bool bPlay);

	void set_contrast(int nPos);
	void set_brightness(int nPos);
	void set_saturation(int nPos);

	void enable_audio(int nStream);

	void set_crop_value(int nLeft, int nTop, int nRight, int nBottom);

	void set_viewer_resolution(int nPercent);

	bool connect_channel();
	void disconnect_channel();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.
	virtual BOOL OnInitDialog();
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	afx_msg void OnDestroy();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnSize(UINT nType, int cx, int cy);	
	DECLARE_MESSAGE_MAP()

private:
	void resize_viewer(int percent);

private:
	int m_nIndex;

	channel_cfg_s m_ch_cfg;

	Ctnm_channel *mp_tnm_channel;

public:
	
};
