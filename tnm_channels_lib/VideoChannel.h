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
		
	/* ��� ���� */
	bool create_viewer(CWnd *pParent, RECT rect, UINT uID);		
	void delete_viewer();	/* ��� ���� */
	void resize_viewer(RECT rect);

	void set_crop_value(uint32_t left, uint32_t top, uint32_t right, uint32_t bottom);

	void get_cur_mouse_pos(int &nx, int &ny);
		
	/* ������ ���� */	
	bool start_channel(channel_cfg_s ch_cfg);	
	void stop_channel();		/* ������ ���� */

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

	CVideoViewer *mp_viewer;	/* ȭ�� ���� Ŭ���� */
	CVideoReceiver *mp_receiver[2];	/* ���� ������ ����. ���Ϲ���, RTSP ���Ź��� ���� ���� */	
	CVideoDecoder	*mp_decoder;	//> FFMpeg����, Intel SDK ���� ���� ����
	int m_reveiver_index;
	
	CMyRecvAudio *mp_audio;
	byte *m_pVideoData;
	byte *m_pRGBData;

	channel_cfg_s m_ch_cfg;	

	CCriticalSection m_csDraw;
};

