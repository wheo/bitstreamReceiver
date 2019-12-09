#pragma once

class CVideoReceiver : public CVideoThread
{
public:
	CVideoReceiver(int nIndex);
	~CVideoReceiver();

	//void StartRecv(CString strIP, CString strAdaterIP, int nPort);
	void StartRecv(CString strMasterIP, CString strSlaveIP, CString strAdaterIP, int nPort);
	void StopRecv();

	int GetImageData(byte *pRGB32);
	
	CString get_cur_connected_ip() { return m_strCurIP; };
	bool is_connected() { return m_connected; };
	int get_recv_data() { return m_nBitrate; };
	int get_recv_frame() { return m_recv_frame_count; };

protected:
	void Run();

private:
	bool Connect();
	void Disconnect();

	//bool CreateSocket();
	bool CreateSocket(SOCKET & in_socket, CString in_strIpAddr);
	bool CloseSocket(SOCKET & in_socket);

	bool ReCreateSocket();

	bool SocketIsConnected();

	int RTPHeaderParsing(char *pData, int nDataSize);
	int	TSHeaderParsing(byte* p, int nDataSize);
	int TNMHeaderParsing(byte* pdata, int datasize);
private:
	int m_index;
	int m_nSrcWidth;
	int m_nSrcHeight;

	bool m_connected;

	CMyRing			*mp_ring;

	bool m_first_packet;
	int m_nRecvSize;
	int m_nPacketSize;
	byte *mp_packet_data;

	int m_recv_frame_count;
		
private:
	//> rtp
	SOCKET m_sdUDP;
	SOCKET m_sdUDPSlave;
	CString m_strIpAddr;
	CString m_strSlaveAddr;
	CString m_strAdapterIP;
	CString m_strCurIP;
	int m_nPort;

	int m_nBitrate;

	RTP_HEADER	m_rtp_header;

	bool m_bMasterIsConnected;
};

