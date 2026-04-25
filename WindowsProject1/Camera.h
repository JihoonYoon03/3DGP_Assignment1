#pragma once

//#include "Mesh.h"

class CViewport {
public:
	CViewport() {}
	CViewport(int nLeft, int nTop, int nWidth, int nHeight);
	virtual ~CViewport() {}

	int m_nLeft = 0;
	int m_nTop = 0;
	int m_nWidth = 0;
	int m_nHeight = 0;

	void SetViewport(int nLeft, int nTop, int nWidth, int nHeight);
};

class CPlayer;

class CCamera
{
public:
	CCamera();
	virtual ~CCamera();

	void GenerateViewMatrix();
	void GeneratePerspectiveProjectionMatrix(float fNearPlaneDistance, float fFarPlaneDistance, float fFOVAngle);
	void GenerateOrthographicProjectionMatrix(float fNearPlaneDistance, float fFarPlaneDistance, float fWidth, float hHeight);

	// 카메라의 뷰포트와 시야각을 설정한다.
	void SetViewport(int xTopLeft, int yTopLeft, int nWidth, int nHeight);
	void SetFOVAngle(float fFOVAngle);

	void SetLookAt(const XMFLOAT3& xmf3LookAt, const XMFLOAT3& xmf3Up);
	void SetLookAt(const XMFLOAT3& xmf3Position, const XMFLOAT3& xmf3LookAt, const XMFLOAT3& xmf3Up);

	const XMFLOAT3& GetPosition() const { return m_xmf3Position; }
	const XMFLOAT3& GetLook() const { return m_xmf3Look; }
	const XMFLOAT4X4& GetViewMatrix() const { return m_xmf4x4View; }
	const XMFLOAT4X4& GetPerspectiveProjectMatrix() const { return m_xmf4x4PerspectiveProject; }

	// 카메라를 이동하고 회전한다.
	void Move(const XMFLOAT3& xmf3Shift);
	void Move(float x, float y, float z);
	void Rotate(float fPitch = 0.f, float fYaw = 0.f, float fRoll = 0.f);
	void Rotate(XMVECTOR& quaternion);
	void Update(CPlayer* pPlayer, XMFLOAT3& xmf3LookAt, float fTimeElapsed = 0.016f);

	bool IsInFrustum(BoundingOrientedBox& xmBoundingBox);

	XMFLOAT4X4	m_xmf4x4View = Matrix4x4::Identity();
	XMFLOAT4X4	m_xmf4x4PerspectiveProject = Matrix4x4::Identity();
	XMFLOAT4X4	m_xmf4x4ViewPerspectiveProject = Matrix4x4::Identity();

	XMFLOAT4X4	m_xmf4x4OrthographicProject = Matrix4x4::Identity();
	XMFLOAT4X4	m_xmf4x4ViewOrthographicProject = Matrix4x4::Identity();

	CViewport	m_Viewport;

private:
	// 위치, 기저벡터
	XMFLOAT3	m_xmf3Position = XMFLOAT3(0.f, 0.f, 0.f);
	XMFLOAT3	m_xmf3Right = XMFLOAT3(1.f, 0.f, 0.f);
	XMFLOAT3	m_xmf3Up	= XMFLOAT3(0.f, 1.f, 0.f);
	XMFLOAT3	m_xmf3Look	= XMFLOAT3(0.f, 0.f, 1.f);

	// 카메라의 시야각, 투영 사각형까지의 거리
	float m_fFOVAngle = 90.0f;
	float m_fProjectRectDistance = 1.0f;

	// 시야 절두체, 뷰 역행렬
	BoundingFrustum				m_xmFrustumView = BoundingFrustum();
	BoundingFrustum				m_xmFrustumWorld = BoundingFrustum();
	XMFLOAT4X4					m_xmf4x4InverseView = Matrix4x4::Identity();

	// 종횡비
	float m_fAspectRatio = FRAMEBUFFER_WIDTH / FRAMEBUFFER_HEIGHT;
};

