#pragma once
class Receiver : public JThread
{
public:
	Receiver(uint32_t portNum, const TCHAR* cp);
	Receiver(channel_cfg_s ch_cfg);
	~Receiver();

	int readSocket(uint8_t* buffer, unsigned bufferSize);

	void* Thread();

	void close_event_handle();

	void ReleaseResources();

private:
	int setupDatagramSocket(uint32_t portNum, const TCHAR* cp);

public:
	uint8_t* m_bitstream[MAX_NUM_bitstream];
	unsigned int m_read_bitstream_count;
	unsigned int m_write_bitstream_count;
	bool m_is_bitstream_full[MAX_NUM_bitstream];
	int m_bitstreamSize[MAX_NUM_bitstream];

	int m_EvenOrOdd[MAX_NUM_bitstream];

	unsigned long long lStart, lEnd, lFreq;
	double elapsedTime;

	HANDLE m_hEvent;

	float m_bitrate_per_sec;
	unsigned int count_bitrate;

private:
	channel_cfg_s m_ch_cfg;

	int fSocket;
	struct sockaddr_in fDest;
	struct sockaddr_in fSource;
	struct sockaddr_in fFromAddress;
	int fFromAddressSize;
};

