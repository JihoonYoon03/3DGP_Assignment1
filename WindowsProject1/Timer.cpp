#include "framework.h"
#include "Timer.h"

CTimer::CTimer()
	: m_DeltaTime(0.0f)
{
	QueryPerformanceFrequency(&m_Frequency);
	QueryPerformanceCounter(&m_PrevTime);
}


CTimer& CTimer::GetInstance()
{
	static CTimer instance;
	return instance;
}

void CTimer::Update()
{
	LARGE_INTEGER currentTime;
	QueryPerformanceCounter(&currentTime);

	m_DeltaTime =
		static_cast<float>(currentTime.QuadPart - m_PrevTime.QuadPart)
		/ static_cast<float>(m_Frequency.QuadPart);

	m_PrevTime = currentTime;
}
