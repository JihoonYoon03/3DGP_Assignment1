#pragma once

#include "GameObject.h"
#include "Camera.h"

class CPlayer :public CGameObject {
public:
	CPlayer() {}
	virtual ~CPlayer();
	
	void SetPosition(float x, float y, float z);
	//void SetRotation(float x, float y, float z);

	// 바라보는 지점 입력받아 기저벡터 갱신
	void LookAt(XMFLOAT3& xmf3LookAt, XMFLOAT3& xmf3Up);

	// 방향키 입력 이동 1개, 수치 입력 이동 2개
	void Move(DWORD dwDirection, float fDistance);
	void Move(const XMFLOAT3& xmf3Shift, bool bUpdateVelocity);
	void Move(float x, float y, float z);

	void Rotate(float fPitch = 0.f, float fYaw = 0.f, float fRoll = 0.f);
	
	void SetCameraOffset(const XMFLOAT3& xmf3CameraOffset);

	void Update(float fTimeElapsed = 0.016f);

	virtual void OnUpdateTransform();
	virtual void Animate(float fElapsedTime);

	void SetCamera(CCamera* pCamera) { m_pCamera = pCamera; }
	CCamera* GetCamera() { return m_pCamera; }

	const XMFLOAT3& GetPosition() const { return m_xmf3Position; }

	const XMFLOAT3& GetRight() const { return m_xmf3Right; }
	const XMFLOAT3& GetUp() const { return m_xmf3Up; }
	const XMFLOAT3& GetLook() const { return m_xmf3Look; }

	const XMFLOAT3& GetCameraOffset() const { return m_xmf3CameraOffset; }

private:
	XMFLOAT3	m_xmf3Position = XMFLOAT3(0.0f, 0.0f, 0.0f);
	XMFLOAT3	m_xmf3Right = XMFLOAT3(1.0f, 0.0f, 0.0f);
	XMFLOAT3	m_xmf3Up = XMFLOAT3(0.0f, 1.0f, 0.0f);
	XMFLOAT3	m_xmf3Look = XMFLOAT3(0.0f, 0.0f, 1.0f);

	XMFLOAT3	m_xmf3CameraOffset = XMFLOAT3(0.0f, 0.0f, 0.0f);
	XMFLOAT3	m_xmf3Velocity = XMFLOAT3(0.0f, 0.0f, 0.0f);

	float		m_fFriction = 125.0f;

	float       m_fPitch = 0.0f;
	float       m_fYaw = 0.0f;
	float       m_fRoll = 0.0f;

	CCamera*	m_pCamera = nullptr;
};

class CAirplanePlayer : public CPlayer {
public:
	CAirplanePlayer();
	virtual ~CAirplanePlayer();

	void FireBullet(CGameObject* pLockedObject, std::vector<CGameObject*>& vBullets);
	const unsigned int getMaxAmmo() const { return m_ammo; }
	const float getRange() const { return m_bulletRange; }

	virtual void OnUpdateTransform();
	virtual void Animate(float fElapsedTime);
	virtual void Render(HDC hDCFrameBuffer, CCamera* pCamera);

private:
	float			m_bulletRange = 150.f;
	unsigned int	m_ammo = 50;
};

