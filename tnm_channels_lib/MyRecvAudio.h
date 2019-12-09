#pragma once

#define MAX_NUM_QUEUE			512

typedef struct tagElem
{

	char *p;
	int len;
	int channel;

} Elem;

typedef struct tagSQ
{
	Elem e[MAX_NUM_QUEUE];

	int r;
	int w;

	int n;
} SQ;

class CMyRecvAudio
{
public:
	CMyRecvAudio(void);
	~CMyRecvAudio(void);

	void set_channel_info(channel_s ch1, channel_s ch2);

	void change_audio_channel();

	void enable_audio(int nChannel);
	bool Disable();

	int GetBitrate() { return m_nBitrate; };
	
	bool GetStatus() { return m_bIsStart; };

private:
	int m_nCurChannel;
	int m_nBitrate;


	HANDLE m_hThreadRecv;
	HANDLE m_hThreadAudio;

	bool m_bExitThread;

	char *m_pMassBuffer;

	SQ	m_q;

	bool m_bIsStart;

	SOCKET m_sdRecvUDP;
	
	///> 멀티캐스트 추가
	CString m_strMulticastIp;
	int		m_nMulticastPort;

	channel_s m_ch_info[2];
	
protected:

	static unsigned __stdcall ThreadRecv(LPVOID lParam);
	void RecvProc();

	static unsigned __stdcall ThreadAudio(LPVOID lParam);
	void AudioProc();
};

