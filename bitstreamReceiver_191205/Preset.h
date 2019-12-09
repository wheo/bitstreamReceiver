#pragma once

//#define inputWidth 1920
//#define inputHeight 1080

#define inputWidth 720
#define inputHeight 240

#define renderWidth 720
#define renderHeight 480

#define PACKET_SIZE 4096

#define CODEC 2 // 0:H264, 1:JPEG2000, 2:HEVC 

#define ENCODER 0

#define MAX_NUM_Q 4

#define MAX_NUM_bitstream MAX_NUM_Q

#define EVEN 1
#define ODD	0

typedef struct
{
	unsigned width;
	unsigned height;
	unsigned bitrate;
	unsigned quality;
} Preset;

template<class Interface>
inline void SafeRelease(Interface** ppInterfaceToRelease)
{
	if (*ppInterfaceToRelease != NULL)
	{
		(*ppInterfaceToRelease)->Release();
		*ppInterfaceToRelease = NULL;
	}
}

#ifndef HINST_THISCOMPONENT
EXTERN_C IMAGE_DOS_HEADER __ImageBase;
#define HINST_THISCOMPONENT ((HINSTANCE)&__ImageBase)
#endif // !HINST_THISCOMPONENT