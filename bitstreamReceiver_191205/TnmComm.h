#pragma once

class CTnmComm : public CTnmThread
{
public:
	CTnmComm();
	~CTnmComm();

	void StartRecv(CString strIP, int send_port, int recv_port);
	void StopRecv();
	
	bool is_connected() { return m_connected; };

	/* 
	* cmd. 1: start, 0: stop
	* codec. 1 HEVC, 2: H264
	* bitrate. 256 ~ 10240 kbps 
	*/
	void set_conversion_cfg(int cmd, int codec, int bitrate);
	
	/*
	* mode. 1: panorama, 2: focus
	*/
	void set_analog_cfg(int mode);

	void set_distribution_cfg(byte* poutput);

	/*
	* nType. 0: 상시, 1: 이벤트
	* mux_cfg. 비디오 채널 설정
	* file_name. 이벤트 녹화시 파일명
	*/
	void set_save_start(int nType, byte enable, mux_cfg_s *mux_cfg, char *file_name);
	void set_save_stop(int nType);

protected:
	void Run();

private:
	bool Connect();
	void Disconnect();

	bool CreateSocket();

	bool send_data(byte* pdata, int size);
	
private:
	bool m_connected;
		
	int m_nRecvSize;
			
private:	
	SOCKET m_sd_send;
	SOCKET m_sd_recv;
	CString m_strIpAddr;	
	int m_send_port;
	int m_recv_port;

	int m_nBitrate;	
};

