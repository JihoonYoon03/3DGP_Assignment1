#include "framework.h"
#include "GameObject.h"
#include "GraphicsPipeline.h"

CGameObject::~CGameObject()
{
	if (m_pMesh) m_pMesh->Release();
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

void CGameObject::SetMovingDirection(const XMFLOAT3& xmf3MovingDirection)
{
	XMStoreFloat3(
		&m_xmf3MovingDirection,
		XMVector3Normalize(XMLoadFloat3(&xmf3MovingDirection)
		));
}

void CGameObject::SetRotationAxis(const XMFLOAT3& xmf3RotationAxis)
{
	XMStoreFloat3(
		&m_xmf3RotationAxis,
		XMVector3Normalize(XMLoadFloat3(&xmf3RotationAxis)
		));
}

void CGameObject::Move(XMFLOAT3& xmf3Direction, float fSpeed)
{
	SetPosition(
		m_xmf4x4World._41 + xmf3Direction.x * fSpeed,
		m_xmf4x4World._42 + xmf3Direction.y * fSpeed,
		m_xmf4x4World._43 + xmf3Direction.z * fSpeed
	);
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
}

void CGameObject::Render(HDC hDCFrameBuffer, CCamera* pCamera)
{
	if (m_pMesh) {
		CGraphicsPipeline::SetWorldTransform(&m_xmf4x4World);

		HPEN hPen = ::CreatePen(PS_SOLID, 0, m_dwColor);
		HPEN hOldPen = (HPEN)::SelectObject(hDCFrameBuffer, hPen);
		HBRUSH hBrush = ::CreateSolidBrush(m_dwColor);
		HBRUSH hOldBrush = (HBRUSH)::SelectObject(hDCFrameBuffer, hBrush);

		m_pMesh->Render(hDCFrameBuffer);

		::SelectObject(hDCFrameBuffer, hOldPen);
		::SelectObject(hDCFrameBuffer, hOldBrush);
		::DeleteObject(hPen);
		::DeleteObject(hBrush);
	}
}