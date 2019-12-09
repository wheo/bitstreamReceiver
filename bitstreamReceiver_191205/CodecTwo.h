#pragma once

#include "yadif.h"
#include "TnmVideoEQFilter.h"

struct tagVideoFilter
{
	double fContrast;
	double fBrightness;
	double fGamma_r;
	double fGamma_g;
	double fGamma_b;
};

class CCodecTwo {
public:
	CCodecTwo();
	~CCodecTwo();

	/**
	@args
	pPreset : Preset ������, (width, height, bitrate, quality ����)
	@return
	bool : true �� ���� ����, false �� ����
	*/
	bool Create(Preset *pPreset, bool bIsJ2K = false);

	bool Create(Preset *pPreset, int selectCodec = 0, bool bIsEnc = true, int IsDouble = 1);
		
	bool CreateDecoder(int nCodecID, int nSrcWidth, int nSrcHeight, bool bIsInterlace = false);

	void SetContrast(double fValue);
	void SetBrightness(double fValue);
	void SetGamma_r(double fValue);
	void SetGamma_g(double fValue);
	void SetGamma_b(double fValue);
		
	/** ��Ŷ�� ���ڵ��Ѵ�.
	@args
	stream_in : ��Ʈ��Ʈ�� ���� ������, NULL �̸� flush
	sz_stream : ��Ʈ��Ʈ�� ũ��
	frame_out : ������ ���� ������
	@return
	int : ������ ũ��, 0 �̸� �����̳� �غ�ȵ�, 0���� ������ ����
	*/
	int decode_frame(uint8_t *stream_in, int sz_stream, uint8_t *frame_out);
	int decode_deinterlace_frame3(uint8_t *stream_in, int sz_stream, uint8_t *frame_out);
	
	
private:
	HRESULT YV12toARGB(uint8_t* Frame);
	HRESULT YUV420PtoARGBx2(uint8_t * Frame);
	HRESULT YUV420PtoARGB(uint8_t* Frame);
	HRESULT NV12toARGB(uint8_t* Frame);
	HRESULT UYVYtoARGB(uint8_t* Frame);
	HRESULT NV12toARGB(uint8_t* source, uint8_t* dst);
	HRESULT NV12toARGB(uint8_t* source, uint8_t* dst, unsigned int width, unsigned int height);

protected:
	unsigned int i_frame;
	int p_frame;
	int b_frame;
	int undefined_frame;

	int encoded_total_pkt_size;
	int encoded_pkt_count;
	   
	int m_nFrameCount;

	Preset m_preset;

	AVCodecContext *m_pEnc;
	AVCodecContext *m_pDec;

	AVFrame *m_pFrameIn;
	AVFrame *m_pFrameOut;
	AVFrame *m_pFiltFrame;
	AVFrame *m_pFilterIn;
	
	yadif * m_pYadif = nullptr;
	CTnmVideoEQFilter *m_pEQFilter;

	tagVideoFilter m_videoFilter;

	struct SwsContext *m_pSwsCtx = nullptr;
	uint8_t *m_src_data[4], *m_dst_data[4];
	int m_src_line[4], m_dst_line[4];
};

