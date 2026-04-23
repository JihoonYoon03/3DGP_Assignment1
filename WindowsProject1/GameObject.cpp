#include "framework.h"
#include "GameObject.h"
#include "GraphicsPipeline.h"
#include "GameVar.h"
#include "Scene.h"

std::uniform_real_distribution<float> disFloat{ -1.f, 1.f };

XMVECTOR RandomUnitVectorOnSphere()
{
	XMVECTOR xmvOne = XMVectorSet(1.0f, 1.0f, 1.0f, 1.0f);

	while (true)
	{
		XMVECTOR v = XMVectorSet(disFloat(rde), disFloat(rde), disFloat(rde), 0.0f);
		if (!XMVector3Greater(XMVector3LengthSq(v), xmvOne)) return(XMVector3Normalize(v));
	}
}

CGameObject::~CGameObject()
{
	if (m_pMesh) m_pMesh->Release();

	if (hPen)	::DeleteObject(hPen);
	if (hBrush) ::DeleteObject(hBrush);
}

void CGameObject::SetMesh(CMesh* pMesh)
{
	m_pMesh = pMesh;
	if (pMesh) pMesh->AddRef();
}

void CGameObject::SetPosition(float x, float y, float z)
{
	m_xmf4x4World._41 = x;
	m_xmf4x4World._42 = y;
	m_xmf4x4World._43 = z;
}

void CGameObject::SetPosition(XMFLOAT3& xmf3Position)
{
	m_xmf4x4World._41 = xmf3Position.x;
	m_xmf4x4World._42 = xmf3Position.y;
	m_xmf4x4World._43 = xmf3Position.z;
}

XMFLOAT3 CGameObject::GetPosition()
{
	return XMFLOAT3(m_xmf4x4World._41, m_xmf4x4World._42, m_xmf4x4World._43);
}

void CGameObject::Move(XMFLOAT3& xmf3Direction, float fSpeed)
{
	SetPosition(
		m_xmf4x4World._41 + xmf3Direction.x * fSpeed,
		m_xmf4x4World._42 + xmf3Direction.y * fSpeed,
		m_xmf4x4World._43 + xmf3Direction.z * fSpeed
	);
}

void CGameObject::LookTo(XMFLOAT3& xmf3LookTo, XMFLOAT3& xmf3Up)
{
	XMFLOAT4X4 xmf4x4View = Matrix4x4::LookToLH(GetPosition(), xmf3LookTo, xmf3Up);
	m_xmf4x4World._11 = xmf4x4View._11; m_xmf4x4World._12 = xmf4x4View._21; m_xmf4x4World._13 = xmf4x4View._31;
	m_xmf4x4World._21 = xmf4x4View._12; m_xmf4x4World._22 = xmf4x4View._22; m_xmf4x4World._23 = xmf4x4View._32;
	m_xmf4x4World._31 = xmf4x4View._13; m_xmf4x4World._32 = xmf4x4View._23; m_xmf4x4World._33 = xmf4x4View._33;
}

void CGameObject::LookAt(const XMFLOAT3& xmf3LookAt, const XMFLOAT3& xmf3Up)
{
	XMFLOAT4X4 xmf4x4View = Matrix4x4::LookAtLH(GetPosition(), xmf3LookAt, xmf3Up);
	m_xmf4x4World._11 = xmf4x4View._11; m_xmf4x4World._12 = xmf4x4View._21; m_xmf4x4World._13 = xmf4x4View._31;
	m_xmf4x4World._21 = xmf4x4View._12; m_xmf4x4World._22 = xmf4x4View._22; m_xmf4x4World._23 = xmf4x4View._32;
	m_xmf4x4World._31 = xmf4x4View._13; m_xmf4x4World._32 = xmf4x4View._23; m_xmf4x4World._33 = xmf4x4View._33;
}

void CGameObject::Rotate(float fPitch, float fYaw, float fRoll)
{
	XMMATRIX xmmRotate = XMMatrixRotationRollPitchYaw(
		XMConvertToRadians(fPitch), XMConvertToRadians(fYaw), XMConvertToRadians(fRoll));
	XMStoreFloat4x4(&m_xmf4x4World, XMMatrixMultiply(xmmRotate, XMLoadFloat4x4(&m_xmf4x4World)));
}

void CGameObject::Rotate(XMFLOAT3& xmf3RotationAxis, float fAngle)
{
	XMMATRIX xmmRotate = XMMatrixRotationAxis(
		XMLoadFloat3(&xmf3RotationAxis), XMConvertToRadians(fAngle));
	XMStoreFloat4x4(&m_xmf4x4World, XMMatrixMultiply(xmmRotate, XMLoadFloat4x4(&m_xmf4x4World)));
}

void CGameObject::Animate(float fElapsedTime)
{
	if (m_fRotationSpeed != 0.0f) Rotate(m_xmf3RotationAxis, m_fRotationSpeed * fElapsedTime);
	if (m_fMovingSpeed != 0.0f) Move(m_xmf3MovingDirection, m_fMovingSpeed * fElapsedTime);

	UpdateBoundingBox();
}

void CGameObject::UpdateBoundingBox()
{
	if (m_pMesh) {
		m_pMesh->m_xmOOBB.Transform(m_xmOOBB, XMLoadFloat4x4(&m_xmf4x4World));
		XMStoreFloat4(&m_xmOOBB.Orientation, XMQuaternionNormalize(XMLoadFloat4(&m_xmOOBB.Orientation)));
	}
}

bool CGameObject::FrustumCullingTest(CCamera* pCamera) {
	return pCamera->IsInFrustum(m_xmOOBB);
}

// render called by instance
void CGameObject::Render(HDC hDCFrameBuffer, CCamera* pCamera)
{
	CGameObject::Render(hDCFrameBuffer, pCamera, &m_xmf4x4World, m_pMesh);
}

void CGameObject::Render(HDC hDCFrameBuffer, CCamera* pCamera, XMFLOAT4X4* pxmf4x4World, CMesh* pMesh)
{
	if (pMesh) {
		CGraphicsPipeline::SetWorldTransform(pxmf4x4World);
		
		XMMATRIX mtxWorldInv = XMMatrixInverse(nullptr, XMLoadFloat4x4(pxmf4x4World));
		XMVECTOR vLocalCameraPos = XMVector3TransformCoord(XMLoadFloat3(&pCamera->GetPosition()), mtxWorldInv);

		if (not hPen) {
			hPen = ::CreatePen(PS_SOLID, 0, m_dwColor);
		}
		if (not hBrush) {
			hBrush = ::CreateSolidBrush(m_dwColor);
			//hBrush = ::CreateSolidBrush(RGB(255, 255, 255));
		}
		
		HPEN hOldPen = (HPEN)::SelectObject(hDCFrameBuffer, hPen);
		HBRUSH hOldBrush = (HBRUSH)::SelectObject(hDCFrameBuffer, hBrush);

		pMesh->Render(hDCFrameBuffer, pCamera, vLocalCameraPos);

		::SelectObject(hDCFrameBuffer, hOldPen);
		::SelectObject(hDCFrameBuffer, hOldBrush);
	}
}

void CGameObject::GenerateRayForPicking(XMVECTOR& xmvPickPosition, XMMATRIX& xmmtxView, XMVECTOR& xmvPickRayOrigin, XMVECTOR& xmvPickRayDirection)
{
	XMMATRIX xmmtxToModel = XMMatrixInverse(NULL, XMLoadFloat4x4(&m_xmf4x4World) * xmmtxView);

	XMFLOAT3 xmf3CameraOrigin(0.0f, 0.0f, 0.0f);
	xmvPickRayOrigin = XMVector3TransformCoord(XMLoadFloat3(&xmf3CameraOrigin), xmmtxToModel);
	xmvPickRayDirection = XMVector3TransformCoord(xmvPickPosition, xmmtxToModel);
	xmvPickRayDirection = XMVector3Normalize(xmvPickRayDirection - xmvPickRayOrigin);
}

int CGameObject::PickObjectByRayIntersection(XMVECTOR& xmvPickPosition, XMMATRIX& xmmtxView, float& fHitDistance)
{
	bool nIntersected = false;

	if (m_pMesh) {
		XMVECTOR xmvPickRayOrigin, xmvPickRayDirection;
		GenerateRayForPicking(xmvPickPosition, xmmtxView, xmvPickRayOrigin, xmvPickRayDirection);
		nIntersected = m_pMesh->CheckRayIntersection(xmvPickRayOrigin, xmvPickRayDirection, fHitDistance);
	}

	return nIntersected;
}

// ===========================================================================
CWallsObject::CWallsObject()
// ===========================================================================
{

}

CWallsObject::~CWallsObject()
{

}

void CWallsObject::Render(HDC hDCFrameBuffer, CCamera* pCamera)
{
	CGameObject::Render(hDCFrameBuffer, pCamera, &m_xmf4x4World, m_pMesh);
}



// ===========================================================================
XMFLOAT3 CExplosiveObject::m_pxmf3SphereVectors[EXPLOSION_DEBRISES];
CMesh* CExplosiveObject::m_pExplosionMesh = nullptr;

CExplosiveObject::CExplosiveObject()
{

}

CExplosiveObject::~CExplosiveObject()
{

}

void CExplosiveObject::PrepareExplosion()
{
	for (int i = 0; i < EXPLOSION_DEBRISES; i++)
		XMStoreFloat3(&m_pxmf3SphereVectors[i], RandomUnitVectorOnSphere());

	m_pExplosionMesh = new CCubeMesh(0.5f, 0.5f, 0.5f);
}

void CExplosiveObject::Animate(float fElapsedTime)
{
	if (m_bBlowingUp)
	{
		m_fElapsedTimes += fElapsedTime;

		// 파티클 lifetime동안 파티클 별 방향으로 이동 및 방향벡터 기준 회전
		if (m_fElapsedTimes <= m_fDuration)	{
			XMFLOAT3 xmf3Position = GetPosition();
			for (int i = 0; i < EXPLOSION_DEBRISES; i++) {
				m_pxmf4x4Transforms[i] = Matrix4x4::Identity();
				m_pxmf4x4Transforms[i]._41 = xmf3Position.x + m_pxmf3SphereVectors[i].x * m_fExplosionSpeed * m_fElapsedTimes;
				m_pxmf4x4Transforms[i]._42 = xmf3Position.y + m_pxmf3SphereVectors[i].y * m_fExplosionSpeed * m_fElapsedTimes;
				m_pxmf4x4Transforms[i]._43 = xmf3Position.z + m_pxmf3SphereVectors[i].z * m_fExplosionSpeed * m_fElapsedTimes;
				m_pxmf4x4Transforms[i] = Matrix4x4::Multiply(Matrix4x4::RotationAxis(m_pxmf3SphereVectors[i], m_fExplosionRotation * m_fElapsedTimes), m_pxmf4x4Transforms[i]);
			}
		}
		else {
			m_bBlowingUp = false;
			m_fElapsedTimes = 0.0f;
			XMFLOAT3 xmf3Position = GetPosition();
			for (int i = 0; i < EXPLOSION_DEBRISES; i++) {
				m_pxmf4x4Transforms[i]._41 = xmf3Position.x;
				m_pxmf4x4Transforms[i]._42 = xmf3Position.y;
				m_pxmf4x4Transforms[i]._43 = xmf3Position.z;
			}
		}
	}
	else
	{
		CGameObject::Animate(fElapsedTime);
	}
}

void CExplosiveObject::Render(HDC hDCFrameBuffer, CCamera* pCamera)
{
	if (m_bBlowingUp) {
		for (int i = 0; i < EXPLOSION_DEBRISES; i++) {
			// TODO: 파티클 출력 시스템 분리
			CGameObject::Render(hDCFrameBuffer, pCamera, &m_pxmf4x4Transforms[i], m_pExplosionMesh);
		}
	}
	else {
		CGameObject::Render(hDCFrameBuffer, pCamera, &m_xmf4x4World, m_pMesh);
	}
}

void CExplosiveObject::EventCollision(CGameObject* objCollided, const eObjType objType)
{
	if (objCollided && objType == eObjType::Bullet) {
		OutputDebugStringW(L"Explosive Collided\n");
		m_bBlowingUp = true;
	}
}


// ===========================================================================
CBulletObject::CBulletObject(float fEffectiveRange)
// ===========================================================================
{
	m_fBulletEffectiveRange = fEffectiveRange;
}

CBulletObject::~CBulletObject()
{
}

void CBulletObject::SetFirePosition(XMFLOAT3 xmf3FirePosition)
{
	m_xmf3FirePosition = xmf3FirePosition;
	SetPosition(xmf3FirePosition);
}

void CBulletObject::Reset()
{
	m_pLockedObject = nullptr;
	m_fElapsedTimeAfterFire = 0;
	m_fMovingDistance = 0;
	m_fRotationAngle = 0.0f;

	m_bActive = false;
}

void CBulletObject::Animate(float fElapsedTime)
{
	m_fElapsedTimeAfterFire += fElapsedTime;

	float fDistance = m_fMovingSpeed * fElapsedTime;

	if ((m_fElapsedTimeAfterFire > m_fLockingDelayTime) && m_pLockedObject)
	{
		XMFLOAT3 xmf3Position = GetPosition();
		XMVECTOR xmvPosition = XMLoadFloat3(&xmf3Position);

		XMFLOAT3 xmf3LockedObjectPosition = m_pLockedObject->GetPosition();
		XMVECTOR xmvLockedObjectPosition = XMLoadFloat3(&xmf3LockedObjectPosition);
		// 락온 대상까지의 방향 벡터
		XMVECTOR xmvToLockedObject = XMVectorSubtract(xmvLockedObjectPosition, xmvPosition);
		xmvToLockedObject = XMVector3Normalize(xmvToLockedObject);

		XMVECTOR xmvMovingDirection = XMLoadFloat3(&m_xmf3MovingDirection);
		// 현재 이동방향 벡터와 목표물 방향 벡터를 선형 보간 후 정규화
		xmvMovingDirection = XMVector3Normalize(XMVectorLerp(xmvMovingDirection, xmvToLockedObject, 0.25f));
		// 갱신
		XMStoreFloat3(&m_xmf3MovingDirection, xmvMovingDirection);
	}

	// 총알 회전 효과
	XMFLOAT4X4 mtxRotate = Matrix4x4::RotationYawPitchRoll(0.0f, 0.0f, m_fRotationSpeed * fElapsedTime);
	m_xmf4x4World = Matrix4x4::Multiply(mtxRotate, m_xmf4x4World);

	XMFLOAT3 xmf3Movement = Vector3::ScalarProduct(m_xmf3MovingDirection, fDistance, false);
	XMFLOAT3 xmf3Position = GetPosition();
	xmf3Position = Vector3::Add(xmf3Position, xmf3Movement);
	SetPosition(xmf3Position);
	m_fMovingDistance += fDistance;

	UpdateBoundingBox();

	// 총알 사거리 벗어나거나 lifetime 끝인 경우 리셋
	if ((m_fMovingDistance > m_fBulletEffectiveRange) || (m_fElapsedTimeAfterFire > m_fLockingTime)) Reset();
}

void CBulletObject::EventCollision(CGameObject* objCollided, const eObjType objType)
{
	if (objCollided && objType == eObjType::Explosive) {
		OutputDebugStringW(L"Bullet Collided\n");
		this->Reset();
	}
}

// ===========================================================================

CUIObject::CUIObject()
{
}

CUIObject::~CUIObject()
{
	if (hPenPicked)	::DeleteObject(hPenPicked);
	if (hBrushPicked)	::DeleteObject(hBrushPicked);
}

// render called by instance
void CUIObject::Render(HDC hDCFrameBuffer, CCamera* pCamera)
{
	CUIObject::Render(hDCFrameBuffer, pCamera, &m_xmf4x4World, m_pMesh);
}

void CUIObject::Render(HDC hDCFrameBuffer, CCamera* pCamera, XMFLOAT4X4* pxmf4x4World, CMesh* pMesh)
{
	if (pMesh) {
		CGraphicsPipeline::SetWorldTransform(pxmf4x4World);

		XMMATRIX mtxWorldInv = XMMatrixInverse(nullptr, XMLoadFloat4x4(pxmf4x4World));
		XMVECTOR vLocalCameraPos = XMVector3TransformCoord(XMLoadFloat3(&pCamera->GetPosition()), mtxWorldInv);

		if (not hPen) {
			hPen = ::CreatePen(PS_SOLID, 0, m_dwColor);
		}
		if (not hBrush) {
			hBrush = ::CreateSolidBrush(m_dwColor);
			//hBrush = ::CreateSolidBrush(RGB(255, 255, 255));
		}

		if (not hPenPicked) {
			hPenPicked = ::CreatePen(PS_SOLID, 0, RGB(255, 255, 255) - m_dwColor);
		}
		if (not hBrushPicked) {
			hBrushPicked = ::CreateSolidBrush(RGB(255, 255, 255) - m_dwColor);
			//hBrush = ::CreateSolidBrush(RGB(255, 255, 255));
		}

		HPEN hOldPen = NULL;
		HBRUSH hOldBrush = NULL;

		if (m_bMouseHover) {
			hOldPen = (HPEN)::SelectObject(hDCFrameBuffer, hPenPicked);
			hOldBrush = (HBRUSH)::SelectObject(hDCFrameBuffer, hBrushPicked);
		}
		else {
			hOldPen = (HPEN)::SelectObject(hDCFrameBuffer, hPen);
			hOldBrush = (HBRUSH)::SelectObject(hDCFrameBuffer, hBrush);
		}

		pMesh->Render(hDCFrameBuffer, pCamera, vLocalCameraPos);

		::SelectObject(hDCFrameBuffer, hOldPen);
		::SelectObject(hDCFrameBuffer, hOldBrush);
	}
}

void CUIObject::EventPicking()
{
	if (m_OnClickCallback) {
		m_OnClickCallback();
	}
}