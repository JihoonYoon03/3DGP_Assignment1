#pragma once

#include "Mesh.h"
#include "Camera.h"

class CGameObject
{
public:
	CGameObject() {}
	virtual ~CGameObject();

	void SetActive(bool bActive) { m_bActive = bActive; }

	void SetMesh(CMesh* pMesh);
	void SetColor(DWORD dwColor) { m_dwColor = dwColor; }

	void SetPosition(float x, float y, float z);
	void SetPosition(XMFLOAT3& xmf3Position);
	
	void SetMovingDirection(const XMFLOAT3& xmf3MovingDirection);
	void SetMovingSpeed(float fSpeed) { m_fMovingSpeed = fSpeed; }
	void SetMovingRange(float fRange) { m_fMovingRange = fRange; }

	void SetRotationAxis(const XMFLOAT3& xmf3RotationAxis);
	void SetRotationSpeed(float fSpeed) { m_fRotationSpeed = fSpeed; }

	const XMFLOAT4X4& GetWorldMatrix() const { return m_xmf4x4World; }
	
	void Move(XMFLOAT3& xmf3Direction, float fSpeed);

	void Rotate(float fPitch = 10.f, float fYaw = 10.f, float fRoll = 10.f);
	void Rotate(XMFLOAT3& xmf3Axis, float fAngle);

	virtual void OnUpdateTransform() { }
	
	void UpdateBoundingBox();

	virtual void Animate(float fElapsedTime);
	virtual void Render(HDC hDCFrameBuffer, CCamera* pCamera);
	virtual void Render(HDC hDCFrameBuffer, CCamera* pCamera, CMesh* pMesh);

protected:
	bool						m_bActive = true;

	XMFLOAT4X4					m_xmf4x4World = Matrix4x4::Identity();

	XMFLOAT3					m_xmf3MovingDirection = XMFLOAT3(0.0f, 0.0f, 1.0f);
	float						m_fMovingSpeed = 0.0f;
	float						m_fMovingRange = 0.0f;

	XMFLOAT3					m_xmf3RotationAxis = XMFLOAT3(0.0f, 1.0f, 0.0f);
	float						m_fRotationSpeed = 0.0f;

	// 모양(메쉬/모델)
	CMesh*						m_pMesh = nullptr;

	BoundingOrientedBox			m_xmOOBB = BoundingOrientedBox();
	CGameObject*				m_pObjectCollided = nullptr;

	// 게임 객체의 색상이다.
	DWORD						m_dwColor = RGB(255, 0, 0);

	HPEN hPen = NULL;
	HBRUSH hBrush = NULL;

};

