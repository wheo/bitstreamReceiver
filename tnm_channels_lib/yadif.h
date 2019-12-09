#pragma once
class yadif
{
public:
	yadif();
	~yadif();

	void create_yadif_filter(AVFrame *pFrame, int IsDouble = 1);


	void Put(AVFrame *pFrame);
	bool Get(AVFrame *pFrame);

	int m_nSrcWidth;
	int m_nSrcHeight;

	int m_nDstWidth;
	int m_nDstHeight;

	char m_strFilterDesc[256];

	AVFilterContext *m_pCtxBuffSrc;
	AVFilterContext *m_pCtxBuffSink;

	AVFilterGraph *m_pFG;

	AVBufferRef *m_pHwFramesCtx;

	AVBufferRef *m_pHWDeviceCtx;

	AVFrame *m_pVF;
};

