// stdafx.h : 자주 사용하지만 자주 변경되지는 않는
// 표준 시스템 포함 파일 또는 프로젝트 관련 포함 파일이
// 들어 있는 포함 파일입니다.

#pragma once

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN            // 거의 사용되지 않는 내용은 Windows 헤더에서 제외합니다.
#endif

#include "targetver.h"

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS      // 일부 CString 생성자는 명시적으로 선언됩니다.

#include <afxwin.h>         // MFC 핵심 및 표준 구성 요소입니다.
#include <afxext.h>         // MFC 확장입니다.

#ifndef _AFX_NO_OLE_SUPPORT
#include <afxole.h>         // MFC OLE 클래스입니다.
#include <afxodlgs.h>       // MFC OLE 대화 상자 클래스입니다.
#include <afxdisp.h>        // MFC 자동화 클래스입니다.
#endif // _AFX_NO_OLE_SUPPORT

#ifndef _AFX_NO_DB_SUPPORT
#include <afxdb.h>                      // MFC ODBC 데이터베이스 클래스입니다.
#endif // _AFX_NO_DB_SUPPORT

#ifndef _AFX_NO_DAO_SUPPORT
#include <afxdao.h>                     // MFC DAO 데이터베이스 클래스입니다.
#endif // _AFX_NO_DAO_SUPPORT

#ifndef _AFX_NO_OLE_SUPPORT
#include <afxdtctl.h>           // Internet Explorer 4 공용 컨트롤에 대한 MFC 지원입니다.
#endif
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>                     // Windows 공용 컨트롤에 대한 MFC 지원입니다.
#endif // _AFX_NO_AFXCMN_SUPPORT


#include "TnmUtility.h"
#include "Ctnm_channel.h"

#define MAX_IMAGE_WIDTH		1920
#define MAX_IMAGE_HEIGHT	1080

#define MAX_DETECTION_COUNT	50

///> 탐지결과
typedef struct tagDetectionInfo
{
	int nIndex;	//> 파노라믹을 위한 준비
	int nTargetCount;
	int nx[MAX_DETECTION_COUNT];
	int ny[MAX_DETECTION_COUNT];
	int nWidth[MAX_DETECTION_COUNT];
	int nHeight[MAX_DETECTION_COUNT];
}detection_Info_s;

typedef struct
{
	unsigned width;
	unsigned height;
	unsigned bitrate;
	unsigned quality;
} Preset;

/* 제어비트 - 16 bit
* version - 2bit (2, RFC 3550)
* padding - 1bit
* extension - 1bit : 가변길이 헤더 확장 (Extension Header)여부
* cc (CSRC Count) - 4bit. 기본 헤더 바로 뒤에 나타나는 CSRC (Countributing Source) ID의 갯수.
* Marker - 1bit. 이벤트 발생이 시작되었음을 알림
* 유료부하 타입 (Payload type) - 7bit 오디오/비디오 코덱 종류
*   오디오 0 ~ 15
*   비디오
*   26 : 화상 JPEG, 31 : H.261, 32 : MPEG-1 or MPEG-2 video, 33 : MPEG-2 TS 등
*   기타 임의 지정 (dynamic payload type) : 96 ~ 127 */

/* Sequence number - 16bit : 초기값은 랜덤. 이후 매 패킷마다 1씩 증가*/

/* Timestamp - 32bit : 초기값 랜덤. 통상적으로 카운터에 의해 1씩 증가*/

/* 동기 발신 식별자 (SSRC ID, Synchroniszation Source ID) - 32bit*/

typedef struct
{
	WORD cc : 4;      /* csrc count */
	WORD extension : 1;      /* header extension flag */
	WORD padding : 1;      /* padding flag - for encryption */
	WORD version : 2;      /* protocal version */
	WORD payload_type : 7;      /* payload type */
	WORD marker : 1;      /* marker bit - for profile */
	WORD sequence_num;   /* sequence number of this packet */
	DWORD timestamp;    /* timestamp of this packet */
	DWORD ssrc;   /* source of packet */
} RTP_HEADER;

typedef struct
{
	unsigned char sync_byte;
	unsigned char transport_error_indicator;
	unsigned char payload_start_indicator;
	unsigned char transport_priority;
	unsigned int PID;
	unsigned char transport_scrambling_control;
	unsigned char adaption_field_control;
	unsigned char continuity_counter;
} TS_HEADER;

struct ImageYUV420Planar
{
	int	nWidth;
	int nHeight;

	int nStrideY;
	int	nStrideUV;

	unsigned char *pY;
	unsigned char *pU;
	unsigned char *pV;
};

#include <D2D1.h>
#include <D2D1Helper.h>
#include <DWrite.h>
#include <Wincodec.h>

static ID2D1Factory  *gp_D2DFactory = NULL;
static IDWriteFactory	*gp_WriteFactory = NULL;

extern int g_nIndex;