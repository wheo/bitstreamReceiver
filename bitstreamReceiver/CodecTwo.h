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
	pPreset : Preset 포인터, (width, height, bitrate, quality 설정)
	@return
	bool : true 면 생성 성공, false 면 실패
	*/
	bool Create(Preset *pPreset, bool bIsJ2K = false);

	bool Create(Preset *pPreset, int selectCodec = 0, bool bIsEnc = true, int IsDouble = 1);
		
	bool CreateDecoder(int nCodecID, int nSrcWidth, int nSrcHeight, bool bIsInterlace = false);

	void SetContrast(double fValue);
	void SetBrightness(double fValue);
	void SetGamma_r(double fValue);
	void SetGamma_g(double fValue);
	void SetGamma_b(double fValue);
		
	/** 패킷을 디코딩한다.
	@args
	stream_in : 비트스트림 버퍼 포인터, NULL 이면 flush
	sz_stream : 비트스트림 크기
	frame_out : 프레임 버퍼 포인터
	@return
	int : 프레임 크기, 0 이면 성공이나 준비안됨, 0보다 작으면 에러
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

