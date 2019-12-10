#pragma once
#define TNM_CODEC_ID_H264	0
#define TNM_CODEC_ID_HEVC	1
#define TNM_CODEC_ID_JPEG	2

#define MAX_VIDEO_CHANNEL_COUNT 6
#define MAX_AUDIO_CHANNEL_COUNT 2

#define RECORD_PORT 0
#define EVENT_PORT 1
#define PLAY_PORT 2
#define NUM_CONTROL_DEVICE 3

typedef struct {
	int codec;		//> 0- H264, 1- HEVC
	int profile;
	int level;
	int quality;
	int resolution;
	int width;
	int height;
	int rc;
	int bitrate;
	double fps;
	int min_gop;
	int max_gop;
	int bframes;
	int refs;
	int ratio_x;
	int ratio_y;

	bool is_cabac;
	bool is_interlace;
	bool is_dummy1;
	bool is_dummy2;
}video_cfg_s;
typedef struct {
	int output;	//> 0- ts, 1- mp4
	int mux_rate;

	video_cfg_s vid;
}mux_cfg_s;

typedef struct {
	TCHAR strIP[100];
	int nPort;
}channel_s;
typedef struct {
	TCHAR strIP[100];
	int nPort[NUM_CONTROL_DEVICE];
}control_s;
typedef struct {
	int channel_count;
	channel_s vid_ch[2];	
	channel_s aud_ch[2];
	mux_cfg_s mux;
}channel_cfg_s;


typedef struct {
	TCHAR strIP[100];
	int nPort;
}stream_s;

typedef struct {
	control_s ctr_channel;
	channel_s vid_channel[MAX_VIDEO_CHANNEL_COUNT];
	channel_s aud_channel[MAX_AUDIO_CHANNEL_COUNT];	

	int nRecvPort;
}conversion_cfg_s;
