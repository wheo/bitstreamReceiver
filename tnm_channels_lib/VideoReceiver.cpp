#include "stdafx.h"
#include "VideoReceiver.h"
//#define RECV_SIZE			(188*70+12) // lcj
#define RECV_SIZE			4096 + 16 // lcj
#define QUEUE_SIZE			1

#define ENABLE_RTP_MPEGTS	0

CVideoReceiver::CVideoReceiver(int nIndex)
{
	m_index = nIndex;
	m_nSrcHeight = 1080;
	m_nSrcWidth = 1920;

	m_sdUDP = INVALID_SOCKET;
	m_sdUDPSlave = INVALID_SOCKET;

	m_bMasterIsConnected = true;

	m_nBitrate = 0;

	mp_ring = NULL;

	m_first_packet = false;

	mp_packet_data = new byte[MAX_IMAGE_WIDTH * MAX_IMAGE_HEIGHT * 4];

	m_connected = false;

	m_recv_frame_count = 0;
}


CVideoReceiver::~CVideoReceiver()
{
	StopRecv();

	delete[] mp_packet_data;
}


void CVideoReceiver::StartRecv(CString strMasterIP, CString strSlaveIP, CString strAdaterIP, int nPort)
{
	StopRecv();

	m_strIpAddr.Format(_T("%s"), strMasterIP);
	m_strSlaveAddr.Format(_T("%s"), strSlaveIP);
	m_strAdapterIP.Format(_T("%s"), strAdaterIP);
	m_nPort = nPort;

	m_first_packet = false;
	m_nRecvSize = 0;
	m_nPacketSize = 0;

	m_connected = false;

	if (Connect()) {
		mp_ring = new CMyRing(188 * 65535 * 32);

		Start();
		//	SetThreadMAXPriority();
	}
}

void CVideoReceiver::StopRecv()
{
	Terminate();

	Disconnect();

	if (mp_ring) {
		mp_ring->SetExit();
	}
	SAFE_DELETE(mp_ring);
}

void CVideoReceiver::Disconnect()
{
	SOCKET * in_socket;

	if (m_bMasterIsConnected)
	{
		in_socket = &m_sdUDP;
	}
	else
	{
		in_socket = &m_sdUDPSlave;
	}
	if (*in_socket != INVALID_SOCKET) {
		closesocket(*in_socket);
	}
	*in_socket = INVALID_SOCKET;
}


bool CVideoReceiver::Connect()
{
	SOCKET * in_socket;
	CString in_strIpAddr;

	if (m_bMasterIsConnected)
	{
		in_socket = &m_sdUDP;
		in_strIpAddr = m_strIpAddr;
		_d(_T("[Recv.%d] connect ip : %s, adapterip: %s, port : %d\n"), g_nIndex, in_strIpAddr, m_strAdapterIP, m_nPort);		
	}
	else
	{
		in_socket = &m_sdUDPSlave;
		in_strIpAddr = m_strSlaveAddr;
		_d(_T("[Recv.%d] connect ip : %s, adapterip: %s, port : %d\n"), g_nIndex, in_strIpAddr, m_strAdapterIP, m_nPort);
	}

	m_strCurIP.Format(_T("%s"), in_strIpAddr);
	if (CreateSocket(*in_socket, in_strIpAddr) == false) {
		return false;
	}

	return true;
}
bool CVideoReceiver::CreateSocket(SOCKET & in_socket, CString in_strIpAddr)
{
	if (in_socket != INVALID_SOCKET)
	{
		_d(_T("[Recv] socket is not INVALID_SOCKET\n"));
		return false;
	}

	SOCKADDR_IN addr;

	int nState;

	DWORD dwErr;

	in_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
	if (in_socket == INVALID_SOCKET) {
		_d(_T("[Recv] Failed to create socket\n"));
		return false;
	}

	nState = 1;
	if (setsockopt(in_socket, SOL_SOCKET, SO_REUSEADDR, (const char*)&nState, sizeof(nState)) < 0) {
		closesocket(in_socket);
		dwErr = GetLastError();
		_d(_T("[Recv] Failed to set sockopt : REUSEADDR, %x\n"), dwErr);
		return false;
	}

#if 1
	nState = 3000;
	if (setsockopt(in_socket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&nState, sizeof(struct timeval)) < 0) {
		_d(_T("[Recv] 소켓옵션 설정 실패\n"));
		return false;
	}
#endif

	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(m_nPort);
	if (bind(in_socket, (SOCKADDR*)&addr, sizeof(addr)) == SOCKET_ERROR) {
		closesocket(in_socket);
		dwErr = GetLastError();
		_d(_T("[Recv] Failed to bind, %x\n"), dwErr);
		return false;
	}

	CString m_strEthernetIP = _T("");
	//	m_strEthernetIP.Format("%s", "192.168.2.36");	
	CString strUnicast = _T("@");

	if (in_strIpAddr.Find('@') < 0) {
		ip_mreq joinAddr;
		joinAddr.imr_multiaddr.s_addr = inet_addr(in_strIpAddr); // 멀티캐스트 그룹 주소 반드시 클래스 D 그룹 IP 어야 한다.
		//> 161110 - 멀티케스트 수신할 어뎁터 선택해서 연결하도록 수정
		if (m_strEthernetIP.IsEmpty()) {
			joinAddr.imr_interface.s_addr = htonl(INADDR_ANY); // 자기 자신을 알려줌	
		}
		else {
			joinAddr.imr_interface.s_addr = inet_addr(m_strEthernetIP);
		}

		// 소켓 옵션 설정. IP_ADD_MEMBERSHIP : 멀티캐스트 그룹에 가입
		if (setsockopt(in_socket, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*)&joinAddr, sizeof(joinAddr)) < 0) {
			closesocket(in_socket);
			dwErr = GetLastError();
			_d(_T("[Recv] Failed to add membership: %s, Ethernetip: %s, err: %x\n"), in_strIpAddr, m_strEthernetIP, dwErr);
			return false;
		}
	}

	return true;
}

bool CVideoReceiver::CloseSocket(SOCKET &in_socket)
{
	if (in_socket != INVALID_SOCKET) {
		closesocket(in_socket);
	}
	in_socket = INVALID_SOCKET;

	return true;
}

bool CVideoReceiver::ReCreateSocket()
{
	_d(_T("[Recv.%d] ReCreateSocket\n"), m_index);

	if (SocketIsConnected())
	{
		Disconnect();

		Sleep(1000);

		Connect();

		return true;
	}
	else
	{
		Connect();

		return false;
	}
}

bool CVideoReceiver::SocketIsConnected()
{
	SOCKET in_socket;
	if (m_bMasterIsConnected)
	{
		in_socket = m_sdUDP;
	}
	else
	{
		in_socket = m_sdUDPSlave;
	}

	if (in_socket == INVALID_SOCKET)
	{
		return false;
	}
	else
	{
		return true;
	}
}

void CVideoReceiver::Run()
{
	/*DWORD_PTR dOldMask = SetThreadAffinityMask(GetCurrentThread(), 0);
	SetThreadAffinityMask(GetCurrentThread(), dOldMask);*/

	struct sockaddr_in sin;

	//BYTE *pRecv = new BYTE[188 * 70 + 12];
	BYTE *pRecv = new BYTE[RECV_SIZE];

	DWORD dwErr;
	DWORD dwStart = GetTickCount();
	DWORD dwTick = dwStart;

	bool bProbePacket = false; // 패킷의 크기 및 헤더 유무를 판단하기 위한 플래그
	bool bIsHeader = false; // 패킷에 12byte RTP 헤더가 있는지 여부

	int nBitrate = 0;
	int nQueued = 0;

	m_nBitrate = 0;
	m_recv_frame_count = 0;

	int frame_count = 0;

	memset(&sin, 0, sizeof(struct sockaddr_in));
	sin.sin_family = PF_INET;
	sin.sin_port = htons(m_nPort);
	sin.sin_addr.s_addr = htonl(INADDR_ANY);

	if (m_bMasterIsConnected)
	{
		_d(_T("[Recv] Start receive thread. ip: %s, port: %d\n"), m_strIpAddr, m_nPort);
	}
	else
	{
		_d(_T("[Recv] Start receive thread. ip: %s, port: %d\n"), m_strSlaveAddr, m_nPort);
	}

	while (!m_bExit) {

		int nRecvLen;
		int nFromLen;

		if (SocketIsConnected())
		{
			nFromLen = sizeof(struct sockaddr_in);
			if (m_bMasterIsConnected)
			{
				nRecvLen = recvfrom(m_sdUDP, (char*)pRecv, RECV_SIZE, 0, (sockaddr*)&sin, &nFromLen);
			}
			else
			{
				nRecvLen = recvfrom(m_sdUDPSlave, (char*)pRecv, RECV_SIZE, 0, (sockaddr*)&sin, &nFromLen);
			}

			if (nRecvLen < 0) {
				dwErr = GetLastError();
				_d(_T("[Recv.%d] Receive error : %d, Error code : %x(%d)\n"), m_index, nRecvLen, dwErr, WSAGetLastError());

				m_connected = false;
				m_nBitrate = 0;
				m_recv_frame_count = 0;

				if (m_bMasterIsConnected)
				{
					Disconnect();
					m_bMasterIsConnected = false;
					Connect();
				}
				else
				{
					Disconnect();
					m_bMasterIsConnected = true;
					Connect();
				}

				continue;
			}
		}
		else
		{
			if (m_bMasterIsConnected)
			{
				CreateSocket(m_sdUDP, m_strIpAddr);
				CloseSocket(m_sdUDPSlave);
			}
			else
			{
				CreateSocket(m_sdUDPSlave, m_strSlaveAddr);
				CloseSocket(m_sdUDP);
			}

			continue;
		}

		//		_d("[Recv] Receive data : %d\n", nRecvLen);		

#if ENABLE_RTP_MPEGTS
		//// 1. RTP Header 처리
		if (!bProbePacket) {
			int nResidue = nRecvLen % 188;
			//	int nPackets = (nRecvLen - nResidue)/188;
			if (nResidue) {
				bIsHeader = true;
			}
			bProbePacket = true;
		}

		byte *p = pRecv;
		nDataSize = nRecvLen;
		if (bIsHeader) {
			nDataSize = nRecvLen - 12;
			p += 12;

			RTPHeaderParsing((char*)pRecv, nRecvLen);
		}

		//// 2. TS Header 처리
	//	TSHeaderParsing(p, nDataSize);
#endif
		if (TNMHeaderParsing(pRecv, nRecvLen) == 1) {
			frame_count++;
		}

		nBitrate += nRecvLen * 8;

		if ((GetTickCount() - dwTick) >= 1000) {
			if (nBitrate) {
				m_nBitrate = nBitrate;
			}
			dwTick = GetTickCount();

			nBitrate = 0;

			m_recv_frame_count = frame_count;
			frame_count = 0;
		}
	}
	_d(_T("[Recv] End of receive thread\n"));

	if (pRecv) {
		delete[] pRecv;
	}
}

int CVideoReceiver::TNMHeaderParsing(byte* pdata, int datasize)
{
	int recv_nTotalStreamSize = 0;
	int recv_nCurStreamSize = 0;
	int nTotalPacketNum = 0;
	int nCurPacketNum = 0;

	memcpy(&recv_nTotalStreamSize, pdata, 4);
	memcpy(&recv_nCurStreamSize, pdata + 4, 4);
	memcpy(&nTotalPacketNum, pdata + 8, 4);
	memcpy(&nCurPacketNum, pdata + 12, 4);
	if (m_first_packet == false) {
		if (nCurPacketNum != 1) {
			return -1;
		}
		else {
			m_nRecvSize = 0;
			m_nPacketSize = 0;
			m_first_packet = true;
		}
	}

	memcpy(&mp_packet_data[m_nRecvSize], pdata + 16, recv_nCurStreamSize);

	m_nRecvSize += recv_nCurStreamSize;

	if (nTotalPacketNum == nCurPacketNum) {
		m_nPacketSize = m_nRecvSize;
#if 0
		if (m_fpWrite) {
			fwrite(m_bitstream[m_write_bitstream_count], 1, m_bitstreamSize[m_write_bitstream_count], m_fpWrite);
		}

		m_is_bitstream_full[m_write_bitstream_count++] = true;
		SetEvent(m_hEvent);

		if (m_write_bitstream_count == MAX_NUM_bitstream)
		{
			m_write_bitstream_count = 0;
		}

		recv_nestedStreamSize = 0;
		nPrevPacketNum = 0;
#endif
		//	_d(_T("[ch.%d] make frame. sise: %d\n"), m_index, m_nPacketSize);
		if (mp_ring) {
			mp_ring->Write((uint8_t*)&m_nPacketSize, 4);
			mp_ring->Write(mp_packet_data, m_nRecvSize);
		}

		m_connected = true;

		m_first_packet = false;

		return 1;
	}

	return 0;
}
int	CVideoReceiver::TSHeaderParsing(byte* p, int nDataSize)
{
	//// sync byte check
	if (p[0] != 0x47) {
		_d(_T("[Recv] Receive error.. Can't find 0x47 in ts header\n"));
		return -1;
	}

	return 1;
}


int CVideoReceiver::RTPHeaderParsing(char *pData, int nDataSize)
{
	int nOffset = 0;

	RTP_HEADER *p = (RTP_HEADER*)pData;
	RTP_HEADER curHeader = { 0, };
	char *pCurData = pData;

	int nHeaderSize = sizeof(RTP_HEADER);
	int nPayloadSize = nDataSize - nHeaderSize;

	if (nDataSize < nHeaderSize) {
		return 0;
	}

	curHeader.sequence_num = ntohs(p->sequence_num);
	curHeader.timestamp = ntohl(p->timestamp);
	curHeader.ssrc = ntohl(p->ssrc);

	_d(_T("[Recv] rtp header.. mark: %d, seq : %d, timestampe: %d\n"), p->marker, curHeader.sequence_num - m_rtp_header.sequence_num, curHeader.timestamp - m_rtp_header.timestamp);

	// check RTP version (it must be 2)
	if (p->version != 2) {
		_d(_T("[Recv] rtp header parsing failed.. invalid rtp version: %d\n"), p->version);
		return 0;
	}

	// skip CSRC
	if (p->cc > 0) {
#if 0
		if (nPayloadSize <= p->cc * 4) {
			_d("[Recv] rtp header parsing failed.. CSRC count error : %d\n", p->cc);
			return 0;
		}
		else {
			pCurData += (p->cc * 4);
			nOffset += (p->cc * 4);
		}
#endif
		_d(_T("[Recv] >>>>>>>>> CSRC in rtp header\n"));
	}

	// skip Extension field
	if (p->extension > 0) {
		_d(_T("[Recv] >>>>>>>>> extension field in rtp header\n"));
	}

	// remove padding
	if (p->padding > 0) {
		_d(_T("[Recv] >>>>>>>>> padding in rtp header\n"));
	}

	memcpy(&m_rtp_header, pData, nHeaderSize);
	m_rtp_header.sequence_num = curHeader.sequence_num;
	m_rtp_header.timestamp = curHeader.timestamp;
	m_rtp_header.ssrc = curHeader.ssrc;

	return nOffset;
}

int CVideoReceiver::GetImageData(byte *pRGB32)
{
	int nSize = 0;
	if (mp_ring) {
		int buf_size = 0;
		int nRead = mp_ring->Read((uint8_t*)&buf_size, 4, false);
		if (nRead > 0 && buf_size > 0) {
			while (1) {
				nRead = mp_ring->Read(pRGB32, buf_size, false);
				if (nRead > 0) {
					nSize = nRead;
					break;
				}
				else {
					Sleep(1);
					if (m_connected == false) {
						break;
					}
					else {
						continue;
					}
				}
			}
		}
	}
	return nSize;
}