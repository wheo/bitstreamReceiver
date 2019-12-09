#include "stdafx.h"
#include "yadif.h"



yadif::yadif()
{
	m_pFG = NULL;
}


yadif::~yadif()
{
	if (m_pFG) {
		avfilter_graph_free(&m_pFG);
	}
}

void yadif::create_yadif_filter(AVFrame *pFrame, int IsDouble)
{
	int ret;

	char args[512];

	m_nSrcWidth = pFrame->width;
	m_nSrcHeight = pFrame->height;

	if (IsDouble == 1) {
		sprintf(m_strFilterDesc, "yadif=1:-1:0");
	}
	else {
		sprintf(m_strFilterDesc, "yadif=0:-1:0");
	}
	
	//sprintf(m_strFilterDesc, "kerndeint=sharp=1");

	const AVFilter *buffersrc = avfilter_get_by_name("buffer");
	const AVFilter *buffersink = avfilter_get_by_name("buffersink");

	AVFilterInOut *outputs = avfilter_inout_alloc();
	AVFilterInOut *inputs = avfilter_inout_alloc();

	enum AVPixelFormat pix_fmts[] = { AV_PIX_FMT_YUV420P, AV_PIX_FMT_NONE };
	pix_fmts[0] = (AVPixelFormat)pFrame->format;
	m_pFG = avfilter_graph_alloc();
	if (!outputs || !inputs || !m_pFG) {
		ret = AVERROR(ENOMEM);
		_d(_T("[SCALE_DOUBLE] Not have enough resources\n"));
		avfilter_inout_free(&outputs);
		avfilter_inout_free(&inputs);
		return;
	}

	sprintf(args, "video_size=%dx%d:pix_fmt=%d:time_base=%d/%d",
		m_nSrcWidth, m_nSrcHeight, pFrame->format, 1, 1);


	ret = avfilter_graph_create_filter(&m_pCtxBuffSrc, buffersrc, "in", args, NULL, m_pFG);
	if (ret < 0) {
		_d(_T("[SCALE_DOUBLE] Cannot create buffer source\n"));
	}

	ret = avfilter_graph_create_filter(&m_pCtxBuffSink, buffersink, "out", NULL, NULL, m_pFG);
	if (ret < 0) {
		_d(_T("[SCALE_DOUBLE] Cannot create buffer sink\n"));
	}
	ret = av_opt_set_int_list(m_pCtxBuffSink, "pix_fmts", pix_fmts, AV_PIX_FMT_NONE, AV_OPT_SEARCH_CHILDREN);
	if (ret < 0) {
		_d(_T("[SCALE_DOUBLE] Cannot set output pixel format\n"));
	}

	outputs->name = av_strdup("in");
	outputs->filter_ctx = m_pCtxBuffSrc;
	outputs->pad_idx = 0;
	outputs->next = NULL;

	inputs->name = av_strdup("out");
	inputs->filter_ctx = m_pCtxBuffSink;
	inputs->pad_idx = 0;
	inputs->next = NULL;

	if ((ret = avfilter_graph_parse_ptr(m_pFG, m_strFilterDesc, &inputs, &outputs, NULL)) < 0) {
		_d(_T("[SCALE_DOUBLE] Failed to parse filter desc. %s\n"), m_strFilterDesc);
	}

	if ((ret = avfilter_graph_config(m_pFG, NULL)) < 0) {
		_d(_T("[SCALE_DOUBLE] Failed to create filter graph\n"));
	}

	avfilter_inout_free(&outputs);
	avfilter_inout_free(&inputs);
}

void yadif::Put(AVFrame * pFrame)
{
	if (!m_pFG) {
		create_yadif_filter(pFrame);
	}

	AVFrame *pSource = pFrame;

	//pSource->pkt_pts = AV_NOPTS_VALUE;
	//pSource->pkt_dts = AV_NOPTS_VALUE;

	int ret = av_buffersrc_add_frame_flags(m_pCtxBuffSrc, pSource, AV_BUFFERSRC_FLAG_KEEP_REF);
	if (ret < 0) {
		char errbuf[256];
		av_strerror(ret, errbuf, 256);
		_d(_T("[SCALE_DOUBLE] Error while feeding the filtergraph %s\n"), errbuf);
	}
}

bool yadif::Get(AVFrame * pFrame)
{
	while (1) {
		int ret = av_buffersink_get_frame(m_pCtxBuffSink, pFrame);
		if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
			return false;
		}
		if (ret < 0) {
			_d(_T("[SCALE_DOUBLE] CRITICAL ERR!!!\n"));
			return false;
		}
		return true;
	}
}
