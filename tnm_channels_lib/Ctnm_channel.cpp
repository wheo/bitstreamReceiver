#include "stdafx.h"
#include "Ctnm_channel.h"

#include "VideoChannel.h"

int g_nIndex = 0;

Ctnm_channel::Ctnm_channel(int nIndex)
{
	mp_channel = new CVideoChannel();	

	g_nIndex = nIndex;
}

Ctnm_channel::~Ctnm_channel()
{
	_d(_T("[lib] delete lib\n"));
	if (mp_channel) {
		CVideoChannel* pChannel = (CVideoChannel *)mp_channel;
		delete pChannel;
		mp_channel = NULL;
	}	
}

void Ctnm_channel::enable_audio(int nChannel)
{
	if (mp_channel) {
		CVideoChannel* pChannel = (CVideoChannel *)mp_channel;
		if (nChannel < 0) {
			nChannel = 0;
		}
		if (nChannel >= MAX_AUDIO_CHANNEL_COUNT) {
			nChannel = MAX_AUDIO_CHANNEL_COUNT - 1;
		}
		pChannel->enable_audio(nChannel);
	}
}

void Ctnm_channel::get_cur_mouse_pos(int &nx, int &ny)
{
	nx = 0;
	ny = 0;
	if (mp_channel) {
		CVideoChannel* pChannel = (CVideoChannel *)mp_channel;
		pChannel->get_cur_mouse_pos(nx, ny);
	}
}

void Ctnm_channel::set_crop_value(uint32_t left, uint32_t top, uint32_t right, uint32_t bottom)
{
	if (mp_channel) {
		CVideoChannel* pChannel = (CVideoChannel *)mp_channel;
		pChannel->set_crop_value(left, top, right, bottom);
	}
}
bool Ctnm_channel::start_channel(channel_cfg_s ch_cfg)
{
	if (mp_channel) {
		CVideoChannel* pChannel = (CVideoChannel *)mp_channel;
		return pChannel->start_channel(ch_cfg);
	}
	
	return false;
}
void Ctnm_channel::stop_channel()
{	
	if (mp_channel) {
		CVideoChannel* pChannel = (CVideoChannel *)mp_channel;
		pChannel->stop_channel();
	}
}

void Ctnm_channel::set_display(bool bdisplay)
{
	if (mp_channel) {
		CVideoChannel* pChannel = (CVideoChannel *)mp_channel;
		pChannel->set_display(bdisplay);
	}
}

void Ctnm_channel::set_contrast(int nLevle)
{
	if (mp_channel) {
		CVideoChannel* pChannel = (CVideoChannel *)mp_channel;
		double fValue = 0.;
		// -1000.0 ~ 1000.0; default : 1
		fValue = (nLevle*0.1) - 4.0;

		pChannel->set_contrast(fValue);
	}
}
void Ctnm_channel::set_brightness(int nLevle)
{
	if (mp_channel) {
		CVideoChannel* pChannel = (CVideoChannel *)mp_channel;
		double fValue = 0.;
		// 20
					// -1.0 ~ 1.0; default : 0
		fValue = -1 + (nLevle * 0.1);
		pChannel->set_brightness(fValue);
	}
}
void Ctnm_channel::set_saturation(int nLevle)
{
	if (mp_channel) {
		CVideoChannel* pChannel = (CVideoChannel *)mp_channel;
		double fValue = 0.;
		// 20
					// 0.0 ~ 3.0; default : 1
		fValue = 0.0 + (nLevle * 0.1);
		pChannel->set_saturation(fValue);
	}
}

/* ºä¾î »ý¼º */
bool Ctnm_channel::create_viewer(CWnd* pParent, const RECT &rect, UINT uID)
{
	if (mp_channel) {
		CVideoChannel* pChannel = (CVideoChannel *)mp_channel;
		return pChannel->create_viewer(pParent, rect, uID);
	}
	return false;	
}
void Ctnm_channel::delete_viewer()
{
	if (mp_channel) {
		CVideoChannel* pChannel = (CVideoChannel *)mp_channel;
		pChannel->delete_viewer();
	}
}
void Ctnm_channel::resize_viewer(const RECT &rect)
{
	if (mp_channel) {
		CVideoChannel* pChannel = (CVideoChannel *)mp_channel;
		pChannel->resize_viewer(rect);
	}	
}
