#include "stdafx.h"
#include "TnmComm.h"
#define RECV_SIZE			(188*70+12)
#define QUEUE_SIZE			1

#define ENABLE_RTP_MPEGTS	0

CTnmComm::CTnmComm()
{	
	m_sd_send = INVALID_SOCKET;
	m_sd_recv = INVALID_SOCKET;	
	
	m_connected = false;
}

CTnmComm::~CTnmComm()
{
	StopRecv();		
}

void CTnmComm::set_save_start(int nType, byte enable, mux_cfg_s *mux_cfg, char *file_name)
{
	byte buffer[sizeof(mux_cfg_s) * MAX_VIDEO_CHANNEL_COUNT + FILENAME_MAX + 5];

	int nStructSize = sizeof(mux_cfg_s);
	int nBuffSize = 0;

	buffer[0] = 'T';
	buffer[1] = 'N';
	buffer[2] = 0x00;
	buffer[3] = 0x01;
	buffer[4] = enable;
	nBuffSize += 5;
	
	byte *pData = &buffer[5];
	for (int i = 0; i < MAX_VIDEO_CHANNEL_COUNT; i++) {
		memcpy(pData, &mux_cfg[i], nStructSize);
		pData += nStructSize;
		nBuffSize += nStructSize;
	}

	if (nType == 1) {
		char strName[FILENAME_MAX];
		_stprintf(strName, _T("%s"), file_name);
		memcpy(pData, strName, FILENAME_MAX);
		nBuffSize += FILENAME_MAX;
	}
	
	if (send_data(buffer, nBuffSize, RECORD_PORT)) {
		_d(_T("[send] set save start.. ip: %s, port: %d, type: %d, sendSize: %d, channel: %02x\n"), m_strIpAddr, m_send_port, nType, nBuffSize, enable);
	}
	else {
		_d(_T("[send] set save start failed.. type: %d, sendSize: %d, channel: %02x\n"), nType, nBuffSize, enable);
	}
}
void CTnmComm::set_save_stop(int nType)
{
	byte buffer[4];
		
	buffer[0] = 'T';
	buffer[1] = 'N';
	buffer[2] = 0x00;	
	buffer[3] = 0x02;	
	
	if (send_data(buffer, 4, RECORD_PORT)) {
		_d(_T("[send] set save stop.. ip: %s, port: %d, type: %d\n"), m_strIpAddr, m_send_port, nType);
	}
	else {
		_d(_T("[send] set save stop failed.. type: %d\n"), nType);
	}
}

void CTnmComm::set_conversion_cfg(int cmd, int codec, int bitrate)
{
	byte buffer[9];

	buffer[0] = 0x7E;
	buffer[1] = 6;
	buffer[2] = cmd;
	buffer[3] = codec;
	memcpy(&buffer[4], &bitrate, 4);
	buffer[8] = 0xC1;

	if (send_data(buffer, 9, RECORD_PORT)) {
		_d(_T("[send] set conversion cfg.. cmd: %d, codec: %d, bitrate: %d\n"), cmd, codec, bitrate);
	}
	else {
		_d(_T("[send] set conversion cfg failed.. cmd: %d, codec: %d, bitrate: %d\n"), cmd, codec, bitrate);
	}
}

void CTnmComm::set_analog_cfg(int mode)
{
	byte buffer[4];

	buffer[0] = 0x7E;
	buffer[1] = 1;
	buffer[2] = mode;	
	buffer[3] = 0xC1;

	if (send_data(buffer, 4, RECORD_PORT)) {
		_d(_T("[send] set analog cfg.. mode: %d\n"), mode);
	}
	else {
		_d(_T("[send] set analog cfg failed.. mode: %d\n"), mode);
	}
}

void CTnmComm::set_distribution_cfg(byte* poutput)
{
	byte buffer[11];

	buffer[0] = 0x7E;
	buffer[1] = 8;
	for (int i = 0; i < 8; i++) {
		buffer[2 + i] = poutput[i];
	}	
	buffer[10] = 0xC1;

	if (send_data(buffer, 11, RECORD_PORT)) {
		_d(_T("[send] set distribution cfg.. 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x\n"), 
			poutput[0], poutput[1], poutput[2], poutput[3], poutput[4], poutput[5], poutput[6], poutput[7]);
	}
	else {
		_d(_T("[send] set distribution cfg failed.. 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x\n"),
			poutput[0], poutput[1], poutput[2], poutput[3], poutput[4], poutput[5], poutput[6], poutput[7]);
	}
}

bool CTnmComm::send_data(byte* pdata, int size, int target_port)
{
	if (m_sd_send == INVALID_SOCKET) {
		return false;
	}

	struct sockaddr_in servAddr;

	servAddr.sin_family = AF_INET;
	servAddr.sin_addr.s_addr = inet_addr(m_strIpAddr);
	servAddr.sin_port = htons(m_send_port[target_port]);

	int send = sendto(m_sd_send, (char*)pdata, size, 0, (struct sockaddr*)&servAddr, sizeof(servAddr));
	if (send != size) {
		return false;
	}

	return true;
}

void CTnmComm::StartRecv(CString strIP, int send_port[NUM_CONTROL_DEVICE], int recv_port)
{
	StopRecv();
	
	m_strIpAddr.Format(_T("%s"), strIP);
	m_send_port[RECORD_PORT] = send_port[RECORD_PORT];
	m_send_port[EVENT_PORT] = send_port[EVENT_PORT];
	m_send_port[PLAY_PORT] = send_port[PLAY_PORT];
	m_recv_port = recv_port;
			
	m_connected = false;

	if (Connect()) {
		Start();	
	}	
}

void CTnmComm::StopRecv()
{
	Terminate();

	Disconnect();	
}

void CTnmComm::Disconnect()
{
	if (m_sd_recv != INVALID_SOCKET) {
		closesocket(m_sd_recv);
	}
	m_sd_recv = INVALID_SOCKET;

	if (m_sd_send != INVALID_SOCKET) {
		closesocket(m_sd_send);
	}
	m_sd_send = INVALID_SOCKET;
}
bool CTnmComm::Connect()
{	
	if (CreateSocket() == false) {
		return false;
	}

	return true;
}
bool CTnmComm::CreateSocket()
{
	SOCKADDR_IN addr;

	int nState;

	DWORD dwErr;

	m_sd_recv = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
	if (m_sd_recv == INVALID_SOCKET) {
		_d(_T("[Recv] Failed to create socket\n"));
		return false;
	}

	nState = 1;
	if (setsockopt(m_sd_recv, SOL_SOCKET, SO_REUSEADDR, (const char*)&nState, sizeof(nState)) < 0) {
		closesocket(m_sd_recv);
		dwErr = GetLastError();
		_d(_T("[Recv] Failed to set sockopt : REUSEADDR, %x\n"), dwErr);
		return false;
	}
#if 1
	nState = 100;
	if (setsockopt(m_sd_recv, SOL_SOCKET, SO_RCVTIMEO, (const char*)&nState, sizeof(struct timeval)) < 0) {
		_d(_T("[Recv] 소켓옵션 설정 실패\n"));
		return false;
	}
#endif
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(m_recv_port);
	if (bind(m_sd_recv, (SOCKADDR*)&addr, sizeof(addr)) == SOCKET_ERROR) {
		closesocket(m_sd_recv);
		dwErr = GetLastError();
		_d(_T("[Recv] Failed to bind, %x\n"), dwErr);
		return false;
	}
		
	m_sd_send = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
	if (m_sd_send == INVALID_SOCKET) {
		_d(_T("[send] Failed to create socket\n"));
		return false;
	}
	
	return true;
}

void CTnmComm::Run()
{
	/*DWORD_PTR dOldMask = SetThreadAffinityMask(GetCurrentThread(), 0);
	SetThreadAffinityMask(GetCurrentThread(), dOldMask);*/

	struct sockaddr_in sin;
		
	BYTE *pRecv = new BYTE[188 * 70 + 12];

	DWORD dwErr;
	DWORD dwStart = GetTickCount();
	DWORD dwTick = dwStart;

	bool bProbePacket = false; // 패킷의 크기 및 헤더 유무를 판단하기 위한 플래그
	bool bIsHeader = false; // 패킷에 12byte RTP 헤더가 있는지 여부

	int nBitrate = 0;
	int nQueued = 0;

	m_nBitrate = 0;
	
	memset(&sin, 0, sizeof(struct sockaddr_in));
	
	_d(_T("[Recv] Start receive thread. ip: %s, port: %d\n"), m_strIpAddr, m_recv_port);
	while (!m_bExit) {
		int nRecvLen;
		int nFromLen;		

		nFromLen = sizeof(struct sockaddr_in);
		nRecvLen = recvfrom(m_sd_recv, (char*)pRecv, RECV_SIZE, 0, (sockaddr*)&sin, &nFromLen);

		if (nRecvLen < 0) {
			dwErr = GetLastError();
			//_d(_T("[Recv] Receive error : %d, Error code : %x(%d)\n"), nRecvLen, dwErr, WSAGetLastError());

			m_connected = false;			
			Sleep(1);
			continue;
		} 

		_d("[Recv] Receive data : %d\n", nRecvLen);		
				
		nBitrate += nRecvLen * 8;

		if ((GetTickCount() - dwTick) >= 1000) {
			if (nBitrate) {
				m_nBitrate = nBitrate;
			}
			dwTick = GetTickCount();

			nBitrate = 0;
		}
	}
	_d(_T("[Recv] End of receive thread\n"));
		
	if (pRecv) {
		delete[] pRecv;
	}
}

