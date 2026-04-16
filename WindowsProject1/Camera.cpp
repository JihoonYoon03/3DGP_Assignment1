#include "framework.h"
#include "Camera.h"
#include "Player.h"

// ======================================
CViewport::CViewport(int nLeft, int nTop, int nWidth, int nHeight)
// ======================================
	: m_nLeft{ nLeft }, m_nTop{ nTop }, m_nWidth{ nWidth }, m_nHeight{ nHeight }
{

}

void CViewport::SetViewport(int nLeft, int nTop, int nWidth, int nHeight)
{
	m_nLeft = nLeft;
	m_nTop = nTop;
	m_nWidth = nWidth;
	m_nHeight = nHeight;
}


// ======================================
CCamera::CCamera()
// ======================================
{

}

CCamera::~CCamera()
{

}

void CCamera::GenerateViewMatrix()
{
	XMVECTOR xmvPos = XMLoadFloat3(&m_xmf3Position);
	XMVECTOR xmvLook = XMVector3Normalize(XMLoadFloat3(&m_xmf3Look));
	XMVECTOR xmvUp = XMVector3Normalize(XMLoadFloat3(&m_xmf3Up));
	XMVECTOR xmvRight = XMVector3Normalize(XMVector3Cross(xmvUp, xmvLook));
	xmvUp = XMVector3Normalize(XMVector3Cross(xmvLook, xmvRight));

	XMStoreFloat3(&m_xmf3Look, xmvLook);
	XMStoreFloat3(&m_xmf3Right, xmvRight);
	XMStoreFloat3(&m_xmf3Up, xmvUp);

	XMMATRIX xmmView = XMMatrixLookToLH(xmvPos, xmvLook, xmvUp);
	XMStoreFloat4x4(&m_xmf4x4View, xmmView);

	XMMATRIX xmmPersProj = XMLoadFloat4x4(&m_xmf4x4PerspectiveProject);
	XMStoreFloat4x4(
		&m_xmf4x4ViewPerspectiveProject,
		XMMatrixMultiply(xmmView, xmmPersProj)
	);
}

void CCamera::GeneratePerspectiveProjectionMatrix(float fNearPlaneDistance, float fFarPlaneDistance, float fFOVAngle)
{
	float fAspectRatio = (float(m_Viewport.m_nWidth) / float(m_Viewport.m_nHeight));
	XMStoreFloat4x4(
		&m_xmf4x4PerspectiveProject,
		XMMatrixPerspectiveFovLH(
			XMConvertToRadians(fFOVAngle),
			fAspectRatio,
			fNearPlaneDistance,
			fFarPlaneDistance)
	);
}

void CCamera::SetLookAt(const XMFLOAT3& xmf3LookAt, const XMFLOAT3& xmf3Up)
{
	SetLookAt(m_xmf3Position, xmf3LookAt, xmf3Up);
}

void CCamera::SetLookAt(const XMFLOAT3& xmf3Position, const XMFLOAT3& xmf3LookAt, const  XMFLOAT3& xmf3Up)
{
	m_xmf3Position = xmf3Position;
	XMStoreFloat4x4(
		&m_xmf4x4View,
		XMMatrixLookAtLH(
			XMLoadFloat3(&m_xmf3Position),
			XMLoadFloat3(&xmf3LookAt),
			XMLoadFloat3(&xmf3Up)
		));

	// 뷰 변환 행렬에서 다시 기저 벡터 추출해 저장
	XMVECTORF32 xmf32vRight = { m_xmf4x4View._11, m_xmf4x4View._21, m_xmf4x4View._31, 0.0f };
	XMVECTORF32 xmf32vUp = { m_xmf4x4View._12, m_xmf4x4View._22, m_xmf4x4View._32, 0.0f };
	XMVECTORF32 xmf32vLook = { m_xmf4x4View._13, m_xmf4x4View._23, m_xmf4x4View._33, 0.0f };

	XMStoreFloat3(&m_xmf3Right, XMVector3Normalize(xmf32vRight));
	XMStoreFloat3(&m_xmf3Up, XMVector3Normalize(xmf32vUp));
	XMStoreFloat3(&m_xmf3Look, XMVector3Normalize(xmf32vLook));
}

void CCamera::SetFOVAngle(float fFOVAngle)
{
	m_fFOVAngle = fFOVAngle;
	m_fProjectRectDistance = float(1.0f / tan(XMConvertToRadians(fFOVAngle * 0.5f)));
}

void CCamera::SetViewport(int nLeft, int nTop, int nWidth, int nHeight)
{
	m_Viewport.SetViewport(nLeft, nTop, nWidth, nHeight);
	m_fAspectRatio = float(m_Viewport.m_nWidth) / float(m_Viewport.m_nHeight);
}

void CCamera::Move(const XMFLOAT3& xmf3Shift)
{
	XMStoreFloat3(&m_xmf3Position,
		XMVectorAdd(
			XMLoadFloat3(&m_xmf3Position),
			XMLoadFloat3(&xmf3Shift)
		));
}

void CCamera::Move(float x, float y, float z)
{
	Move(XMFLOAT3(x, y, z));
}

void CCamera::Rotate(float fPitch, float fYaw, float fRoll)
{
	// 축마다 pitch yaw roll 적용
	if (fPitch != 0.0f) {
		XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&m_xmf3Right), XMConvertToRadians(fPitch));
		XMStoreFloat3(&m_xmf3Look, XMVector3TransformNormal(XMLoadFloat3(&m_xmf3Look), xmmtxRotate));
		XMStoreFloat3(&m_xmf3Up, XMVector3TransformNormal(XMLoadFloat3(&m_xmf3Up), xmmtxRotate));
	}
	if (fYaw != 0.0f) {
		XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&m_xmf3Up), XMConvertToRadians(fYaw));
		XMStoreFloat3(&m_xmf3Look, XMVector3TransformNormal(XMLoadFloat3(&m_xmf3Look), xmmtxRotate));
		XMStoreFloat3(&m_xmf3Right,	XMVector3TransformNormal(XMLoadFloat3(&m_xmf3Right), xmmtxRotate));
	}
	if (fRoll != 0.0f) {
		XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&m_xmf3Look), XMConvertToRadians(fRoll));
		XMStoreFloat3(&m_xmf3Up, XMVector3TransformNormal(XMLoadFloat3(&m_xmf3Up), xmmtxRotate));
		XMStoreFloat3(&m_xmf3Right,	XMVector3TransformNormal(XMLoadFloat3(&m_xmf3Right), xmmtxRotate));
	}
}


void CCamera::Update(CPlayer* pPlayer, XMFLOAT3& xmf3LookAt, float fTimeElapsed)
{
	// Player를 기준으로 하는 회전 행렬
	XMFLOAT4X4 mtxRotate = Matrix4x4::Identity();
	XMFLOAT3 pRight = pPlayer->GetRight();
	XMFLOAT3 pUp = pPlayer->GetUp();
	XMFLOAT3 pLook = pPlayer->GetLook();

	mtxRotate._11 = pRight.x; mtxRotate._21 = pUp.x; mtxRotate._31 = pLook.x;
	mtxRotate._12 = pRight.y; mtxRotate._22 = pUp.y; mtxRotate._32 = pLook.y;
	mtxRotate._13 = pRight.z; mtxRotate._23 = pUp.z; mtxRotate._33 = pLook.z;
	

	// 현재 위치에서, 플레이어 위치와 방향 및 오프셋(카메라 거리)에 기반하여 카메라 위치 조정
	XMFLOAT3 xmf3Offset = Vector3::TransformCoord(pPlayer->GetCameraOffset(), mtxRotate);
	XMFLOAT3 xmf3Position = Vector3::Add(pPlayer->GetPosition(), xmf3Offset);
	XMFLOAT3 xmf3Direction = Vector3::Subtract(xmf3Position, m_xmf3Position);

	float fLength = Vector3::Length(xmf3Direction);
	xmf3Direction = Vector3::Normalize(xmf3Direction);

	float fTimeLagScale = fTimeElapsed * 4.0f;
	float fDistance = fLength * fTimeLagScale;
	if (fDistance > fLength) fDistance = fLength;
	if (fLength < 0.01f) fDistance = fLength;
	if (fDistance > 0)
	{
		m_xmf3Position = Vector3::Add(m_xmf3Position, xmf3Direction, fDistance);
		SetLookAt(pPlayer->GetPosition(), pPlayer->GetUp());
	}
}
