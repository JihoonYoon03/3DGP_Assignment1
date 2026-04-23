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

	void SetMovingDirection(const XMFLOAT3& xmf3MovingDirection) { m_xmf3MovingDirection = Vector3::Normalize(xmf3MovingDirection); };
	void SetMovingSpeed(float fSpeed) { m_fMovingSpeed = fSpeed; }
	void SetMovingRange(float fRange) { m_fMovingRange = fRange; }

	void SetRotationAxis(const XMFLOAT3& xmf3RotationAxis) { m_xmf3RotationAxis = Vector3::Normalize(xmf3RotationAxis); };
	void SetRotationSpeed(float fSpeed) { m_fRotationSpeed = fSpeed; }

	void LookTo(XMFLOAT3& xmf3LookTo, XMFLOAT3& xmf3Up);
	void LookAt(const XMFLOAT3& xmf3LookAt, const XMFLOAT3& xmf3Up);

	void SetWorldMatrix(const XMFLOAT4X4& matrix) { m_xmf4x4World = matrix; }
	
	void Move(XMFLOAT3& xmf3Direction, float fSpeed);

	void Rotate(float fPitch = 10.f, float fYaw = 10.f, float fRoll = 10.f);
	void Rotate(XMFLOAT3& xmf3Axis, float fAngle);

	virtual void OnUpdateTransform() { }
	
	void UpdateBoundingBox();

	virtual void Animate(float fElapsedTime);
	bool FrustumCullingTest(CCamera* pCamera);
	virtual void Render(HDC hDCFrameBuffer, CCamera* pCamera);
	virtual void Render(HDC hDCFrameBuffer, CCamera* pCamera, XMFLOAT4X4* pxmf4x4World, CMesh* pMesh);

	void GenerateRayForPicking(XMVECTOR& xmvPickPosition, XMMATRIX& xmmtxView, XMVECTOR& xmvPickRayOrigin, XMVECTOR& xmvPickRayDirection);
	int PickObjectByRayIntersection(XMVECTOR& xmPickPosition, XMMATRIX& xmmtxView, float& pfHitDistance);

	virtual void EventCollision(CGameObject* objCollided, const eObjType objType) {}
	virtual void EventPicking() {}

	XMFLOAT3 GetPosition();
	const XMFLOAT4X4& GetWorldMatrix() const { return m_xmf4x4World; }
	const BoundingOrientedBox& GetOOBB() const { return m_xmOOBB; }
	const bool& isActive() const { return m_bActive; }

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

class CExplosiveObject : public CGameObject
{
public:
	CExplosiveObject();
	virtual ~CExplosiveObject();

	bool						m_bBlowingUp = false;

	XMFLOAT4X4					m_pxmf4x4Transforms[EXPLOSION_DEBRISES];

	float						m_fElapsedTimes = 0.0f;
	float						m_fDuration = 2.0f;
	float						m_fExplosionSpeed = 10.0f;
	float						m_fExplosionRotation = 720.0f;

	void Animate(float fElapsedTime) override;
	void Render(HDC hDCFrameBuffer, CCamera* pCamera) override;
	void EventCollision(CGameObject* objCollided, const eObjType objType) override;

public:
	static CMesh* m_pExplosionMesh;
	static XMFLOAT3				m_pxmf3SphereVectors[EXPLOSION_DEBRISES];

	static void PrepareExplosion();
};


class CWallsObject : public CGameObject
{
public:
	CWallsObject();
	virtual ~CWallsObject();

public:
	BoundingOrientedBox			m_xmOOBBPlayerMoveCheck = BoundingOrientedBox();
	XMFLOAT4					m_pxmf4WallPlanes[6];

	void Render(HDC hDCFrameBuffer, CCamera* pCamera) override;
};

class CBulletObject : public CGameObject
{
public:
	CBulletObject(float fEffectiveRange);
	virtual ~CBulletObject();

	void SetFirePosition(XMFLOAT3 xmf3FirePosition);

	void SetLockedObject(CGameObject* object) { m_pLockedObject = object; }

	void Animate(float fElapsedTime) override;
	void EventCollision(CGameObject* objCollided, const eObjType objType) override;

private:
	float						m_fBulletEffectiveRange = 50.0f;
	float						m_fMovingDistance = 0.0f;
	float						m_fRotationAngle = 0.0f;
	XMFLOAT3					m_xmf3FirePosition = XMFLOAT3(0.0f, 0.0f, 1.0f);

	float						m_fElapsedTimeAfterFire = 0.0f;
	float						m_fLockingDelayTime = 0.3f;
	float						m_fLockingTime = 4.0f;

	CGameObject* m_pLockedObject = nullptr;

	void Reset();
};

class CUIObject : public CGameObject {
public:
	CUIObject();
	virtual ~CUIObject();

	void setCheckMouseHover(bool set) { m_bCheckMouseHover = set; }

	virtual void Render(HDC hDCFrameBuffer, CCamera* pCamera) override;
	virtual void Render(HDC hDCFrameBuffer, CCamera* pCamera, XMFLOAT4X4* pxmf4x4World, CMesh* pMesh) override;

	virtual void EventBeginMouseHovering() { if (m_bCheckMouseHover) m_bMouseHover = true; }
	virtual void EventEndMouseHovering() { m_bMouseHover = false; }
private:
	bool	m_bCheckMouseHover = false;
	bool	m_bMouseHover = false;

	HPEN hPenPicked = NULL;
	HBRUSH hBrushPicked = NULL;
};