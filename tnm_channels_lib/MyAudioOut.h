#pragma once

#include <mmsystem.h>

#define PACKET_SIZE 24

/** WaveFileHeader ����ü */
typedef struct tagWaveFileHeader
{
    WORD  wFormatTag;         // Format category
    WORD  wChannels;          // Number of channels
    DWORD dwSamplesPerSec;    // Sampling rate
    DWORD dwAvgBytesPerSec;   // For buffer estimation
    WORD  wBlockAlign;        // Data block size
    WORD  wBitsPerSample;
} WAVE_FILE_HEADER, *PWAVE_FILE_HEADER;

/** WaveSample ����ü */
typedef struct tagWaveSample
{
     WAVEFORMATEX WaveFormatEx;
     char *pSampleData;
     UINT Index;
     UINT Size;
     DWORD dwId;
     DWORD bPlaying;
     struct tagWaveSample *pNext;

} WAVE_SAMPLE, *PWAVE_SAMPLE;
 
#define NUM_BLOB			(3)
#define MAX_SAMPLE_SIZE		(2*2*32000) 

/** Windows API�� �̿��� ����� ��� Ŭ���� */
class CMyAudioOut
{
public:
	CMyAudioOut(void);
	~CMyAudioOut(void);
	
public:
	/** �ʱ�ȭ�Ѵ�.
	@param nSampleRate : ���÷���Ʈ
	@param nChannel : ä�ΰ��� 
	@param nMillisec : �и��� ��� ����, ���� ũ�� ������ ��� */
	void Init(int nSampleRate, int nChannel, int nMillisec);

	/** ����� ����� ������ �߰��Ѵ�.
	@param pData : PCM ���� ������
	@return �����ϸ� true, �����ϸ� false */
	bool AddToAudioBuffer(void *pData);
	bool AddToAudioBuffer(void *pData, int32_t size);

	/** Callback���� ȣ���ϴ� �Լ�, �������� ���Ұ� ���� ����*/
	void OnWaveDone();
	
protected:

	int m_nBuffSize;

	HWAVEOUT m_hWaveOut; ///> WaveOut �ڵ�

	int		m_nIndexHdr; ///> ���� �ε���

	HANDLE	m_hSem; ///> �������� �ڵ�

	WAVEHDR *m_pWaveHdr[NUM_BLOB]; ///> �������ü ������
	
	CCriticalSection m_cs; ///> ��Ƽ�������� ����� ũ��Ƽ�ü���
};

