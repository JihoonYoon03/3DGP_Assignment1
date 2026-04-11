#pragma once

#include "Mesh.h"

class CGameObject
{
public:
	CGameObject() {}
	~CGameObject();

	void SetMesh(CMesh* pMesh);
	void SetColor(DWORD dwColor);
	void SetPosition(float x, float y, float z);
	void SetRotation(float x, float y, float z);
	void SetRotationSpeed(float x, float y, float z);
	
	void Move(float x, float y, float z);
	void Rotate(float x, float y, float z);

	// 메쉬 정점 하나를 게임 객체 위치와 방향을 통해 월드 좌표 변환을 한다
	CPoint3D WorldTransform(CPoint3D& f3Model);

	virtual void Animate(float fElapsedTime);
	virtual void Render(HDC hDCFrameBuffer);

private:
	// 게임 객체의 월드 위치
	float m_fxPosition = 0.f;
	float m_fyPosition = 0.f;
	float m_fzPosition = 0.f;

	// 회전량(반시계 방향)
	float m_fxRotation = 0.f;
	float m_fyRotation = 0.f;
	float m_fzRotation = 0.f;

	// 회전 속도
	float m_fxRotationSpeed = 0.f;
	float m_fyRotationSpeed = 0.f;
	float m_fzRotationSpeed = 0.f;

	// 모양(메쉬/모델)
	CMesh* m_pMesh = nullptr;
	// 게임 객체의 색상이다.
	DWORD	m_dwColor = RGB(255, 0, 0);

};

