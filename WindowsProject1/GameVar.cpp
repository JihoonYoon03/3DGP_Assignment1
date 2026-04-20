#include "framework.h"
#include "GameVar.h"

std::random_device rd;
std::mt19937 rde{ rd() };

// 마우스 포인터 위치
POINT		oldCursorPos{};