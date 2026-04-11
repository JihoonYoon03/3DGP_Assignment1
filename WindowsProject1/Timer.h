#pragma once


class CTimer {
public:
	static CTimer& GetInstance();

	// 싱글톤, 복사및 복사할당 금지
	CTimer(const CTimer&) = delete;
	CTimer& operator=(const CTimer&) = delete;

	void Update();

	float GetDeltaTime() const { return m_DeltaTime; };

private:
	CTimer();

	LARGE_INTEGER m_Frequency;
	LARGE_INTEGER m_PrevTime;

	float m_DeltaTime;
};

#define DELTA_TIME			GET_SINGLE(CTimer).GetDeltaTime()