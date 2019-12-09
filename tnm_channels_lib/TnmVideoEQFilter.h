#pragma once
struct tagVideoFilter
{
	double fContrast;
	double fBrightness;
	double fSaturation;
	double fGamma;
	double fGamma_r;
	double fGamma_g;
	double fGamma_b;

	int	nCropLeft;
	int nCropTop;
	int nCropRight;
	int nCropBottom;
};

class CTnmVideoEQFilter
{
public:
	CTnmVideoEQFilter();
	~CTnmVideoEQFilter();

	void create_filter(int nSrcWidth, int nSrcHeight, AVPixelFormat format);
	void delete_filter();

	/* -1000.0 ~ 1000.0 */
	void SetContrast(double fContrast);
	/* -1.0 ~ 1.0 */
	void SetBrightness(double fBrightness);

	void set_saturation(double fGamma);
	/* 0.1 ~ 10.0 */
	void set_gamma_r(double fValue);
	void set_gamma_g(double fValue);
	void set_gamma_b(double fValue);

	void set_filter_value(tagVideoFilter *value);

	AVFrame *Convert(AVFrame *pSrc);

protected:
	AVFrame *m_pFrame;
	AVFrame *m_pDst;

	SwrContext *m_pSC;

	AVFilterGraph *m_pFG;

	AVFilterContext *m_pCtxBuffSrc;
	AVFilterContext *m_pCtxBuffSink;	

	AVFilterInOut *m_pOutputs;
	AVFilterInOut *m_pInputs;

	int m_nSrcWidth;
	int m_nSrcHeight;

	tagVideoFilter m_videoFilter;

	CCriticalSection m_cs;
};

