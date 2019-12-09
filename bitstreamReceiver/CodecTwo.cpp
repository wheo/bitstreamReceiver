#include "stdafx.h"
#include "CodecTwo.h"

// Jpeg2000 + HEVC codec class

CCodecTwo::CCodecTwo() {
	m_preset.width = 352;
	m_preset.height = 288;
	m_preset.bitrate = 512000;
	m_preset.quality = 0;

	m_nFrameCount = 0;

	m_pEnc = NULL;
	m_pDec = NULL;

	m_pYadif = NULL;
	m_pEQFilter = NULL;

	m_pFrameIn = NULL;
	m_pFrameOut = NULL;	
	m_pFilterIn = NULL;
	m_pFiltFrame = NULL;
	
	i_frame = 0;
	p_frame = 0;
	b_frame = 0;
	undefined_frame = 0;

	encoded_total_pkt_size = 0;
	encoded_pkt_count = 0;

	m_videoFilter.fContrast = 1.;
	m_videoFilter.fBrightness = 0.;
	m_videoFilter.fGamma_r = 1.;
	m_videoFilter.fGamma_g = 1.;
	m_videoFilter.fGamma_b = 1.;
}

CCodecTwo::~CCodecTwo() {
	if (m_pEnc) {
		avcodec_free_context(&m_pEnc);
		m_pEnc = NULL;
	}
	if (m_pDec) {
		avcodec_free_context(&m_pDec);
		m_pDec = NULL;
	}
	
	if (m_pFrameIn) {
		av_frame_unref(m_pFrameIn);
		av_frame_free(&m_pFrameIn);
	}
	if (m_pFrameOut) {
		av_frame_unref(m_pFrameOut);
		av_frame_free(&m_pFrameOut);
	}
	if (m_pFilterIn) {
		av_frame_unref(m_pFilterIn);
		av_frame_free(&m_pFilterIn);
	}
	if (m_pFiltFrame) {
		av_frame_unref(m_pFiltFrame);
		av_frame_free(&m_pFiltFrame);
	}

	SAFE_DELETE(m_pYadif);
	SAFE_DELETE(m_pEQFilter);
}

void CCodecTwo::SetContrast(double fValue)
{
	m_videoFilter.fContrast = fValue;
}
void CCodecTwo::SetBrightness(double fValue)
{
	m_videoFilter.fBrightness = fValue;
}
void CCodecTwo::SetGamma_r(double fValue)
{
	m_videoFilter.fGamma_r = fValue;
}
void CCodecTwo::SetGamma_g(double fValue)
{
	m_videoFilter.fGamma_g = fValue;
}
void CCodecTwo::SetGamma_b(double fValue)
{
	m_videoFilter.fGamma_b = fValue;
}
bool CCodecTwo::CreateDecoder(int nCodecID, int nSrcWidth, int nSrcHeight, bool bIsInterlace)
{
	int ret = 0;

	AVCodec *codec = NULL;
	AVDictionary *opts = NULL;

	m_preset.width = nSrcWidth;
	m_preset.height = nSrcHeight;
		
	if (nCodecID == TNM_CODEC_ID_H264) {		
#if ENABLE_HW
		codec = avcodec_find_decoder_by_name("h264_cuvid");
#else
		codec = avcodec_find_decoder(AV_CODEC_ID_H264);
#endif		
	}
	else if (nCodecID == TNM_CODEC_ID_JPEG) {
		codec = avcodec_find_decoder(AV_CODEC_ID_JPEG2000);
	}
	else if (nCodecID == TNM_CODEC_ID_HEVC) {		
#if ENABLE_HW
		codec = avcodec_find_decoder_by_name("hevc_cuvid");		
#else
		codec = avcodec_find_decoder(AV_CODEC_ID_HEVC);
#endif
	}
	if (!codec) {
		printf("Failed to find decoder\n");
		goto init_error;
	}

	m_pDec = avcodec_alloc_context3(codec);
	if (!m_pDec) {
		printf("Failed to allocate decoder context\n");
		goto init_error;
	}

	ret = avcodec_open2(m_pDec, codec, NULL);
	//printf("[dec] bit_rate(%d)\n", m_pDec->bit_rate);
	if (ret < 0) {
		printf("Failed to open decoder (%d)\n", ret);
		goto init_error;
	}

	m_nFrameCount = 0;

	m_pFrameIn = av_frame_alloc();
	m_pFrameIn->width = m_preset.width;
	m_pFrameIn->height = m_preset.height;
#if ENABLE_HW
	m_pFrameIn->format = (int)AV_PIX_FMT_NV12;
#else	
	m_pFrameIn->format = (int)AV_PIX_FMT_YUV420P;
#endif
	av_frame_get_buffer(m_pFrameIn, 64);
	
	m_pFrameOut = av_frame_alloc();
	
	m_pFilterIn = av_frame_alloc();
	m_pFilterIn->width = m_preset.width;
	if (bIsInterlace) {
		m_pFilterIn->height = m_preset.height * 2;
	}
	else {
		m_pFilterIn->height = m_preset.height;
	}
#if ENABLE_HW
//	m_pFilterIn->format = AV_PIX_FMT_NV12;
	m_pFilterIn->format = AV_PIX_FMT_YUV420P;
#else
	m_pFilterIn->format = AV_PIX_FMT_YUV420P;
#endif
	av_frame_get_buffer(m_pFilterIn, 64);

	m_pFiltFrame = av_frame_alloc();

	m_pYadif = new yadif();
//	m_pYadif->create_yadif_filter(m_preset.width, m_preset.height*2);	

	m_pEQFilter = new CTnmVideoEQFilter();
	
	return true;

init_error:

	if (m_pEnc) {
		avcodec_free_context(&m_pEnc);		
	}
	m_pEnc = NULL;
	if (m_pDec) {
		avcodec_free_context(&m_pDec);		
	}
	m_pDec = NULL;
	if (m_pFrameIn) {
		av_frame_unref(m_pFrameIn);
		av_frame_free(&m_pFrameIn);
	}
	m_pFrameIn = NULL;
	if (m_pFrameOut) {
		av_frame_unref(m_pFrameOut);
		av_frame_free(&m_pFrameOut);
	}
	m_pFrameOut = NULL;
	if (m_pFilterIn) {
		av_frame_unref(m_pFilterIn);
		av_frame_free(&m_pFilterIn);
	}
	m_pFilterIn = NULL;
	if (m_pFiltFrame) {
		av_frame_unref(m_pFiltFrame);
		av_frame_free(&m_pFiltFrame);
	}
	m_pFiltFrame = NULL;

	return false;
}

int CCodecTwo::decode_frame(uint8_t *stream_in, int sz_stream, uint8_t *frame_out) {
	int ret = 0;
	int nFrameSize = 0;	
	int nGetFrame = 0;
	if (!m_pDec) {
		return -1;
	}

	CMyWarkingTime work;
	int nDecode = 0;
	int nScale = 0;

	work.SetStart();
	if (stream_in) {
		AVPacket pkt = { 0 };
		av_init_packet(&pkt);

		pkt.data = stream_in;
		pkt.size = sz_stream;

		ret = avcodec_send_packet(m_pDec, &pkt);
	}
	else {
		ret = avcodec_send_packet(m_pDec, NULL);
	}

	ret = avcodec_receive_frame(m_pDec, m_pFrameOut);
	work.SetEnd();
	if (ret != 0) {
		return -1;
	}
	nDecode = work.GetDuration();

	work.SetStart();
	if (!m_pSwsCtx) {
		m_pSwsCtx = sws_getContext(m_pFrameOut->width, m_pFrameOut->height, (AVPixelFormat)m_pFrameOut->format,
			m_pFilterIn->width, m_pFilterIn->height, (AVPixelFormat)m_pFilterIn->format, SWS_FAST_BILINEAR, NULL, NULL, NULL);
	}

	if (m_pSwsCtx) {
		m_src_data[0] = m_pFrameOut->data[0];
		m_src_data[1] = m_pFrameOut->data[1];
		m_src_data[2] = m_pFrameOut->data[2];
		m_src_data[3] = 0;

		m_src_line[0] = m_pFrameOut->linesize[0];
		m_src_line[1] = m_pFrameOut->linesize[1];
		m_src_line[2] = m_pFrameOut->linesize[2];
		m_src_line[3] = 0;

		m_dst_data[0] = m_pFilterIn->data[0];
		m_dst_data[1] = m_pFilterIn->data[1];
		m_dst_data[2] = m_pFilterIn->data[2];
		m_dst_data[3] = 0;

		m_dst_line[0] = m_pFilterIn->linesize[0];
		m_dst_line[1] = m_pFilterIn->linesize[1];
		m_dst_line[2] = m_pFilterIn->linesize[2];
		m_dst_line[3] = 0;

		sws_scale(m_pSwsCtx, m_src_data, m_src_line, 0, m_pFrameOut->height, m_dst_data, m_dst_line);
	}
	work.SetEnd();
	nScale = work.GetDuration();
			
	uint8_t *pd = frame_out;

	if (m_pEQFilter) {
		m_pEQFilter->SetContrast(m_videoFilter.fContrast);
		m_pEQFilter->SetBrightness(m_videoFilter.fBrightness);
		m_pEQFilter->set_gamma_r(m_videoFilter.fGamma_r);
		m_pEQFilter->set_gamma_g(m_videoFilter.fGamma_g);
		m_pEQFilter->set_gamma_b(m_videoFilter.fGamma_b);

		AVFrame *pSrcFrame = m_pEQFilter->Convert(m_pFilterIn);

		if (pSrcFrame) {
			for (int i = 0; i < 3; i++) {
				uint8_t *ps = pSrcFrame->data[i];
				int nWidth = pSrcFrame->width;
				int nHeight = pSrcFrame->height;
				if (i) {
					nWidth >>= 1;
					nHeight >>= 1;
				}
				for (int j = 0; j < nHeight; j++) {
					memcpy(pd, ps, nWidth);
					ps += pSrcFrame->linesize[i];
					pd += nWidth;
				}
			}
			nFrameSize = (int)(pd - frame_out);
		}
		else {
			nFrameSize = 0;
		}
	}
	else {
		for (int i = 0; i < 3; i++) {
			uint8_t *ps = m_pFilterIn->data[i];
			int nWidth = m_pFilterIn->width;
			int nHeight = m_pFilterIn->height;
			if (i) {
				nWidth >>= 1;
				nHeight >>= 1;
			}
			for (int j = 0; j < nHeight; j++) {
				memcpy(pd, ps, nWidth);
				ps += m_pFilterIn->linesize[i];
				pd += nWidth;
			}
		}

		nFrameSize = (int)(pd - frame_out);

		av_frame_unref(m_pFilterIn);
	}
	
	return nFrameSize;
}

int CCodecTwo::decode_deinterlace_frame3(uint8_t *stream_in, int sz_stream, uint8_t *frame_out) {
	int ret = 0;
	int nFrameSize = 0;
	int nGetFrame = 0;
	if (!m_pDec) {
		return -1;
	}

	int nDecode = 0;
	int nFilter = 0;
	int nScale = 0;

	CMyWarkingTime work;
		
	work.SetStart();
	if (stream_in) {
		AVPacket pkt = { 0 };
		av_init_packet(&pkt);

		pkt.data = stream_in;
		pkt.size = sz_stream;
				
		ret = avcodec_send_packet(m_pDec, &pkt);
	}
	else {
		ret = avcodec_send_packet(m_pDec, NULL);
	}
	
	ret = avcodec_receive_frame(m_pDec, m_pFrameOut);	
	work.SetEnd();
	if (ret != 0) {
		return -1;
	}	
	nDecode = work.GetDuration();

#if 1
	work.SetStart();
	if (!m_pSwsCtx) {
		m_pSwsCtx = sws_getContext(m_pFrameOut->width, m_pFrameOut->height, (AVPixelFormat)m_pFrameOut->format, 
			m_pFilterIn->width, m_pFilterIn->height, (AVPixelFormat)m_pFilterIn->format, SWS_FAST_BILINEAR, NULL, NULL, NULL);
	}

	if (m_pSwsCtx) {
		m_src_data[0] = m_pFrameOut->data[0];
		m_src_data[1] = m_pFrameOut->data[1];
		m_src_data[2] = m_pFrameOut->data[2];
		m_src_data[3] = 0;

		m_src_line[0] = m_pFrameOut->linesize[0];
		m_src_line[1] = m_pFrameOut->linesize[1];
		m_src_line[2] = m_pFrameOut->linesize[2];
		m_src_line[3] = 0;

		m_dst_data[0] = m_pFilterIn->data[0];
		m_dst_data[1] = m_pFilterIn->data[1];
		m_dst_data[2] = m_pFilterIn->data[2];
		m_dst_data[3] = 0;

		m_dst_line[0] = m_pFilterIn->linesize[0];
		m_dst_line[1] = m_pFilterIn->linesize[1];
		m_dst_line[2] = m_pFilterIn->linesize[2];
		m_dst_line[3] = 0;

		sws_scale(m_pSwsCtx, m_src_data, m_src_line, 0, m_pFrameOut->height, m_dst_data, m_dst_line);
	}
	work.SetEnd();
	nScale = work.GetDuration();
		
	work.SetStart();
	m_pYadif->Put(m_pFilterIn);
#else
	work.SetStart();
	m_pYadif->Put(m_pFrameOut);
#endif
	if (m_pYadif->Get(m_pFiltFrame)) {
		work.SetEnd();
		nFilter = work.GetDuration();
		uint8_t *pd = frame_out;
			
		if (m_pEQFilter) {
			m_pEQFilter->SetContrast(m_videoFilter.fContrast);
			m_pEQFilter->SetBrightness(m_videoFilter.fBrightness);
			m_pEQFilter->set_gamma_r(m_videoFilter.fGamma_r);
			m_pEQFilter->set_gamma_g(m_videoFilter.fGamma_g);
			m_pEQFilter->set_gamma_b(m_videoFilter.fGamma_b);

			AVFrame *pSrcFrame = m_pEQFilter->Convert(m_pFiltFrame);

			if (pSrcFrame) {
				for (int i = 0; i < 3; i++) {
					uint8_t *ps = pSrcFrame->data[i];
					int nWidth = pSrcFrame->width;
					int nHeight = pSrcFrame->height;
					if (i) {
						nWidth >>= 1;
						nHeight >>= 1;
					}
					for (int j = 0; j < nHeight; j++) {
						memcpy(pd, ps, nWidth);
						ps += pSrcFrame->linesize[i];
						pd += nWidth;
					}
				}
				nFrameSize = (int)(pd - frame_out);
			}
			else {
				nFrameSize = 0;
			}
		}
		else {
			for (int i = 0; i < 3; i++) {
				uint8_t *ps = m_pFiltFrame->data[i];
				int nWidth = m_pFiltFrame->width;
				int nHeight = m_pFiltFrame->height;
				if (i) {
					nWidth >>= 1;
					nHeight >>= 1;
				}
				for (int j = 0; j < nHeight; j++) {
					memcpy(pd, ps, nWidth);
					ps += m_pFiltFrame->linesize[i];
					pd += nWidth;
				}
			}

			nFrameSize = (int)(pd - frame_out);

			av_frame_unref(m_pFiltFrame);
		}			
	}	

//	_d(L"[codec] decode: %d, scale: %d, filter: %d\n", nDecode, nScale, nFilter);
	return nFrameSize;
}

bool CCodecTwo::Create(Preset *pPreset, bool bIsJ2K) {
	int ret = 0;

	AVCodec *codec = NULL;
	AVDictionary *opts = NULL;

	memcpy(&m_preset, pPreset, sizeof(Preset));

	if (bIsJ2K) {
		//codec = avcodec_find_encoder(AV_CODEC_ID_JPEG2000);
		codec = avcodec_find_encoder(AV_CODEC_ID_H264);
	}
	else {
		codec = avcodec_find_encoder_by_name("libx265");
	}
	if (!codec) {
		printf("Failed to find encoder\n");
		goto init_error;
	}
	m_pEnc = avcodec_alloc_context3(codec);
	if (!m_pEnc) {
		printf("Failed to allocate encoder context\n");
		goto init_error;
	}

	m_pEnc->width = m_preset.width;
	m_pEnc->height = m_preset.height;
	m_pEnc->bit_rate = m_preset.bitrate;
	m_pEnc->time_base.num = 1;
	m_pEnc->time_base.den = 25;
	m_pEnc->pix_fmt = AV_PIX_FMT_YUV420P;

	if (bIsJ2K) {
		av_dict_set(&opts, "format", "j2k", 0);
		av_dict_set(&opts, "pred", "1", 0);

	}
	else {
		switch (m_preset.quality) {
		case 0:
			av_dict_set(&opts, "preset", "veryslow", 0);
			break;
		case 1:
			av_dict_set(&opts, "preset", "slow", 0);
			break;
		case 2:
			av_dict_set(&opts, "preset", "medium", 0);
			break;
		case 3:
			av_dict_set(&opts, "preset", "veryfast", 0);
			break;
		case 4:
			av_dict_set(&opts, "preset", "ultrafast", 0);
			break;
		}
		//av_dict_set(&opts, "zerolatency", "1", 0);
		//av_dict_set(&opts, "threads", "1", 0);
		//av_dict_set(&opts, "tune", "zero-latency", 0);
		//av_dict_set(&opts, "x265-params", "keyint=1", 0);
		//av_dict_set(&opts, "x265-params", "bitrate=1024000", 0);
		//av_dict_set(&opts, "b", "2048000", 0);
		//av_dict_set(&opts, "x265-params", "muxrate=512000", 0);
	}
	av_dict_set(&opts, "delay", "0", 0);
	//av_dict_set(&opts, "zerolatency", "1", 0);
	//if (!av_dict_get(opts, "threads", NULL, 0))
	//{
	//	av_dict_set(&opts, "threads", "1", 0);
	//}
	ret = avcodec_open2(m_pEnc, codec, &opts);
	//printf("gop_size(%d)\n", m_pEnc->gop_size);
	//printf("bit_rate(%d)\n", m_pEnc->bit_rate);
	if (ret < 0) {
		printf("Failed to open encoder (%d)\n", ret);
		goto init_error;
	}

	m_pFrameIn = av_frame_alloc();

	m_pFrameIn->width = m_preset.width;
	m_pFrameIn->height = m_preset.height;
	m_pFrameIn->format = (int)AV_PIX_FMT_YUV420P;

	av_frame_get_buffer(m_pFrameIn, 64);

	if (bIsJ2K) {
		//codec = avcodec_find_decoder(AV_CODEC_ID_JPEG2000);
		codec = avcodec_find_decoder(AV_CODEC_ID_H264);
	}
	else {
		codec = avcodec_find_decoder(AV_CODEC_ID_H265);
	}
	if (!codec) {
		printf("Failed to find decoder\n");
		goto init_error;
	}

	m_pDec = avcodec_alloc_context3(codec);
	if (!m_pDec) {
		printf("Failed to allocate decoder context\n");
		goto init_error;
	}

	ret = avcodec_open2(m_pDec, codec, NULL);
	//printf("[dec] bit_rate(%d)\n", m_pDec->bit_rate);
	if (ret < 0) {
		printf("Failed to open decoder (%d)\n", ret);
		goto init_error;
	}

	m_pFrameOut = av_frame_alloc();
	m_nFrameCount = 0;

	return true;

init_error:

	if (m_pEnc) {
		avcodec_free_context(&m_pEnc);
		m_pEnc = NULL;
	}
	if (m_pDec) {
		avcodec_free_context(&m_pDec);
		m_pDec = NULL;
	}
	if (m_pFrameIn) {
		av_frame_unref(m_pFrameIn);
		av_frame_free(&m_pFrameIn);
	}
	if (m_pFrameOut) {
		av_frame_unref(m_pFrameOut);
		av_frame_free(&m_pFrameOut);
	}

	return false;
}

bool CCodecTwo::Create(Preset * pPreset, int selectCodec, bool bIsEnc, int IsDouble)
{
	int ret = 0;

	AVCodec *codec = NULL;
	AVDictionary *opts = NULL;

	memcpy(&m_preset, pPreset, sizeof(Preset));

	if (bIsEnc == true) {
		if (selectCodec == 0) {
			//codec = avcodec_find_encoder(AV_CODEC_ID_JPEG2000);
			codec = avcodec_find_encoder(AV_CODEC_ID_H264);
		}
		else if (selectCodec == 1) {
			codec = avcodec_find_encoder(AV_CODEC_ID_JPEG2000);
		}
		else {
			//codec = avcodec_find_encoder_by_name("libx265");
			codec = avcodec_find_encoder_by_name("hevc_cuvid");
		}
		if (!codec) {
			printf("Failed to find encoder\n");
			goto init_error;
		}
		m_pEnc = avcodec_alloc_context3(codec);
		if (!m_pEnc) {
			printf("Failed to allocate encoder context\n");
			goto init_error;
		}

		m_pEnc->width = m_preset.width;
		m_pEnc->height = m_preset.height;
		m_pEnc->bit_rate = m_preset.bitrate;
		m_pEnc->time_base.num = 1;
		m_pEnc->time_base.den = 25;
		m_pEnc->pix_fmt = AV_PIX_FMT_YUV420P;


		if (selectCodec == 1) {
			av_dict_set(&opts, "format", "j2k", 0);
			av_dict_set(&opts, "pred", "1", 0);

		}
		else {
			switch (m_preset.quality) {
			case 0:
				av_dict_set(&opts, "preset", "veryslow", 0);
				break;
			case 1:
				av_dict_set(&opts, "preset", "slow", 0);
				break;
			case 2:
				av_dict_set(&opts, "preset", "medium", 0);
				break;
			case 3:
				av_dict_set(&opts, "preset", "veryfast", 0);
				break;
			case 4:
				av_dict_set(&opts, "preset", "ultrafast", 0);
				break;
			}
			//av_dict_set(&opts, "zerolatency", "1", 0);
			//av_dict_set(&opts, "threads", "1", 0);
			//av_dict_set(&opts, "tune", "zero-latency", 0);
			//av_dict_set(&opts, "x265-params", "keyint=1", 0);
			//av_dict_set(&opts, "x265-params", "bitrate=1024000", 0);
			//av_dict_set(&opts, "b", "2048000", 0);
			//av_dict_set(&opts, "x265-params", "muxrate=512000", 0);
		}
		av_dict_set(&opts, "delay", "0", 0);
		//av_dict_set(&opts, "zerolatency", "1", 0);
		//if (!av_dict_get(opts, "threads", NULL, 0))
		//{
		av_dict_set(&opts, "threads", "1", 0);
		//}
		ret = avcodec_open2(m_pEnc, codec, &opts);
		//printf("gop_size(%d)\n", m_pEnc->gop_size);
		//printf("bit_rate(%d)\n", m_pEnc->bit_rate);
		if (ret < 0) {
			printf("Failed to open encoder (%d)\n", ret);
			goto init_error;
		}
	} // if (bIsEnc == true)


	m_pFrameIn = av_frame_alloc();

	m_pFrameIn->width = m_preset.width;
	m_pFrameIn->height = m_preset.height;
	if (selectCodec == 2)
	{
		m_pFrameIn->format = (int)AV_PIX_FMT_NV12;
	}
	else
	{
		m_pFrameIn->format = (int)AV_PIX_FMT_YUV420P;
	}

	av_frame_get_buffer(m_pFrameIn, 64);

	if (bIsEnc == false) {
		if (selectCodec == 0) {
			//codec = avcodec_find_decoder(AV_CODEC_ID_JPEG2000);
			codec = avcodec_find_decoder(AV_CODEC_ID_H264);
		}
		else if (selectCodec == 1) {
			codec = avcodec_find_decoder(AV_CODEC_ID_JPEG2000);
		}
		else {
			codec = avcodec_find_decoder(AV_CODEC_ID_HEVC);
			//codec = avcodec_find_decoder_by_name("hevc_cuvid");
			//codec = avcodec_find_decoder(AV_CODEC_ID_H264);
		}
		if (!codec) {
			printf("Failed to find decoder\n");
			goto init_error;
		}

		m_pDec = avcodec_alloc_context3(codec);
		if (!m_pDec) {
			printf("Failed to allocate decoder context\n");
			goto init_error;
		}

		ret = avcodec_open2(m_pDec, codec, NULL);
		//printf("[dec] bit_rate(%d)\n", m_pDec->bit_rate);
		if (ret < 0) {
			printf("Failed to open decoder (%d)\n", ret);
			goto init_error;
		}

		m_pFrameOut = av_frame_alloc();
	} // if (bIsEnc == false)

	m_nFrameCount = 0;

	m_pYadif = new yadif();
//	m_pYadif->create_yadif_filter(720, 480, IsDouble);
	m_pFiltFrame = av_frame_alloc();

	m_pFilterIn = av_frame_alloc();
	m_pFilterIn->width = 720;
	m_pFilterIn->height = 480;
	m_pFilterIn->format = AV_PIX_FMT_YUV420P;
	av_frame_get_buffer(m_pFilterIn, 64);

	return true;

init_error:

	if (m_pEnc) {
		avcodec_free_context(&m_pEnc);
		m_pEnc = NULL;
	}
	if (m_pDec) {
		avcodec_free_context(&m_pDec);
		m_pDec = NULL;
	}
	if (m_pFrameIn) {
		av_frame_unref(m_pFrameIn);
		av_frame_free(&m_pFrameIn);
	}
	if (m_pFrameOut) {
		av_frame_unref(m_pFrameOut);
		av_frame_free(&m_pFrameOut);
	}

	return false;
}

HRESULT CCodecTwo::UYVYtoARGB(uint8_t* Frame)
{
	HRESULT hr = S_FALSE;
#if 0
	if (!m_pSwsCtx){
		m_pSwsCtx = sws_getContext(inputWidth, ACTUAL_HEIGHT, AV_PIX_FMT_UYVY422/*AV_PIX_FMT_YUYV422*/, inputWidth, ACTUAL_HEIGHT, AV_PIX_FMT_RGB32, SWS_FAST_BILINEAR, NULL, NULL, NULL);
	}

	if (m_pSwsCtx) {
		m_src_data[0] = Frame;
		m_src_data[1] = 0;
		m_src_data[2] = 0;
		m_src_data[3] = 0;

		m_src_line[0] = inputWidth * 2;
		m_src_line[1] = 0;
		m_src_line[2] = 0;
		m_src_line[3] = 0;

		m_dst_data[0] = m_pDrawBuffer;
		m_dst_data[1] = 0;
		m_dst_data[2] = 0;
		m_dst_data[3] = 0;

		m_dst_line[0] = inputWidth * 4;
		m_dst_line[1] = 0;
		m_dst_line[2] = 0;
		m_dst_line[3] = 0;

		sws_scale(m_pSwsCtx, m_src_data, m_src_line, 0, ACTUAL_HEIGHT, m_dst_data, m_dst_line);

		hr = S_OK;
	}
#endif

	return hr;
}

HRESULT CCodecTwo::YV12toARGB(uint8_t* Frame)
{
	HRESULT hr = S_FALSE;
#if 0
	if (m_nWndWidth != m_nRenderWidth || m_nWndHeight != m_nRenderHeight) {
		if (m_pSwsCtx) {
			sws_freeContext(m_pSwsCtx);
			m_pSwsCtx = NULL;
		}

		m_nRenderWidth = m_nWndWidth;
		m_nRenderHeight = m_nWndHeight;
	}

	if (!m_pSwsCtx) {
		m_pSwsCtx = sws_getContext(renderWidth, renderHeight, AV_PIX_FMT_YUV420P, m_nRenderWidth, m_nRenderHeight, AV_PIX_FMT_RGB32, SWS_FAST_BILINEAR, NULL, NULL, NULL);
	}

	if (m_pSwsCtx) {
		m_src_data[0] = Frame;
		m_src_data[1] = Frame + (inputWidth * inputHeight);
		m_src_data[2] = Frame + (inputWidth * inputHeight) + (inputWidth * inputHeight / 4);
		m_src_data[3] = 0;

		m_src_line[0] = renderWidth;
		m_src_line[1] = renderWidth / 2;
		m_src_line[2] = renderWidth / 2;
		m_src_line[3] = 0;

		m_dst_data[0] = m_pDrawBuffer;
		m_dst_data[1] = 0;
		m_dst_data[2] = 0;
		m_dst_data[3] = 0;

		m_dst_line[0] = m_nRenderWidth * 4;
		m_dst_line[1] = 0;
		m_dst_line[2] = 0;
		m_dst_line[3] = 0;

		sws_scale(m_pSwsCtx, m_src_data, m_src_line, 0, renderHeight, m_dst_data, m_dst_line);

		hr = S_OK;
	}
#endif

	return hr;
}

HRESULT CCodecTwo::YUV420PtoARGB(uint8_t * Frame)
{
	HRESULT hr = S_FALSE;

#if 0
	if (m_nWndWidth != m_nRenderWidth || m_nWndHeight != m_nRenderHeight) {
		if (m_pSwsCtx) {
			sws_freeContext(m_pSwsCtx);
			m_pSwsCtx = NULL;
		}

		m_nRenderWidth = m_nWndWidth;
		m_nRenderHeight = m_nWndHeight;
	}

	if (!m_pSwsCtx) {
		m_pSwsCtx = sws_getContext(m_nRenderWidth, m_nRenderHeight, AV_PIX_FMT_YUV420P, m_nRenderWidth, m_nRenderHeight, AV_PIX_FMT_RGB32, SWS_FAST_BILINEAR, NULL, NULL, NULL);
	}

	if (m_pSwsCtx) {
		m_src_data[0] = Frame;
		m_src_data[1] = Frame + (m_nRenderWidth * m_nRenderHeight);
		m_src_data[2] = Frame + (m_nRenderWidth * m_nRenderHeight) + (m_nRenderWidth * m_nRenderHeight / 4);
		m_src_data[3] = 0;

		m_src_line[0] = m_nRenderWidth;
		m_src_line[1] = m_nRenderWidth / 2;
		m_src_line[2] = m_nRenderWidth / 2;
		m_src_line[3] = 0;

		m_dst_data[0] = m_pDrawBuffer;
		m_dst_data[1] = 0;
		m_dst_data[2] = 0;
		m_dst_data[3] = 0;

		m_dst_line[0] = m_nRenderWidth * 4;
		m_dst_line[1] = 0;
		m_dst_line[2] = 0;
		m_dst_line[3] = 0;

		sws_scale(m_pSwsCtx, m_src_data, m_src_line, 0, m_nRenderHeight, m_dst_data, m_dst_line);

		hr = S_OK;
	}
#endif

	return hr;
}

HRESULT CCodecTwo::YUV420PtoARGBx2(uint8_t * Frame)
{
	HRESULT hr = S_FALSE;
#if 0
	if (m_nWndWidth != m_nRenderWidth || m_nWndHeight != m_nRenderHeight) {
		if (m_pSwsCtx) {
			sws_freeContext(m_pSwsCtx);
			m_pSwsCtx = NULL;
		}

		m_nRenderWidth = m_nWndWidth;
		m_nRenderHeight = m_nWndHeight;
	}

	if (!m_pSwsCtx) {
		m_pSwsCtx = sws_getContext(inputWidth, inputHeight, AV_PIX_FMT_YUV420P, m_nRenderWidth, m_nRenderHeight, AV_PIX_FMT_RGB32, SWS_FAST_BILINEAR, NULL, NULL, NULL);
	}

	if (m_pSwsCtx) {
		m_src_data[0] = Frame;
		m_src_data[1] = Frame + (inputWidth * inputHeight);
		m_src_data[2] = Frame + (inputWidth * inputHeight) + (inputWidth * inputHeight / 4);
		m_src_data[3] = 0;

		m_src_line[0] = inputWidth;
		m_src_line[1] = inputWidth / 2;
		m_src_line[2] = inputWidth / 2;
		m_src_line[3] = 0;

		m_dst_data[0] = m_pDrawBuffer;
		m_dst_data[1] = 0;
		m_dst_data[2] = 0;
		m_dst_data[3] = 0;

		m_dst_line[0] = m_nRenderWidth * 4;
		m_dst_line[1] = 0;
		m_dst_line[2] = 0;
		m_dst_line[3] = 0;

		sws_scale(m_pSwsCtx, m_src_data, m_src_line, 0, inputHeight, m_dst_data, m_dst_line);

		hr = S_OK;
	}
#endif

	return hr;
}

HRESULT CCodecTwo::NV12toARGB(uint8_t * Frame)
{
	HRESULT hr = S_FALSE;
#if 0
	if (m_nWndWidth != m_nRenderWidth || m_nWndHeight != m_nRenderHeight) {
		if (m_pSwsCtx) {
			sws_freeContext(m_pSwsCtx);
			m_pSwsCtx = NULL;
		}

		m_nRenderWidth = m_nWndWidth;
		m_nRenderHeight = m_nWndHeight;
	}

	if (!m_pSwsCtx) {
		m_pSwsCtx = sws_getContext(inputWidth, inputHeight, AV_PIX_FMT_NV12, m_nRenderWidth, m_nRenderHeight, AV_PIX_FMT_RGB32, SWS_FAST_BILINEAR, NULL, NULL, NULL);
	}

	if (m_pSwsCtx) {
		m_src_data[0] = Frame;
		m_src_data[1] = Frame + (inputWidth * inputHeight);
		m_src_data[2] = 0;
		m_src_data[3] = 0;

		m_src_line[0] = inputWidth;
		m_src_line[1] = inputWidth;
		m_src_line[2] = 0;
		m_src_line[3] = 0;

		m_dst_data[0] = m_pDrawBuffer;
		m_dst_data[1] = 0;
		m_dst_data[2] = 0;
		m_dst_data[3] = 0;

		m_dst_line[0] = m_nRenderWidth * 4;
		m_dst_line[1] = 0;
		m_dst_line[2] = 0;
		m_dst_line[3] = 0;

		sws_scale(m_pSwsCtx, m_src_data, m_src_line, 0, inputHeight, m_dst_data, m_dst_line);

		hr = S_OK;
	}
#endif
	return hr;
}

HRESULT CCodecTwo::NV12toARGB(uint8_t * source, uint8_t * dst)
{
	HRESULT hr = S_FALSE;
#if 0
	if (m_nWndWidth != m_nRenderWidth || m_nWndHeight != m_nRenderHeight) {
		if (m_pSwsCtx) {
			sws_freeContext(m_pSwsCtx);
			m_pSwsCtx = NULL;
		}

		m_nRenderWidth = m_nWndWidth;
		m_nRenderHeight = m_nWndHeight;
	}

	if (!m_pSwsCtx) {
		m_pSwsCtx = sws_getContext(renderWidth, renderHeight, AV_PIX_FMT_YUV420P, m_nRenderWidth, m_nRenderHeight, AV_PIX_FMT_RGB32, SWS_FAST_BILINEAR, NULL, NULL, NULL);
	}

	if (m_pSwsCtx) {
		m_src_data[0] = source;
		m_src_data[1] = source + (renderWidth * renderHeight);
		m_src_data[2] = source + (renderWidth * renderHeight) + (renderWidth * renderHeight / 4);
		m_src_data[3] = 0;

		m_src_line[0] = renderWidth;
		m_src_line[1] = renderWidth / 2;
		m_src_line[2] = renderWidth / 2;
		m_src_line[3] = 0;

		m_dst_data[0] = dst;
		m_dst_data[1] = 0;
		m_dst_data[2] = 0;
		m_dst_data[3] = 0;

		m_dst_line[0] = m_nRenderWidth * 4;
		m_dst_line[1] = 0;
		m_dst_line[2] = 0;
		m_dst_line[3] = 0;

		sws_scale(m_pSwsCtx, m_src_data, m_src_line, 0, renderHeight, m_dst_data, m_dst_line);

		hr = S_OK;
	}
#endif

	return hr;
}

HRESULT CCodecTwo::NV12toARGB(uint8_t * source, uint8_t * dst, unsigned int width, unsigned int height)
{
	HRESULT hr = S_FALSE;
#if 0
	if (m_nWndWidth != m_nRenderWidth || m_nWndHeight != m_nRenderHeight) {
		if (m_pSwsCtx) {
			sws_freeContext(m_pSwsCtx);
			m_pSwsCtx = NULL;
		}

		m_nRenderWidth = m_nWndWidth;
		m_nRenderHeight = m_nWndHeight;
	}

	if (!m_pSwsCtx) {
		m_pSwsCtx = sws_getContext(width, height, AV_PIX_FMT_YUV420P, width, height, AV_PIX_FMT_RGB32, SWS_FAST_BILINEAR, NULL, NULL, NULL);
	}

	if (m_pSwsCtx) {
		m_src_data[0] = source;
		m_src_data[1] = source + (width * height);
		m_src_data[2] = source + (width * height) + (width * height / 4);
		//m_src_data[1] = source + (inputWidth * inputHeight) + (inputWidth * inputHeight / 4);
		//m_src_data[2] = source + (inputWidth * inputHeight);
		m_src_data[3] = 0;

		m_src_line[0] = width;
		m_src_line[1] = width / 2;
		m_src_line[2] = width / 2;
		m_src_line[3] = 0;

		m_dst_data[0] = dst;
		m_dst_data[1] = 0;
		m_dst_data[2] = 0;
		m_dst_data[3] = 0;

		m_dst_line[0] = width * 4;
		m_dst_line[1] = 0;
		m_dst_line[2] = 0;
		m_dst_line[3] = 0;

		sws_scale(m_pSwsCtx, m_src_data, m_src_line, 0, height, m_dst_data, m_dst_line);

		hr = S_OK;
	}
#endif

	return hr;
}

#if 0
#define NV12 0
#define MP4 1
AVFormatContext *fmt_ctx = nullptr;
AVCodecContext *dec_ctx = nullptr;

int ret;
int eof = 0;
AVCodec *dec = nullptr;
int video_stream_index = -1;

if ((ret = avformat_open_input(&fmt_ctx, "zombie.mp4", NULL, NULL)) < 0) {
	av_log(NULL, AV_LOG_ERROR, "Cannot open input file\n");
	return ret;
}

if ((ret = avformat_find_stream_info(fmt_ctx, NULL)) < 0) {
	av_log(NULL, AV_LOG_ERROR, "Cannot find stream information\n");
	return ret;
}

/* select the video stream */
ret = av_find_best_stream(fmt_ctx, AVMEDIA_TYPE_VIDEO, -1, -1, &dec, 0);
if (ret < 0) {
	av_log(NULL, AV_LOG_ERROR, "Cannot find a video stream in the input file\n");
	return ret;
}
video_stream_index = ret;

dec_ctx = avcodec_alloc_context3(dec);
if (!dec_ctx)
return AVERROR(ENOMEM);
avcodec_parameters_to_context(dec_ctx, fmt_ctx->streams[video_stream_index]->codecpar);

/* init the video decoder */
if ((ret = avcodec_open2(dec_ctx, dec, NULL)) < 0) {
	av_log(NULL, AV_LOG_ERROR, "Cannot open video decoder\n");
	return ret;
}

AVPacket packet;

yadif_cuda * pYadifCuda = new yadif_cuda();
pYadifCuda->create_yadif_cuda_filter(720, 480, 720, 480);
pYadifCuda->CreateHwFrames(720, 480);

yadif * pYadif = new yadif();


AVFrame *frame = NULL;
AVFrame *sw_frame = NULL;
frame = av_frame_alloc();
sw_frame = av_frame_alloc();

FILE* fpWrite = fopen("yadif_double16.yuv", "wb");

#if MP4
pYadif->create_yadif_filter(512, 288, 512, 288);

while (!eof)
{
	if ((ret = av_read_frame(fmt_ctx, &packet)) < 0)
	{
		eof = 1;
	}

	if (packet.stream_index == video_stream_index)
	{
		ret = avcodec_send_packet(dec_ctx, &packet);
		if (ret < 0) {
			av_log(NULL, AV_LOG_ERROR, "Error while sending a packet to the decoder\n");
			break;
		}

		while (ret >= 0)
		{
			ret = avcodec_receive_frame(dec_ctx, frame);
			if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
				break;
			}
			else if (ret < 0) {
				av_log(NULL, AV_LOG_ERROR, "Error while receiving a frame from the decoder\n");
				getchar();
				exit(0);
			}

#if 0
			av_buffersrc_add_frame_flags(pYadif->m_pCtxBuffSrc, frame, AV_BUFFERSRC_FLAG_KEEP_REF);
			while (1)
			{
				ret = av_buffersink_get_frame(pYadif->m_pCtxBuffSink, sw_frame);
				if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
					break;
				}
				if (ret < 0) {
					_d(_T("[SCALE_DOUBLE] CRITICAL ERR!!!\n"));
					getchar();
					exit(0);
				}

				for (uint32_t n = 0; n < 3; ++n)
				{
					uint8_t * pd = sw_frame->data[n];
					uint32_t nWidth = sw_frame->width;
					uint32_t nHeight = sw_frame->height;
					if (n) {
						nWidth >>= 1;
						nHeight >>= 1;
					}
					for (uint32_t h = 0; h < nHeight; ++h)
					{
						fwrite(pd, 1, nWidth, fpWrite);
						pd += sw_frame->linesize[n];
					}
				}
				av_frame_unref(sw_frame);
			}
#else
			pYadif->Put(frame);
			while (pYadif->Get(sw_frame))
			{
				if (fpWrite)
				{
#if NV12
					for (uint32_t n = 0; n < 2; ++n)
					{
						uint8_t * pd = sw_frame->data[n];
						uint32_t nWidth = sw_frame->width;
						uint32_t nHeight = sw_frame->height;
						if (n) {
							nHeight >>= 1;
						}
						for (uint32_t h = 0; h < nHeight; ++h)
						{
							fwrite(pd, 1, nWidth, fpWrite);
							pd += sw_frame->linesize[n];
						}
					}
#else

					for (uint32_t n = 0; n < 3; ++n)
					{
						uint8_t * pd = sw_frame->data[n];
						uint32_t nWidth = sw_frame->width;
						uint32_t nHeight = sw_frame->height;
						if (n) {
							nWidth >>= 1;
							nHeight >>= 1;
						}
						for (uint32_t h = 0; h < nHeight; ++h)
						{
							fwrite(pd, 1, nWidth, fpWrite);
							pd += sw_frame->linesize[n];
						}
					}
					av_frame_unref(sw_frame);
#endif
					//fclose(fpWrite);
				}
			}
#endif
		}
	}
}

if (fpWrite)
{
	fclose(fpWrite);
}
#else
pYadif->create_yadif_filter(720, 480, 720, 480);

frame->width = 720;
frame->height = 480;
frame->format = AV_PIX_FMT_YUV420P;

av_frame_get_buffer(frame, 64);

FILE* fpRead = fopen("420p.yuv", "rb");

int err;

if (fpRead)
{
	while (!eof)
	{
#if NV12
		for (uint32_t n = 0; n < 2; ++n)
		{
			uint8_t * pd = frame->data[n];
			uint32_t nWidth = frame->width;
			uint32_t nHeight = frame->height;
			if (n) {
				nHeight >>= 1;
			}
			for (uint32_t h = 0; h < nHeight; ++h)
			{
				err = fread(pd, 1, nWidth, fpRead);
				if (err != nWidth)
				{
					exit(0);
				}
				pd += frame->linesize[n];
			}
		}
#else
		for (uint32_t n = 0; n < 3; ++n)
		{
			uint8_t * pd = frame->data[n];
			uint32_t nWidth = frame->width;
			uint32_t nHeight = frame->height;
			if (n) {
				nWidth >>= 1;
				nHeight >>= 1;
			}
			for (uint32_t h = 0; h < nHeight; ++h)
			{
				err = fread(pd, 1, nWidth, fpRead);
				if (err != nWidth)
				{
					eof = 1;
				}
				pd += frame->linesize[n];
			}
		}
#endif
		//err = av_hwframe_transfer_data(pYadifCuda->m_pVF, frame, 0);
		//if (err < 0) {
		//	_d(_T("Error while transferring frame data to surface...(errcode : %d)\n"), err);
		//}



		//err = av_hwframe_transfer_data(sw_frame, pYadifCuda->m_pVF, 0);
		//if (err < 0) {
		//	_d(_T("error while transferring frame data to surface...(errcode : %d)\n"), err);
		//}

		frame->sample_aspect_ratio.num = 1;
		frame->sample_aspect_ratio.den = 1;

		frame->pts = 0;
		frame->pkt_pts = 0;
		frame->pkt_dts = 0;
		frame->pkt_duration = 512;

		av_buffersrc_add_frame_flags(pYadif->m_pCtxBuffSrc, frame, AV_BUFFERSRC_FLAG_KEEP_REF);
		while (1)
		{
			err = av_buffersink_get_frame(pYadif->m_pCtxBuffSink, sw_frame);
			if (err == AVERROR(EAGAIN) || err == AVERROR_EOF) {
				break;
			}
			if (err < 0) {
				_d(_T("[SCALE_DOUBLE] CRITICAL ERR!!!\n"));
				getchar();
				exit(0);
			}

			for (uint32_t n = 0; n < 3; ++n)
			{
				uint8_t * pd = sw_frame->data[n];
				uint32_t nWidth = sw_frame->width;
				uint32_t nHeight = sw_frame->height;
				if (n) {
					nWidth >>= 1;
					nHeight >>= 1;
				}
				for (uint32_t h = 0; h < nHeight; ++h)
				{
					fwrite(pd, 1, nWidth, fpWrite);
					pd += sw_frame->linesize[n];
				}
			}
			av_frame_unref(sw_frame);
		}

		//			pYadif->Put(frame);
		//			if (pYadif->Get(sw_frame))
		//			{
		//				if (fpWrite)
		//				{
		//#if NV12
		//					for (uint32_t n = 0; n < 2; ++n)
		//					{
		//						uint8_t * pd = sw_frame->data[n];
		//						uint32_t nWidth = sw_frame->width;
		//						uint32_t nHeight = sw_frame->height;
		//						if (n) {
		//							nHeight >>= 1;
		//						}
		//						for (uint32_t h = 0; h < nHeight; ++h)
		//						{
		//							fwrite(pd, 1, nWidth, fpWrite);
		//							pd += sw_frame->linesize[n];
		//						}
		//					}
		//#else
		//
		//					for (uint32_t n = 0; n < 3; ++n)
		//					{
		//						uint8_t * pd = sw_frame->data[n];
		//						uint32_t nWidth = sw_frame->width;
		//						uint32_t nHeight = sw_frame->height;
		//						if (n) {
		//							nWidth >>= 1;
		//							nHeight >>= 1;
		//						}
		//						for (uint32_t h = 0; h < nHeight; ++h)
		//						{
		//							fwrite(pd, 1, nWidth, fpWrite);
		//							pd += sw_frame->linesize[n];
		//						}
		//					}
		//					av_frame_unref(sw_frame);
		//#endif
		//					fclose(fpWrite);
		//				}
		//			}
	}

	if (fpWrite)
	{
		fclose(fpWrite);
	}

	fclose(fpRead);
}
#endif

#endif