#include "stdafx.h"
#include "MyRecvAudio.h"

#include "MyAudioOut.h"

#include "G723Dec.h"

#pragma comment(lib, "libG723.1.lib")

#define STREAM_RCV_TIMEOUT			50

CMyRecvAudio::CMyRecvAudio(void)
{
	m_nCurChannel = 0;
	m_pMassBuffer = new char[480*MAX_NUM_QUEUE];

	m_hThreadRecv = INVALID_HANDLE_VALUE;
	m_hThreadAudio = INVALID_HANDLE_VALUE;

	m_sdRecvUDP = INVALID_SOCKET;

	m_bIsStart = false;


	m_q.r = 0;
	m_q.w = 0;
	m_q.n = 0;
	for (int i=0; i<MAX_NUM_QUEUE; i++) {
		Elem *pe = &m_q.e[i];

		pe->len = 0;
		pe->p = m_pMassBuffer + i*480;
		memset(pe->p, 0xff, 480);
	}

	m_nBitrate = 0;

	m_nMulticastPort = 3000;

	memset(&m_ch_info[0], 0, sizeof(channel_s));
	memset(&m_ch_info[1], 0, sizeof(channel_s));

}

CMyRecvAudio::~CMyRecvAudio(void)
{
	Disable();

	delete [] m_pMassBuffer;
}

void CMyRecvAudio::set_channel_info(channel_s ch1, channel_s ch2)
{
	memcpy(&m_ch_info[0], &ch1, sizeof(channel_s));
	memcpy(&m_ch_info[1], &ch2, sizeof(channel_s));
}

void CMyRecvAudio::change_audio_channel()
{
	int nChannel = m_nCurChannel;

	nChannel++;

	if (nChannel >= 2) {
		nChannel = 0;
	}

	enable_audio(nChannel);
}

void CMyRecvAudio::enable_audio(int nChannel)
{ 
	Disable();

	m_bExitThread = false;
	m_nCurChannel = nChannel; 
	
	CString strIP;
	int nPort;

	strIP.Format(_T("%s"), m_ch_info[m_nCurChannel].strIP);
	nPort = m_ch_info[m_nCurChannel].nPort;

	if (strIP.IsEmpty() == false && nPort > 0) {
		UINT uiThreadID = 0;

		m_hThreadAudio = (HANDLE)_beginthreadex(NULL, 0, ThreadAudio, (void*)this, 0, &uiThreadID);
		m_hThreadRecv = (HANDLE)_beginthreadex(NULL, 0, ThreadRecv, (void*)this, 0, &uiThreadID);

		m_bIsStart = true;
	}
}

bool CMyRecvAudio::Disable()
{
	m_bExitThread = true;	

	if (m_sdRecvUDP != INVALID_SOCKET)
	{
		closesocket(m_sdRecvUDP);
		m_sdRecvUDP = INVALID_SOCKET;
	}

	if (m_hThreadRecv != INVALID_HANDLE_VALUE)
		WaitForSingleObject(m_hThreadRecv, INFINITE);
	if (m_hThreadAudio != INVALID_HANDLE_VALUE)
		WaitForSingleObject(m_hThreadAudio, INFINITE);

	m_q.r = 0;
	m_q.w = 0;
	m_q.n = 0;

	m_nBitrate = 0;

	m_bIsStart = false;

	return true;
}

UINT CMyRecvAudio::ThreadRecv(LPVOID lParam)
{
	CMyRecvAudio *pthis = reinterpret_cast<CMyRecvAudio*>(lParam);
	pthis->RecvProc();

	return 0;
}

void CMyRecvAudio::RecvProc()
{
	char ipAddr[32];
	int nBool = 1;

	struct sockaddr_in sin;
	struct ip_mreq join_addr;

	char *pStream = new char[24];

	///> 1. 소켓 생성
	m_sdRecvUDP = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
	if (m_sdRecvUDP == INVALID_SOCKET) {
		_d(_T("음성수신단 소켓생성 실패\n"));
		return;
	}

	if (setsockopt(m_sdRecvUDP, SOL_SOCKET, SO_REUSEADDR, (const char*)&nBool, sizeof(nBool)) < 0) {
		_d(_T("음성수신단 소켓옵션 실패1\n"));
		closesocket(m_sdRecvUDP);
		m_sdRecvUDP = INVALID_SOCKET;
		return;
	}

	memset(&sin, 0, sizeof(struct sockaddr_in));
	sin.sin_family = AF_INET;


	sin.sin_port = htons(m_ch_info[m_nCurChannel].nPort);
	sin.sin_addr.s_addr = htonl(INADDR_ANY);


	if (bind(m_sdRecvUDP, (const sockaddr*)&sin, sizeof(sin)) < 0) {
		_d(_T("음성수신단 bind 실패\n"));
		closesocket(m_sdRecvUDP);
		m_sdRecvUDP = INVALID_SOCKET;
		return;
	}

#if 1
	BOOL optval = FALSE;
	if (setsockopt(m_sdRecvUDP, IPPROTO_IP, IP_MULTICAST_LOOP, (char*)&optval, sizeof(optval)) == SOCKET_ERROR)
	{
		closesocket(m_sdRecvUDP);
		m_sdRecvUDP = INVALID_SOCKET;
		_d(_T("[AM] Multicast loop setopt Failed\n"));
	}
#endif

	join_addr.imr_multiaddr.s_addr = inet_addr(m_ch_info[m_nCurChannel].strIP);
	join_addr.imr_interface.s_addr = htonl(INADDR_ANY);

	if (setsockopt(m_sdRecvUDP, IPPROTO_IP, IP_ADD_MEMBERSHIP, (const char*)&join_addr, sizeof(join_addr)) < 0) {
		_d(_T("음성수신단 멀티케스트 접속 실패\n"));
		closesocket(m_sdRecvUDP);
		m_sdRecvUDP = INVALID_SOCKET;
		return;
	}
		
	int nState = 1000;
	if (setsockopt(m_sdRecvUDP, SOL_SOCKET, SO_RCVTIMEO, (const char*)&nState, sizeof(struct timeval)) < 0) {
		_d(_T("음성수신단 소켓옵션 실패2\n"));
		closesocket(m_sdRecvUDP);
		m_sdRecvUDP = INVALID_SOCKET;
		return;
	}

	int nReceived = 0;
	DWORD dwTick = GetTickCount();
	
	while(!m_bExitThread) {
		int nRecvLen, nFromLen;
		Elem *pe = &m_q.e[m_q.w];

		nFromLen = sizeof(struct sockaddr_in);
		nRecvLen = recvfrom(m_sdRecvUDP, (char*)pStream, PACKET_SIZE, 0, (sockaddr*)&sin, &nFromLen);
		if (nRecvLen < 0) {
			continue;
		}
				
		if (nRecvLen == PACKET_SIZE) {
			memcpy(pe->p + pe->len, pStream, nRecvLen);
			pe->channel = m_nCurChannel - 1;

			pe->len += nRecvLen;

			m_q.w++;
			if (m_q.w >= MAX_NUM_QUEUE) {
				m_q.w = 0;
			}

			m_q.n++;
		}

		nReceived += nRecvLen;

		if ((GetTickCount() - dwTick) >= 1000) {
			m_nBitrate = nReceived * 8;

			dwTick = GetTickCount();
			nReceived = 0;

			_d(_T("[audio.%d]         수신 BPS : %.1fKbps\n"), m_nCurChannel, m_nBitrate/1024.);
		}
	}

	_d(_T("End recv loop\n"));

	delete [] pStream;

}

UINT CMyRecvAudio::ThreadAudio(LPVOID lParam)
{
	CMyRecvAudio *pthis = reinterpret_cast<CMyRecvAudio*>(lParam);

	pthis->AudioProc();

	return 0;
}

void CMyRecvAudio::AudioProc()
{
	CMyAudioOut out;

	unsigned char wav_header[44] = {
	0x52, 0x49, 0x46, 0x46, 0x00, 0x00, 0x00, 0x00, 0x57, 0x41, 0x56, 0x45,
	0x66, 0x6D, 0x74, 0x20, 0x10, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00,
	0x40, 0x1F, 0x00, 0x00, 0x80, 0x3E, 0x00, 0x00, 0x02, 0x00, 0x10, 0x00,
	0x64, 0x61, 0x74, 0x61, 0x00, 0x00, 0x00, 0x00
	};

	short *pPCM = new short [882000];
	CG723Dec *pDec;
	pDec = new CG723Dec();
	pDec->Create();
	//pDec->EnablePF(true);

	out.Init(8000, 1, 30);

	Sleep(200);

	_d(_T("Start play loop\n"));
	while(!m_bExitThread) {

		Elem *pe = &m_q.e[m_q.r];
		if (pe->len >= PACKET_SIZE) {
			pDec->Process((char*)pe->p, pPCM);
			out.AddToAudioBuffer(pPCM);
				
			pe->len = 0;

			m_q.r++;
			if (m_q.r >= MAX_NUM_QUEUE) {
				m_q.r = 0;
			}

			m_q.n--;
		}
	} // while(!m_bExitThread)
	_d(_T("End play loop\n"));

	delete [] pPCM;
	delete pDec;

}
