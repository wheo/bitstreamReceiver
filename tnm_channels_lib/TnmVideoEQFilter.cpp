#include "stdafx.h"
#include "TnmVideoEQFilter.h"


CTnmVideoEQFilter::CTnmVideoEQFilter()
{
	m_pCtxBuffSrc = NULL;
	m_pCtxBuffSink = NULL;

	m_pSC = NULL;
	m_pFrame = NULL;
	m_pDst = NULL;

	m_pFG = NULL;

	m_pOutputs = NULL;
	m_pInputs = NULL;

	//m_fContrast = 1;  // -1000.0 ~ 1000.0; default : 1
	//m_fBrightness = 0; // -1.0 ~ 1.0; default : 0
	//m_fGamma = 1; // 0.1 ~ 10.0; default : 1
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

CTnmVideoEQFilter::~CTnmVideoEQFilter()
{
	if (m_pFrame) {
		av_frame_unref(m_pFrame);
	}
	if (m_pDst) {
		av_frame_unref(m_pDst);
	}

	if (m_pSC) {
		swr_free(&m_pSC);
	}

	delete_filter();
}

void CTnmVideoEQFilter::set_filter_value(tagVideoFilter *value)
{
	memcpy(&m_videoFilter, value, sizeof(tagVideoFilter));
	delete_filter();
}
void CTnmVideoEQFilter::SetContrast(double fContrast)
{
	m_videoFilter.fContrast = fContrast;
	delete_filter();
}
void CTnmVideoEQFilter::SetBrightness(double fBrightness)
{
	m_videoFilter.fBrightness = fBrightness;
	delete_filter();
}
void CTnmVideoEQFilter::set_saturation(double fGamma)
{
	m_videoFilter.fSaturation = fGamma;
	delete_filter();
}
void CTnmVideoEQFilter::set_gamma_r(double fValue)
{
	m_videoFilter.fGamma_r = fValue;
	delete_filter();
}
void CTnmVideoEQFilter::set_gamma_g(double fValue)
{
	m_videoFilter.fGamma_g = fValue;
	delete_filter();
}
void CTnmVideoEQFilter::set_gamma_b(double fValue)
{
	m_videoFilter.fGamma_b = fValue;
	delete_filter();
}

void CTnmVideoEQFilter::delete_filter()
{
	m_cs.Lock();

	if (m_pOutputs) {
		avfilter_inout_free(&m_pOutputs);
	}
	m_pOutputs = NULL;
	if (m_pInputs) {
		avfilter_inout_free(&m_pInputs);
	}
	m_pInputs = NULL;
	if (m_pFG) {
		avfilter_graph_free(&m_pFG);
	}
	m_pFG = NULL;

	m_cs.Unlock();
}

void CTnmVideoEQFilter::create_filter(int nSrcWidth, int nSrcHeight, AVPixelFormat format)
{
	int ret;
	char args[512];
	char strDesc[512];

	const AVFilter *buffersrc = avfilter_get_by_name("buffer");
	const AVFilter *buffersink = avfilter_get_by_name("buffersink");
	m_pOutputs = avfilter_inout_alloc();
	m_pInputs = avfilter_inout_alloc();

	enum AVPixelFormat pix_fmts[] = { AV_PIX_FMT_YUV420P, AV_PIX_FMT_NONE };
	pix_fmts[0] = format;

	m_pFG = avfilter_graph_alloc();
	if (!m_pOutputs || !m_pInputs || !m_pFG) {
		ret = AVERROR(ENOMEM);
		_d(_T("Not have enough resources\n"));
		return;
	}

	sprintf(args, "video_size=%dx%d:pix_fmt=%d:time_base=%d/%d",
		nSrcWidth, nSrcHeight, format, 1001, 30000);
	
	//_d(L"[DEINT] In args = %s\n", args);
	ret = avfilter_graph_create_filter(&m_pCtxBuffSrc, buffersrc, "in", args, NULL, m_pFG);
	if (ret < 0) {
		_d(_T("[DEINT] Cannot create buffer source\n"));
	}

	/* buffer video sink: to terminate the filter chain. */
	ret = avfilter_graph_create_filter(&m_pCtxBuffSink, buffersink, "out", NULL, NULL, m_pFG);
	if (ret < 0) {
		_d(_T("[DEINT] Cannot create buffer sink\n"));
	}

	ret = av_opt_set_int_list(m_pCtxBuffSink, "pix_fmts", pix_fmts, AV_PIX_FMT_NONE, AV_OPT_SEARCH_CHILDREN);
	if (ret < 0) {
		_d(_T("[DEINT] Cannot set output pixel format\n"));
	}

	/* Endpoints for the filter graph. */
	m_pOutputs->name = av_strdup("in");
	m_pOutputs->filter_ctx = m_pCtxBuffSrc;
	m_pOutputs->pad_idx = 0;
	m_pOutputs->next = NULL;

	m_pInputs->name = av_strdup("out");
	m_pInputs->filter_ctx = m_pCtxBuffSink;
	m_pInputs->pad_idx = 0;
	m_pInputs->next = NULL;
	
	if (m_videoFilter.nCropBottom > 0 || m_videoFilter.nCropLeft > 0 || m_videoFilter.nCropRight > 0 || m_videoFilter.nCropTop) {
		sprintf(strDesc, "eq=contrast=%.01f:brightness=%.01f:saturation=%.01f,crop=in_w-%d:in_h-%d:%d:%d",
			m_videoFilter.fContrast, m_videoFilter.fBrightness, m_videoFilter.fSaturation, 
			m_videoFilter.nCropLeft+ m_videoFilter.nCropRight, m_videoFilter.nCropTop+m_videoFilter.nCropBottom, m_videoFilter.nCropLeft, m_videoFilter.nCropTop);
	}
	else {
		sprintf(strDesc, "eq=contrast=%.01f:brightness=%.01f:saturation=%.01f",
			m_videoFilter.fContrast, m_videoFilter.fBrightness, m_videoFilter.fSaturation);
	}

	if ((ret = avfilter_graph_parse_ptr(m_pFG, strDesc, &m_pInputs, &m_pOutputs, NULL)) < 0) {
		char errbuf[256];
		av_strerror(ret, errbuf, 256);
		_d(_T("[DEINT] Failed to parse filter desc. %s\n"), strDesc);
	}

	if ((ret = avfilter_graph_config(m_pFG, NULL)) < 0) {
		_d(_T("[DEINT] Failed to create filter graph\n"));
	}
}

AVFrame *CTnmVideoEQFilter::Convert(AVFrame *pSrc)
{
	int ret;

	m_cs.Lock();

	if (!m_pFG) {
		create_filter(pSrc->width, pSrc->height, (AVPixelFormat)pSrc->format);
	}	
	
	if (!m_pDst) {
		m_pDst = av_frame_alloc();
		//m_pDst->format = AV_PIX_FMT_YUV420P;
		m_pDst->format = pSrc->format;
	}

	//	ret = av_buffersrc_add_frame_flags(m_pCtxBuffSrc, pSrc, AV_BUFFERSRC_FLAG_KEEP_REF);
	ret = av_buffersrc_add_frame_flags(m_pCtxBuffSrc, pSrc, AV_BUFFERSRC_FLAG_PUSH);
	if (ret < 0) {
		char errbuf[256];
		av_strerror(ret, errbuf, 256);

		//TCHAR strWBuff[256];		
		//MultiToWide(errbuf, strWBuff);
		//_d(L"Error while feeding the filtergraph.. %s\n", strWBuff);
		if (m_pFG) {
			avfilter_graph_free(&m_pFG);
			m_pFG = NULL;
		}
		m_cs.Unlock();
		return NULL;
	}

	bool bIsAvail = false;

	while (1) {
		int nCount = 0; 

		AVFrame *filt_frame = m_pDst;
		ret = av_buffersink_get_frame(m_pCtxBuffSink, filt_frame);
		if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
			break;
		}
		if (ret < 0) {
			_d(_T("[DEINT] CRITICAL ERR!!!\n"));
		}
		//_d(L"[DEINT] output frame : %I64d\n", filt_frame->pts);
		if (!m_pFrame) {
			m_pFrame = av_frame_alloc();
			m_pFrame->width = filt_frame->width;
			m_pFrame->height = filt_frame->height;
			m_pFrame->format = filt_frame->format;//AV_PIX_FMT_YUV444P10LE;

			av_frame_get_buffer(m_pFrame, 64);
		}		
		bIsAvail = true;
		av_frame_copy(m_pFrame, filt_frame);
		m_pFrame->width = filt_frame->width;
		m_pFrame->height = filt_frame->height;
		av_frame_unref(filt_frame);

		nCount++;
	}

	m_cs.Unlock();

	if (bIsAvail) {
		return m_pFrame;
	}

	return NULL;
}