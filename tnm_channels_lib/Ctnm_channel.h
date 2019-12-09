#pragma once
#include "TnmGlobal.h"

#ifdef TNMCHANNELSLIB_EXPORTS
#define TNM __declspec(dllexport)
#else 
#define TNM __declspec(dllimport)
#endif

extern "C"

class Ctnm_channel
{
public:
	TNM Ctnm_channel(int nIndex);
	TNM ~Ctnm_channel();

	/* viewer */
	TNM bool create_viewer(CWnd* pParent, const RECT &rect, UINT uID);
	TNM void resize_viewer(const RECT &rect);
	TNM void delete_viewer();

	/* ȭ�鿡 ���õǴ� ���� �� crop�� ����� �ȼ� ��. ���� �ػ� ����. ������ �ʼ���ŭ crop�ǰ� ���� �̹����� ���õ�*/
	TNM void set_crop_value(uint32_t left, uint32_t top, uint32_t right, uint32_t bottom);

	TNM void get_cur_mouse_pos(int &nx, int &ny);

	/* ctrl */
	TNM bool start_channel(channel_cfg_s ch_cfg);
	TNM void stop_channel();

	TNM void set_display(bool bdisplay);

	/* filter */
	TNM void set_contrast(int nLevel);		/* -4.9 ~ 5.0. default: 1*/
	TNM void set_brightness(int nLevle);		/* -1.0 ~ 0.9. default: 0 */
	TNM void set_saturation(int nLevle);		/* 0.0 ~ 3.0. dafault: 1*/	

	TNM void enable_audio(int nChannel);

private:
	void* mp_channel;
};

