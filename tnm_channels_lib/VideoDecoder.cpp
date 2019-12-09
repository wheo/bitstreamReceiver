#include "stdafx.h"
#include "VideoDecoder.h"

#define ENABLE_HW	0
CVideoDecoder::CVideoDecoder()
{
	m_pDec = NULL;	

	m_pFrameIn = NULL;
	m_pFrameOut = NULL;
	m_pFiltFrame = NULL;
	m_pFilterIn = NULL;

	m_pYadif = NULL;
	m_pEQFilter = NULL;

	m_videoFilter.fContrast = 1.;
	m_videoFilter.fBrightness = 0.;
	m_videoFilter.fSaturation = 1.0;
	m_videoFilter.fGamma = 1.0;
	m_videoFilter.fGamma_r = 1.;
	m_videoFilter.fGamma_g = 1.;
	m_videoFilter.fGamma_b = 1.;

	m_videoFilter.nCropBottom = 0;
	m_videoFilter.nCropLeft = 0;
	m_videoFilter.nCropRight = 0;
	m_videoFilter.nCropTop = 0;
}


CVideoDecoder::~CVideoDecoder()
{	
	if (m_pDec) {
		avcodec_free_context(&m_pDec);		
	}

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
	if (m_pFiltFrame) {
		av_frame_unref(m_pFiltFrame);
		av_frame_free(&m_pFiltFrame);
	}
	m_pFiltFrame = NULL;

	m_pDec = NULL;	

	SAFE_DELETE(m_pYadif);
	SAFE_DELETE(m_pEQFilter);
}

void CVideoDecoder::set_contrast(double fValue)
{
	m_videoFilter.fContrast = fValue;
	m_pEQFilter->set_filter_value(&m_videoFilter);
}
void CVideoDecoder::set_brightness(double fValue)
{
	m_videoFilter.fBrightness = fValue;
	m_pEQFilter->set_filter_value(&m_videoFilter);
}
void CVideoDecoder::set_saturation(double fValue)
{
	m_videoFilter.fSaturation = fValue;
	m_pEQFilter->set_filter_value(&m_videoFilter);
}
void CVideoDecoder::set_crop_value(uint32_t left, uint32_t top, uint32_t right, uint32_t bottom)
{
	m_videoFilter.nCropLeft = left;
	m_videoFilter.nCropTop = top;
	m_videoFilter.nCropRight = right;
	m_videoFilter.nCropBottom = bottom;
	m_pEQFilter->set_filter_value(&m_videoFilter);
}

void CVideoDecoder::get_image_size(int &nWidth, int &nHeight)
{
	nWidth = m_image_width;
	nHeight = m_image_height;
}

bool CVideoDecoder::create_decoder(int nCodecID, int nSrcWidth, int nSrcHeight, bool bIsInterlace)
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
	//	codec = avcodec_find_decoder_by_name("libx264");
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
	//	codec = avcodec_find_decoder_by_name("libx265");
#endif
	}
	if (!codec) {
		_d("Failed to find decoder\n");
		goto init_error;
	}

	m_pDec = avcodec_alloc_context3(codec);
	if (!m_pDec) {
		_d("Failed to allocate decoder context\n");
		goto init_error;
	}

	av_dict_set(&opts, "delay", "0", 0);

	ret = avcodec_open2(m_pDec, codec, &opts);
	//printf("[dec] bit_rate(%d)\n", m_pDec->bit_rate);
	if (ret < 0) {
		printf("Failed to open decoder (%d)\n", ret);
		goto init_error;
	}
	
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
	m_pEQFilter->set_filter_value(&m_videoFilter);

	return true;

init_error:	
	if (m_pDec) {
		avcodec_free_context(&m_pDec);
	}
	m_pDec = NULL;

	if (m_pSwsCtx) {
		sws_freeContext(m_pSwsCtx);
	}
	m_pSwsCtx = NULL;

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

int CVideoDecoder::decode_deinterlace_frame(uint8_t *stream_in, int sz_stream, uint8_t *frame_out) {
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
		if (ret != 0) {
			_d("[decoder] failed send packet\n");
			return -1;
		}
	}
	else {
		ret = avcodec_send_packet(m_pDec, NULL);
		if (ret != 0) {
			_d("[decoder] failed send packet\n");
			return -1;
		}
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
#if 0
			m_pEQFilter->SetContrast(m_videoFilter.fContrast);
			m_pEQFilter->SetBrightness(m_videoFilter.fBrightness);
			m_pEQFilter->set_gamma(m_videoFilter.fGamma);
			m_pEQFilter->set_gamma_r(m_videoFilter.fGamma_r);
			m_pEQFilter->set_gamma_g(m_videoFilter.fGamma_g);
			m_pEQFilter->set_gamma_b(m_videoFilter.fGamma_b);
#endif
			AVFrame *pSrcFrame = m_pEQFilter->Convert(m_pFiltFrame);

			if (pSrcFrame) {
				m_image_width = pSrcFrame->width;
				m_image_height = pSrcFrame->height;
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
			m_image_width = m_pFiltFrame->width;
			m_image_height = m_pFiltFrame->height;
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
