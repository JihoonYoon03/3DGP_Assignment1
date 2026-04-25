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
	m_xmf4x4View._41 = -Vector3::DotProduct(m_xmf3Position, m_xmf3Right);
	m_xmf4x4View._42 = -Vector3::DotProduct(m_xmf3Position, m_xmf3Up);
	m_xmf4x4View._43 = -Vector3::DotProduct(m_xmf3Position, m_xmf3Look);

	m_xmf4x4ViewPerspectiveProject = Matrix4x4::Multiply(m_xmf4x4View, m_xmf4x4PerspectiveProject);
	m_xmf4x4OrthographicProject = Matrix4x4::Multiply(m_xmf4x4View, m_xmf4x4OrthographicProject);

	m_xmf4x4InverseView._11 = m_xmf3Right.x; m_xmf4x4InverseView._12 = m_xmf3Right.y; m_xmf4x4InverseView._13 = m_xmf3Right.z;
	m_xmf4x4InverseView._21 = m_xmf3Up.x; m_xmf4x4InverseView._22 = m_xmf3Up.y; m_xmf4x4InverseView._23 = m_xmf3Up.z;
	m_xmf4x4InverseView._31 = m_xmf3Look.x; m_xmf4x4InverseView._32 = m_xmf3Look.y; m_xmf4x4InverseView._33 = m_xmf3Look.z;
	m_xmf4x4InverseView._41 = m_xmf3Position.x; m_xmf4x4InverseView._42 = m_xmf3Position.y; m_xmf4x4InverseView._43 = m_xmf3Position.z;

	m_xmFrustumView.Transform(m_xmFrustumWorld, XMLoadFloat4x4(&m_xmf4x4InverseView));
}

void CCamera::GeneratePerspectiveProjectionMatrix(float fNearPlaneDistance, float fFarPlaneDistance, float fFOVAngle)
{
	float fAspectRatio = (float(m_Viewport.m_nWidth) / float(m_Viewport.m_nHeight));
	XMMATRIX xmmtxProjection =
		XMMatrixPerspectiveFovLH(
			XMConvertToRadians(fFOVAngle),
			fAspectRatio,
			fNearPlaneDistance,
			fFarPlaneDistance
		);
	XMStoreFloat4x4(&m_xmf4x4PerspectiveProject, xmmtxProjection);

	BoundingFrustum::CreateFromMatrix(m_xmFrustumView, xmmtxProjection);
}

void CCamera::GenerateOrthographicProjectionMatrix(float fNearPlaneDistance, float fFarPlaneDistance, float fWidth, float hHeight)
{
	XMMATRIX xmmtxProjection = XMMatrixOrthographicLH(fWidth, hHeight, fNearPlaneDistance, fFarPlaneDistance);
	XMStoreFloat4x4(&m_xmf4x4OrthographicProject, xmmtxProjection);
}

void CCamera::SetLookAt(const XMFLOAT3& xmf3LookAt, const XMFLOAT3& xmf3Up)
{
	SetLookAt(m_xmf3Position, xmf3LookAt, xmf3Up);
}

void CCamera::SetLookAt(const XMFLOAT3& xmf3Position, const XMFLOAT3& xmf3LookAt, const  XMFLOAT3& xmf3Up)
{
	m_xmf3Position = xmf3Position;
	m_xmf4x4View = Matrix4x4::LookAtLH(m_xmf3Position, xmf3LookAt, xmf3Up);
	m_xmf3Right = Vector3::Normalize(XMFLOAT3(m_xmf4x4View._11, m_xmf4x4View._21, m_xmf4x4View._31));
	m_xmf3Up = Vector3::Normalize(XMFLOAT3(m_xmf4x4View._12, m_xmf4x4View._22, m_xmf4x4View._32));
	m_xmf3Look = Vector3::Normalize(XMFLOAT3(m_xmf4x4View._13, m_xmf4x4View._23, m_xmf4x4View._33));
}

void CCamera::SetFOVAngle(float fFOVAngle)
{
	m_fFOVAngle = fFOVAngle;
	m_fProjectRectDistance = float(1.0f / tan(DegreeToRadian(fFOVAngle * 0.5f)));
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

void CCamera::Rotate(XMVECTOR& quaternion)
{
	XMStoreFloat3(&m_xmf3Look, XMVector3Rotate(XMVECTOR{0.f, 0.f, 1.f}, quaternion));
	XMStoreFloat3(&m_xmf3Up, XMVector3Rotate(XMVECTOR{0.f, 1.f, 0.f}, quaternion));
	XMStoreFloat3(&m_xmf3Right, XMVector3Rotate(XMVECTOR{1.f, 0.f, 0.f}, quaternion));
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

bool CCamera::IsInFrustum(BoundingOrientedBox& xmBoundingBox)
{
	return m_xmFrustumWorld.Intersects(xmBoundingBox);
}
