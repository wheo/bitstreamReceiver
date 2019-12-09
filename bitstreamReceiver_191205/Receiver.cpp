#include "stdafx.h"
#include "Receiver.h"

Receiver::Receiver(channel_cfg_s ch_cfg)
	:
	m_write_bitstream_count(0),
	m_read_bitstream_count(0)
{

	memcpy(&m_ch_cfg, &ch_cfg, sizeof(channel_cfg_s));

	fSocket = setupDatagramSocket(m_ch_cfg.nPort, m_ch_cfg.strIP);
	if (fSocket < 0)
		exit(-1);

	for (unsigned int i = 0; i < MAX_NUM_bitstream; ++i) {
		m_bitstream[i] = new uint8_t[m_ch_cfg.mux.vid.width * m_ch_cfg.mux.vid.height];
		if (m_bitstream[i] == NULL) {
			_d(_T("failed to m_bitstream alloc\n"));
			ReleaseResources();
			exit(-1);
		}

		m_is_bitstream_full[i] = false;
	}

	count_bitrate = 0;

	Start();
}
Receiver::Receiver(uint32_t portNum, const TCHAR* cp)
	:
	m_write_bitstream_count(0),
	m_read_bitstream_count(0)
{
	fSocket = setupDatagramSocket(portNum, cp);
	if (fSocket < 0) 
		exit(-1);

	for (unsigned int i = 0; i < MAX_NUM_bitstream; ++i) {
		m_bitstream[i] = new uint8_t[inputWidth * inputHeight];
		if (m_bitstream[i] == NULL) {
			_d(_T("failed to m_bitstream alloc\n"));
			ReleaseResources();
			exit(-1);
		}

		m_is_bitstream_full[i] = false;
	}

	count_bitrate = 0;
	
	Start();
}

Receiver::~Receiver()
{	
	ReleaseResources();
}

int Receiver::readSocket(uint8_t * buffer, unsigned bufferSize)
{
	memset(&fFromAddress, 0, sizeof(struct sockaddr_in));
	fFromAddressSize = sizeof(fFromAddress);
	int bytesRead = recvfrom(fSocket, (char*)buffer, bufferSize, 0, (sockaddr*)&fFromAddress, &fFromAddressSize);
	if (bytesRead < 0) {
		printf("recvfrom() error: ");
	}

	return bytesRead;
}

void * Receiver::Thread()
{	
	ThreadStarted();

	m_hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

	uint8_t buffer[PACKET_SIZE + 16];

	int nTotalPacketNum;
	int nCurPacketNum = 1;

	int recv_nTotalStreamSize;
	int recv_nCurStreamSize;

	int recv_nestedStreamSize = 0;

	int nPrevPacketNum = 0;

	//FILE * m_fpWrite = fopen("C:/yuv/1080p_yuv420p.x264", "wb");

	while (IsRunning() == true) {
		if (nCurPacketNum == 1) {
			TnmQueryPerformanceCounter(&lStart);
		}

		if (readSocket(buffer, PACKET_SIZE + 16) > 0) {
			if (m_is_bitstream_full[m_write_bitstream_count] == false) {
				memcpy(&recv_nTotalStreamSize, buffer, 4);
				memcpy(&recv_nCurStreamSize, buffer + 4, 4);
				memcpy(&nTotalPacketNum, buffer + 8, 4);
				memcpy(&nCurPacketNum, buffer + 12, 4);
				memcpy(m_bitstream[m_write_bitstream_count] + recv_nestedStreamSize, buffer + 16, PACKET_SIZE);

				if ((nPrevPacketNum + 1) != nCurPacketNum) {
					//_d(_T("\nnTotalPacketNum(%d)PrevPacketNum(%d)/nCurPacketNum(%d)\n\n"), nTotalPacketNum, nPrevPacketNum, nCurPacketNum);	
				}
				nPrevPacketNum = nCurPacketNum;

				//_d(_T("nTotalPacketNum(%d)/nCurPacketNum(%d)\n"), nTotalPacketNum, nCurPacketNum);
				
				recv_nestedStreamSize += recv_nCurStreamSize;

				if (nTotalPacketNum == nCurPacketNum) {
					//_d(_T("recv_nTotalStreamSize(%d)/recv_nestedStreamSize(%d)\n"), recv_nTotalStreamSize, recv_nestedStreamSize);
					m_bitstreamSize[m_write_bitstream_count] = recv_nestedStreamSize;
					count_bitrate += recv_nestedStreamSize;
							
					//if (m_fpWrite) {
					//	fwrite(m_bitstream[m_write_bitstream_count], 1, m_bitstreamSize[m_write_bitstream_count], m_fpWrite);
					//}
					
					//_d(_T("m_write_bitstream_count(%d)\n"), m_write_bitstream_count);
					m_is_bitstream_full[m_write_bitstream_count++] = true;
					SetEvent(m_hEvent);
					//TnmQueryPerformanceCounter(&lStart);
					
					if (m_write_bitstream_count == MAX_NUM_bitstream) {
						m_write_bitstream_count = 0;
					}
					//TnmQueryPerformanceCounter(&lEnd);
					//elapsedTime = (double)(lEnd - lStart);
					//NvQueryPerformanceFrequency(&lFreq);
					//_d(_T("%f\n"), (elapsedTime*1000.0) / lFreq);

					recv_nestedStreamSize = 0;
					nPrevPacketNum = 0;
				}
			}
			else {
				//_d(_T("m_is_bitstream_full[%d]\n"), m_write_bitstream_count);
			}
		}
	}

	SetEvent(m_hEvent);
	
	return nullptr;
}

void Receiver::close_event_handle()
{
	CloseHandle(m_hEvent);
}

void Receiver::ReleaseResources()
{
	Kill();

	if (fSocket > 0) {
		closesocket(fSocket);
		fSocket = -1;
	}

	for (unsigned int i = 0; i < MAX_NUM_bitstream; ++i) {
		if (m_bitstream[i]) {
			delete[] m_bitstream[i];
			m_bitstream[i] = NULL;
		}
	}	
}

int Receiver::setupDatagramSocket(uint32_t portNum, const TCHAR * cp)
{
	int newSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
	if (newSocket < 0)
	{
		printf("unable to create datagram socket: ");
		return newSocket;
	}

	int reuseFlag = 1;
	if (setsockopt(newSocket, SOL_SOCKET, SO_REUSEADDR,
		(const char*)&reuseFlag, sizeof reuseFlag) < 0)
	{
		printf("setsockopt(SO_REUSEADDR) error: ");
		closesocket(newSocket);
		return -1;
	}

	int rcvbuf_size = 1040 * 800/*128*/;
	if (newSocket != INVALID_SOCKET) {
		if (setsockopt(newSocket, SOL_SOCKET, SO_RCVBUF, (const char*)&rcvbuf_size, sizeof(rcvbuf_size)) < 0) {
			DWORD dwErr = GetLastError();
			_d(_T("setsockopt(SO_RCVBUF) (%x)\n"), dwErr);
			closesocket(newSocket);
			newSocket = INVALID_SOCKET;
		}
	}

	int nTime = 500;
	if (setsockopt(newSocket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&nTime, sizeof(struct timeval)) < 0) {
		_d(L"[Recv] 家南可记 汲沥 角菩\n");
		return false;
	}

	struct in_addr addr;
	char strIP[200];

	WideToMulti((wchar_t*)cp, strIP);

	addr.s_addr = inet_addr(strIP);

	fSource.sin_family = AF_INET;
	fSource.sin_addr.s_addr = htonl(INADDR_ANY);
	fSource.sin_port = htons(portNum);
	if (bind(newSocket, (const sockaddr*)&fSource, sizeof(fSource)) != 0)
	{
		printf("bind() error (port number: %d): ",
			ntohs(portNum));
		closesocket(newSocket);
		return -1;
	}

	struct ip_mreq imr;
	imr.imr_multiaddr.s_addr = addr.s_addr;
	imr.imr_interface.s_addr = htonl(INADDR_ANY);

	if (setsockopt(newSocket, IPPROTO_IP, IP_ADD_MEMBERSHIP,
		(const char*)&imr, sizeof(struct ip_mreq)) < 0)
	{
		printf("setsockopt(IP_ADD_MEMBERSHIP) error: ");
		closesocket(newSocket);
		return -1;
	}

	return newSocket;
}


#if 0
//ThreadStarted();

	//m_hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

	//uint8_t* buffer = new uint8_t[1040];

	//int nTotalPacketNum;
	//int nCurPacketNum = 1;

	//int recv_nTotalStreamSize;
	//int recv_nCurStreamSize;

	//int recv_nestedStreamSize = 0;

	//int nPrevPacketNum = 0;

	//int nEvenAndOdd;

	//while (IsRunning() == true)
	//{
	//	//if (nCurPacketNum == 1)
	//	//{
	//	//	TnmQueryPerformanceCounter(&lStart);
	//	//}

	//	if (readSocket(buffer, 1024) > 0)
	//	{
	//		if (m_is_bitstream_full[m_write_bitstream_count] == false)
	//		{

	//			recv_nCurStreamSize = (((buffer[4] & 0x03) << 8) | buffer[5]);
	//			nTotalPacketNum = buffer[3];
	//			nCurPacketNum = buffer[2];
	//			nEvenAndOdd = buffer[1];
	//			memcpy(m_bitstream[m_write_bitstream_count] + recv_nestedStreamSize, buffer + 8, 1016);

	//			if (nCurPacketNum == 1 && nEvenAndOdd == EVEN)
	//			{
	//				TnmQueryPerformanceCounter(&lStart);
	//			}

	//			if ((nPrevPacketNum + 1) != nCurPacketNum)
	//			{
	//				//_d(_T("\nnTotalPacketNum(%d)PrevPacketNum(%d)/nCurPacketNum(%d)\n\n"), nTotalPacketNum, nPrevPacketNum, nCurPacketNum);	
	//			}
	//			nPrevPacketNum = nCurPacketNum;

	//			//_d(_T("nEvenAndOdd(%d)\n"), nEvenAndOdd);
	//			//_d(_T("nTotalPacketNum(%d)/nCurPacketNum(%d)\n"), nTotalPacketNum, nCurPacketNum);
	//			//_d(_T("recv_nCurStreamSize(%d)\n"), recv_nCurStreamSize);



	//			recv_nestedStreamSize += recv_nCurStreamSize;

	//			if (nTotalPacketNum == nCurPacketNum)
	//			{
	//				//_d(_T("recv_nTotalStreamSize(%d)/recv_nestedStreamSize(%d)\n"), recv_nTotalStreamSize, recv_nestedStreamSize);
	//				m_bitstreamSize[m_write_bitstream_count] = recv_nestedStreamSize;


	//				//_d(_T("m_write_bitstream_count(%d)\n"), m_write_bitstream_count);
	//				m_EvenOrOdd[m_write_bitstream_count] = nEvenAndOdd;
	//				m_is_bitstream_full[m_write_bitstream_count++] = true;
	//				SetEvent(m_hEvent);

	//				if (nEvenAndOdd == ODD)
	//				{
	//					TnmQueryPerformanceCounter(&lEnd);
	//					elapsedTime = (double)(lEnd - lStart);
	//					NvQueryPerformanceFrequency(&lFreq);
	//					_d(_T("%f\n"), (elapsedTime*1000.0) / lFreq);
	//				}

	//				if (m_write_bitstream_count == MAX_NUM_bitstream)
	//				{
	//					m_write_bitstream_count = 0;
	//				}


	//				recv_nestedStreamSize = 0;
	//				nPrevPacketNum = 0;


	//			}
	//		}
	//		else
	//		{
	//			//_d(_T("m_is_bitstream_full[%d]\n"), m_write_bitstream_count);
	//		}
	//	}
	//}

	//SetEvent(m_hEvent);

	//delete buffer;

	//return nullptr;









	//ThreadStarted();

	//m_hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

	//uint8_t* buffer = new uint8_t[1040];

	//int nTotalPacketNum;
	//int nCurPacketNum = 1;

	//int recv_nTotalStreamSize;
	//int recv_nCurStreamSize;

	//int recv_nestedStreamSize = 0;

	//int nPrevPacketNum = 0;

	//int nEvenAndOdd;

	//while (IsRunning() == true)
	//{
	//	//if (nCurPacketNum == 1)
	//	//{
	//	//	TnmQueryPerformanceCounter(&lStart);
	//	//}

	//	if (readSocket(buffer, 1024) > 0)
	//	{
	//		if (m_is_bitstream_full[m_write_bitstream_count] == false)
	//		{

	//			recv_nCurStreamSize = (((buffer[4] & 0x03) << 8) | buffer[5]);
	//			nTotalPacketNum = buffer[3];
	//			nCurPacketNum = buffer[2];
	//			nEvenAndOdd = buffer[1];
	//			memcpy(m_bitstream[m_write_bitstream_count] + recv_nestedStreamSize, buffer + 8, 1016);

	//			if (nCurPacketNum == 1)
	//			{
	//				TnmQueryPerformanceCounter(&lStart);
	//			}

	//			if ((nPrevPacketNum + 1) != nCurPacketNum)
	//			{
	//				//_d(_T("\nnTotalPacketNum(%d)PrevPacketNum(%d)/nCurPacketNum(%d)\n\n"), nTotalPacketNum, nPrevPacketNum, nCurPacketNum);	
	//			}
	//			nPrevPacketNum = nCurPacketNum;

	//			//_d(_T("nEvenAndOdd(%d)\n"), nEvenAndOdd);
	//			//_d(_T("nTotalPacketNum(%d)/nCurPacketNum(%d)\n"), nTotalPacketNum, nCurPacketNum);
	//			//_d(_T("recv_nCurStreamSize(%d)\n"), recv_nCurStreamSize);



	//			recv_nestedStreamSize += recv_nCurStreamSize;

	//			if (nTotalPacketNum == nCurPacketNum)
	//			{
	//				//_d(_T("recv_nTotalStreamSize(%d)/recv_nestedStreamSize(%d)\n"), recv_nTotalStreamSize, recv_nestedStreamSize);
	//				m_bitstreamSize[m_write_bitstream_count] = recv_nestedStreamSize;


	//				//_d(_T("m_write_bitstream_count(%d)\n"), m_write_bitstream_count);
	//				m_is_bitstream_full[m_write_bitstream_count++] = true;
	//				SetEvent(m_hEvent);
	//				//TnmQueryPerformanceCounter(&lStart);


	//				if (m_write_bitstream_count == MAX_NUM_bitstream)
	//				{
	//					m_write_bitstream_count = 0;
	//				}
	//				//TnmQueryPerformanceCounter(&lEnd);
	//				//elapsedTime = (double)(lEnd - lStart);
	//				//NvQueryPerformanceFrequency(&lFreq);
	//				//_d(_T("%f\n"), (elapsedTime*1000.0) / lFreq);

	//				recv_nestedStreamSize = 0;
	//				nPrevPacketNum = 0;


	//			}
	//		}
	//		else
	//		{
	//			//_d(_T("m_is_bitstream_full[%d]\n"), m_write_bitstream_count);
	//		}
	//	}
	//}

	//SetEvent(m_hEvent);

	//delete buffer;

	//return nullptr;
#endif