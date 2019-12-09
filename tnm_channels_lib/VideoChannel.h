#pragma once
#include "VideoReceiver.h"
#include "VideoViewer.h"
#include "VideoDecoder.h"

#include "MyRecvAudio.h"
//#include "VideoRTSP.h"


class CVideoChannel : public CVideoThread
{
	/*{MEMBER_VARIABLE*/

public:
	CVideoChannel();
	~CVideoChannel();
		
	/* 뷰어 제어 */
	bool create_viewer(CWnd *pParent, RECT rect, UINT uID);		
	void delete_viewer();	/* 뷰어 제거 */
	void resize_viewer(RECT rect);

	void set_crop_value(uint32_t left, uint32_t top, uint32_t right, uint32_t bottom);

	void get_cur_mouse_pos(int &nx, int &ny);
		
	/* 데이터 생성 */	
	bool start_channel(channel_cfg_s ch_cfg);	
	void stop_channel();		/* 데이터 제거 */

	void set_display(bool bdisplay) { m_viewing = bdisplay; };

	void set_contrast(double fValue);
	void set_brightness(double fValue);
	void set_saturation(double fValue);	

	void enable_audio(int nChannel);

protected:
	void Run();

private:
	CWnd *m_pParent;	   

	int m_nCropLeft;
	int m_nCropTop;
	int m_nCropRight;
	int m_nCropBottom;

	bool m_resize;

	bool m_viewing;

	CRect m_rcViewer;
	UINT m_nID;

	CVideoViewer *mp_viewer;	/* 화면 전시 클래스 */
	CVideoReceiver *mp_receiver[2];	/* 비디오 프레임 생성. 파일버전, RTSP 수신버전 생성 예정 */	
	CVideoDecoder	*mp_decoder;	//> FFMpeg버전, Intel SDK 버전 생성 예정
	int m_reveiver_index;
	
	CMyRecvAudio *mp_audio;
	byte *m_pVideoData;
	byte *m_pRGBData;

	channel_cfg_s m_ch_cfg;	

	CCriticalSection m_csDraw;
};

