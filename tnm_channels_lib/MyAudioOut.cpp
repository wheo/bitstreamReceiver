#include "stdafx.h"
#include "MyAudioOut.h"

#pragma comment(lib, "winmm.lib")



void CALLBACK PlayCallback(HWAVEOUT hwo, UINT uMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2);

CMyAudioOut::CMyAudioOut(void)
{
	m_hWaveOut = NULL;

	for(int i = 0; i < NUM_BLOB; i++) {
		m_pWaveHdr[i] = NULL;
	}
}

CMyAudioOut::~CMyAudioOut(void)
{
	if (m_hWaveOut) {
		waveOutReset(m_hWaveOut);
		waveOutClose(m_hWaveOut);
	}

	for (int i = 0; i < NUM_BLOB; i++)
    {
		if (m_pWaveHdr[i]) 	{
			WAVEHDR *pHdr = m_pWaveHdr[i];
			if (pHdr && pHdr->lpData) {
				delete [] pHdr->lpData;
				delete pHdr;
			}
		}
    }
}

void CMyAudioOut::Init(int nSampleRate, int nChannel, int nMillisec)
{
	WAVEFORMATEX waveFormatEx;
	
	waveFormatEx.wFormatTag = WAVE_FORMAT_PCM;
	waveFormatEx.nChannels = nChannel;
	waveFormatEx.nSamplesPerSec = nSampleRate;
	waveFormatEx.wBitsPerSample = 16;
	waveFormatEx.nBlockAlign = nChannel * (waveFormatEx.wBitsPerSample / 8);
	waveFormatEx.nAvgBytesPerSec = nSampleRate * waveFormatEx.nBlockAlign;
	waveFormatEx.cbSize = 0;

	m_nBuffSize = 2*nSampleRate*nChannel*nMillisec/1000;
	//m_nBuffSize = 2*nSampleRate*nChannel;

	_d(_T("Buffer : %d\n"), m_nBuffSize);

	/** 주의 !!!!
	 Callback 함수내에서 WaveOut 함수를 호출할 경우 hang 가능성이 존재한다. 따라서 세마포어를 사용하여 샘플추가시 처리하도록 한다. */
	m_hSem = CreateSemaphore(NULL, 0, NUM_BLOB, NULL);
	//m_hSem = CreateEvent(0, FALSE, FALSE, 0);

	if (waveOutOpen(&m_hWaveOut, 0, &waveFormatEx, (DWORD_PTR)PlayCallback, (DWORD_PTR)this, CALLBACK_FUNCTION) != MMSYSERR_NOERROR)
	{
		_d(_T("Couldn't create wave mapper"));
	}

	m_nIndexHdr = 0;
	for (int i = 0; i < NUM_BLOB; i++)
    {
		WAVEHDR *pHdr = new WAVEHDR;
		
		pHdr->dwBufferLength = 0;
		pHdr->lpData = new char [m_nBuffSize];
		pHdr->dwFlags = 0;

		m_pWaveHdr[i] = pHdr;
    }
}

bool CMyAudioOut::AddToAudioBuffer(void * pData)
{
	static uint32_t count = 0;
	//_d(_T("count(%d)\n"), ++count);
	MMRESULT ret = 0;
	int nCount = 0;
	int nLen = m_nBuffSize;

	WAVEHDR *pHdr = m_pWaveHdr[m_nIndexHdr];

	if (pHdr->dwBufferLength) {
		//m_cs.Lock();
		//_d(_T("WaitForSingleObject\n"));
		WaitForSingleObject(m_hSem, INFINITE);
		//m_cs.Unlock();
	}

	if (pHdr->dwFlags & WHDR_PREPARED) {
		//_d(_T("waveOutUnprepareHeader\n"));
		waveOutUnprepareHeader(m_hWaveOut, pHdr, sizeof(WAVEHDR));
	}

	memcpy(pHdr->lpData, pData, nLen);
	pHdr->dwBufferLength = nLen;
	pHdr->dwUser = (DWORD_PTR)this;

	ret = waveOutPrepareHeader(m_hWaveOut, pHdr, sizeof(WAVEHDR));
	if (ret != MMSYSERR_NOERROR)
	{
		_d(_T("waveOutPrepareHeader err : %d\n"), ret);
		return false;
	}

	ret = waveOutWrite(m_hWaveOut, pHdr, sizeof(WAVEHDR));
	if (ret != MMSYSERR_NOERROR)
	{
		_d(_T("waveOutWrite err : %d\n"), ret);
		return false;
	}

	//Sleep(5000);

	m_nIndexHdr++;
	if (m_nIndexHdr >= NUM_BLOB) {
		m_nIndexHdr = 0;
	}

	return true;
}

bool CMyAudioOut::AddToAudioBuffer(void *pData, int32_t size)
{
	static uint32_t count = 0;
	//_d(_T("count(%d)\n"), ++count);
	MMRESULT ret = 0;
	int nCount = 0;
	int nLen = size;
	//_d(_T("size(%d)\n"), size);

	WAVEHDR *pHdr = m_pWaveHdr[m_nIndexHdr];

	if (pHdr->dwBufferLength) {
		//m_cs.Lock();
		//_d(_T("WaitForSingleObject\n"));
		WaitForSingleObject(m_hSem, INFINITE);
		//m_cs.Unlock();
	}

	if (pHdr->dwFlags & WHDR_PREPARED) {
		//_d(_T("waveOutUnprepareHeader\n"));
		waveOutUnprepareHeader(m_hWaveOut, pHdr, sizeof(WAVEHDR));
	}

	memcpy(pHdr->lpData, pData, nLen);
	pHdr->dwBufferLength = nLen;
	pHdr->dwUser = (DWORD_PTR)this;

	ret = waveOutPrepareHeader(m_hWaveOut, pHdr, sizeof(WAVEHDR));
	if (ret != MMSYSERR_NOERROR)
	{
		_d(_T("waveOutPrepareHeader err : %d\n"), ret);
		return false;
	}

	ret = waveOutWrite(m_hWaveOut, pHdr, sizeof(WAVEHDR));
	if (ret != MMSYSERR_NOERROR)
	{
		_d(_T("waveOutWrite err : %d\n"), ret);
		return false;
	}

	//Sleep(5000);

	m_nIndexHdr++;
	if (m_nIndexHdr >= NUM_BLOB) {
		m_nIndexHdr = 0;
	}

	return true;
}

void CMyAudioOut::OnWaveDone()
{
	m_cs.Lock();
	ReleaseSemaphore(m_hSem, 1, NULL);
	//_d(_T("SetEvent\n"));
	//SetEvent(m_hSem);
	m_cs.Unlock();
}

void CALLBACK PlayCallback(HWAVEOUT hwo, UINT uMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2)
{
	CMyAudioOut *pthis = (CMyAudioOut*)dwInstance;

	switch(uMsg)
	{
	case WOM_OPEN:
		break;
	case WOM_DONE:
		{
			//_d(_T("WOM_DONE\n"));
			pthis->OnWaveDone();
		}
		break;
	case WOM_CLOSE:
		break;
	}
}



