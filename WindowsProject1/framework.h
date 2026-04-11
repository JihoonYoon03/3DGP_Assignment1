// header.h: 표준 시스템 포함 파일
// 또는 프로젝트 특정 포함 파일이 들어 있는 포함 파일입니다.
//

#pragma once

#include "targetver.h"
#define WIN32_LEAN_AND_MEAN             // 거의 사용되지 않는 내용을 Windows 헤더에서 제외합니다.
// Windows 헤더 파일
#include <windows.h>

// 간단한 사운드 출력, 시스템 시간 확인용 (Multimedia system)
#include <Mmsystem.h>
#pragma comment(lib, "winmm.lib")

// DX 관련 헤더 파일
#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include <DirectXColors.h>
#include <DirectXCollision.h>

// C++ 런타임 헤더 파일
#include <memory>
#include <string>
#include <vector>

constexpr int FRAMEBUFFER_WIDTH{ 640 };
constexpr int FRAMEBUFFER_HEIGHT{ 480 };

#define DegreeToRadian(x)	float((x) * 3.141592654f / 180.0f)

#define GET_SINGLE(type)	type::GetInstance()