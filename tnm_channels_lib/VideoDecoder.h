#pragma once
#include "yadif.h"
#include "TnmVideoEQFilter.h"

class CVideoDecoder
{
public:
	CVideoDecoder();
	~CVideoDecoder();

	bool create_decoder(int nCodecID, int nSrcWidth, int nSrcHeight, bool bIsInterlace = false);
	
	int decode_deinterlace_frame(uint8_t *stream_in, int sz_stream, uint8_t *frame_out);

	void set_crop_value(uint32_t left, uint32_t top, uint32_t right, uint32_t bottom);

	void set_contrast(double fValue);
	void set_brightness(double fValue);
	void set_saturation(double fValue);	

	void get_image_size(int &nWidth, int &nHeight);

private:
	Preset m_preset;

	tagVideoFilter m_videoFilter;

	AVCodecContext *m_pDec;

	struct SwsContext *m_pSwsCtx = nullptr;
	yadif * m_pYadif = nullptr;
	CTnmVideoEQFilter *m_pEQFilter;

	AVFrame *m_pFrameIn;	
	AVFrame *m_pFrameOut;
	AVFrame *m_pFilterIn;
	AVFrame *m_pFiltFrame;

	uint8_t *m_src_data[4], *m_dst_data[4];
	int m_src_line[4], m_dst_line[4];

	int m_image_width;
	int m_image_height;
};
