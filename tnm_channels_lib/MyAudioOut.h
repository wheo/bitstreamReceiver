#pragma once

#include <mmsystem.h>

#define PACKET_SIZE 24

/** WaveFileHeader 구조체 */
typedef struct tagWaveFileHeader
{
    WORD  wFormatTag;         // Format category
    WORD  wChannels;          // Number of channels
    DWORD dwSamplesPerSec;    // Sampling rate
    DWORD dwAvgBytesPerSec;   // For buffer estimation
    WORD  wBlockAlign;        // Data block size
    WORD  wBitsPerSample;
} WAVE_FILE_HEADER, *PWAVE_FILE_HEADER;

/** WaveSample 구조체 */
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

/** Windows API를 이용한 오디오 출력 클래스 */
class CMyAudioOut
{
public:
	CMyAudioOut(void);
	~CMyAudioOut(void);
	
public:
	/** 초기화한다.
	@param nSampleRate : 샘플레이트
	@param nChannel : 채널개수 
	@param nMillisec : 밀리초 재생 단위, 버퍼 크기 결정에 사용 */
	void Init(int nSampleRate, int nChannel, int nMillisec);

	/** 출력할 오디오 샘플을 추가한다.
	@param pData : PCM 버퍼 포인터
	@return 성공하면 true, 실패하면 false */
	bool AddToAudioBuffer(void *pData);
	bool AddToAudioBuffer(void *pData, int32_t size);

	/** Callback에서 호출하는 함수, 세마포어 감소가 실제 역할*/
	void OnWaveDone();
	
protected:

	int m_nBuffSize;

	HWAVEOUT m_hWaveOut; ///> WaveOut 핸들

	int		m_nIndexHdr; ///> 쓰기 인덱스

	HANDLE	m_hSem; ///> 세마포어 핸들

	WAVEHDR *m_pWaveHdr[NUM_BLOB]; ///> 헤더구조체 포인터
	
	CCriticalSection m_cs; ///> 멀티쓰레딩에 대비한 크리티컬섹션
};

