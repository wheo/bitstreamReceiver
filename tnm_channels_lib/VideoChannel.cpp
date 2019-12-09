#include "stdafx.h"
#include "VideoChannel.h"


CVideoChannel::CVideoChannel()
{
	mp_viewer = NULL;
	mp_receiver[0] = NULL;
	mp_receiver[1] = NULL;

	mp_audio = NULL;
		
	mp_decoder = NULL;
		
	m_pVideoData = new byte[MAX_IMAGE_WIDTH * MAX_IMAGE_HEIGHT * 4];	
	m_pRGBData = new byte[MAX_IMAGE_WIDTH * MAX_IMAGE_HEIGHT * 4];

	m_resize = false;
	m_viewing = true;

	m_reveiver_index = 1;

	m_nCropLeft = 0;
	m_nCropTop = 0;
	m_nCropRight = 0;
	m_nCropBottom = 0;
}

CVideoChannel::~CVideoChannel()
{	
	stop_channel();

	delete_viewer();	

	delete[] m_pVideoData;
	delete[] m_pRGBData;
}

void CVideoChannel::get_cur_mouse_pos(int &nx, int &ny)
{
	nx = 0;
	ny = 0;
	if (mp_viewer) {
		mp_viewer->get_cur_mouse_pos(nx, ny);
	}
}

void CVideoChannel::set_contrast(double fValue)
{
	if (mp_decoder) {
		mp_decoder->set_contrast(fValue);
	}
}
void CVideoChannel::set_brightness(double fValue)
{
	if (mp_decoder) {
		mp_decoder->set_brightness(fValue);
	}
}
void CVideoChannel::set_saturation(double fValue)
{
	if (mp_decoder) {
		mp_decoder->set_saturation(fValue);
	}
}

void CVideoChannel::set_crop_value(uint32_t left, uint32_t top, uint32_t right, uint32_t bottom)
{
	m_nCropLeft = left;
	m_nCropTop = top;
	m_nCropRight = right;
	m_nCropBottom = bottom;	

	if (mp_decoder) {
		mp_decoder->set_crop_value(m_nCropLeft, m_nCropTop, m_nCropRight, m_nCropBottom);
	}
}

void CVideoChannel::enable_audio(int nChannel)
{
	if (mp_audio) {
		mp_audio->enable_audio(nChannel);
	}	
}

bool CVideoChannel::start_channel(channel_cfg_s ch_cfg)
{
	stop_channel();

	memcpy(&m_ch_cfg, &ch_cfg, sizeof(channel_cfg_s));

	mp_decoder = new CVideoDecoder();
	if (mp_decoder->create_decoder(m_ch_cfg.mux.vid.codec, m_ch_cfg.mux.vid.width, m_ch_cfg.mux.vid.height, m_ch_cfg.mux.vid.is_interlace) == false) {
		SAFE_DELETE(mp_decoder);
	}

	for (int i = 0; i < m_ch_cfg.channel_count; i++) {
#if 0
		mp_receiver[i] = new CVideoReceiver(i);
		mp_receiver[i]->StartRecv(m_ch_cfg.vid_ch[i].strIP, _T(""), m_ch_cfg.vid_ch[i].nPort);
#else
		/* lcj */
		if (i == 0) {
			mp_receiver[i] = new CVideoReceiver(i);
			mp_receiver[i]->StartRecv(m_ch_cfg.vid_ch[0].strIP, m_ch_cfg.vid_ch[1].strIP, _T(""), m_ch_cfg.vid_ch[i].nPort);
		}
		/* lcj */
#endif
	}

	CString strAudio;
	strAudio.Format(_T("%s"), m_ch_cfg.aud_ch[0].strIP);
	if (strAudio.IsEmpty() == false) {
		mp_audio = new CMyRecvAudio();
		mp_audio->set_channel_info(m_ch_cfg.aud_ch[0], m_ch_cfg.aud_ch[1]);
	//	mp_audio->enable_audio(0);
	}

	Start();

	return true;
}
void CVideoChannel::stop_channel()
{
	Terminate();

	for (int i = 0; i < 2; i++) {
		SAFE_DELETE(mp_receiver[i]);
	}

	SAFE_DELETE(mp_decoder);

	SAFE_DELETE(mp_audio);
}

void CVideoChannel::Run()
{		
	//memset(m_pData, 3, MAX_WIDTH * MAX_HEIGHT * 4);
	
	CVideoReceiver *p_cur_receiver = NULL;
	while (!m_bExit) {	
		if (!p_cur_receiver) {
			//> check
			if (mp_receiver[0]) {
				//	_d(_T("[ch.%d] recv size : %d\n"), 0, mp_receiver[0]->get_recv_data());
			}
			if (mp_receiver[1]) {
				//	_d(_T("[ch.%d] recv size : %d\n"), 1, mp_receiver[1]->get_recv_data());
			}

			/*if (m_reveiver_index == 0) {
				p_cur_receiver = mp_receiver[1];
				m_reveiver_index = 1;
			}
			else {
				p_cur_receiver = mp_receiver[0];
				m_reveiver_index = 0;
			}*/
			p_cur_receiver = mp_receiver[0];
			m_reveiver_index = 0;
		}

		if (p_cur_receiver->is_connected() == false) {
			p_cur_receiver = NULL;
			Sleep(1);
			continue;
		}

		int nSize = p_cur_receiver->GetImageData(m_pVideoData);
		if (nSize > 0) {
			int nWidth, nHeight;
			bool decoded = false;
			if (mp_decoder) {					
				if (mp_decoder->decode_deinterlace_frame(m_pVideoData, nSize, m_pRGBData) > 0) {
					mp_decoder->get_image_size(nWidth, nHeight);
					if (m_ch_cfg.mux.vid.is_interlace == false) {		
						//> 감시
						detection_Info_s detection;
						detection.nTargetCount = 2;

						detection.nx[0] = 150;
						detection.ny[0] = 150;

						detection.nWidth[0] = 50;
						detection.nHeight[0] = 50;

						detection.nx[1] = 250;
						detection.ny[1] = 100;

						detection.nWidth[1] = 50;
						detection.nHeight[1] = 50;
						if (mp_viewer) {
							mp_viewer->SetTargetInfo(0, &detection);
						}
					}
					decoded = true;
				//	_d(_T("[ch.%d] decoded channel. %d\n"), m_reveiver_index, nSize);

					//> 수신정보
					CString strCurIP;
					strCurIP.Format(_T("%s"), p_cur_receiver->get_cur_connected_ip());
					if (mp_viewer) {
						mp_viewer->set_cur_connected_ip(strCurIP);
					}

					int nFPS = 0;
					nFPS = p_cur_receiver->get_recv_frame();
					if (mp_viewer) {
						mp_viewer->set_recv_frame_count(nFPS);
					}
				}
			}				
			if (mp_viewer && m_viewing && (m_resize == false)) {
				if (decoded) {
					m_csDraw.Lock();
					mp_viewer->DrawYUV420(nWidth, nHeight, m_pRGBData);
					m_csDraw.Unlock();
				}
			}
		}				

		Sleep(1);
	}		
}

/****************************************************************
파일명        : CalculationDlg-IRSG.cpp
최초 작성일   : 2018/02/07
마지막 수정일 : 2018/02/07
최초 작성자   : 형상관리팀 오찬화
마지막 수정자 : 형상관리팀 오찬화
클래스명      : CCalculationDlg
함수명        : Create
기능          : CCalculationDlg 메시지 처리기
입력값        : const RECT &rect, CWnd *pParentWnd
출력값        : BOOL
비고          :
******************************************************************/
bool CVideoChannel::create_viewer(CWnd *pParent, RECT rect, UINT uID)
{
	/**
	1 해당 채널에 대한 직송 응답전문을 만든다.
	1.1 직송시험전문인경우 : 전문번호는 0x00
	1.2 직송제원전문인경우 : 전문번호는 수신한 전문의 전문번호와 같다.
	1.3 암호를 비교하여 같지 않으면 응답 내용은 xxx 이다.
	2 응답전문의 내용을 해당 채널 메시지 큐에 넣는다.
	**/

	bool bResult = true;
	delete_viewer();

	m_resize = false;

	m_pParent = pParent;
	m_rcViewer.SetRect(rect.left, rect.top, rect.right, rect.bottom);
	m_nID = uID;

	mp_viewer = new CVideoViewer();
	bResult = mp_viewer->CreateViewer(m_rcViewer, m_pParent, m_nID);

	return bResult;
}
void CVideoChannel::delete_viewer()
{
	stop_channel();

	SAFE_DELETE(mp_viewer);
}
void CVideoChannel::resize_viewer(RECT rect)
{
	if (mp_viewer) {
		m_resize = true;

		CRect rcc;		
		rcc.top = rect.top;
		rcc.left = rect.left;
		rcc.bottom = rect.bottom;
		rcc.right = rect.right;

		if (rcc.Width() % 2) {
			rcc.right -= 1;
		}
		if (rcc.Height() % 2) {
			rcc.bottom -= 1;
		}

		m_csDraw.Lock();
		mp_viewer->MoveWindow(&rcc);
		mp_viewer->ResizeViewer();
		m_csDraw.Unlock();
		m_resize = false;
	}
}