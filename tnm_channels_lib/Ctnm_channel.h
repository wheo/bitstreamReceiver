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

	/* 화면에 전시되는 영역 중 crop이 적용될 픽셀 값. 원본 해상도 기준. 지정된 필셀만큼 crop되고 남은 이미지가 전시됨*/
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

