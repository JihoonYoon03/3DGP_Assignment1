#pragma once

#include "Mesh.h"

class CViewport {
public:
	CViewport(int nLeft, int nTop, int nWidth, int nHeight);
	virtual ~CViewport() {}

	int m_nLeft;
	int m_nTop;
	int m_nWidth;
	int m_nHeight;
};

class CCamera
{
public:
	CCamera();
	virtual ~CCamera();

	CPoint3D CameraTransform(CPoint3D& f3World);
	CPoint3D ProjectionTransform(CPoint3D& f3Camera);
	CPoint3D ScreenTransform(CPoint3D& f3Projection);

	void SetPosition(float x, float y, float z);
	void SetRotation(float fPitch, float fYaw, float fRoll);

	// 카메라의 뷰포트와 시야각을 설정한다.
	void SetViewport(int xStart, int yStart, int nWidth, int nHeight);
	void SetFOVAngle(float fFOVAngle);

	//카메라를 이동하고 회전한다.
	void Move(float x, float y, float z);
	void Rotate(float fPitch, float fYaw, float fRoll);
private:
	//위치(월드 좌표계)
	float m_fxPosition = 0.0f;
	float m_fyPosition = 0.0f;
	float m_fzPosition = 0.0f;

	//회전(카메라 좌표계)
	float m_fxRotation = 0.0f;
	float m_fyRotation = 0.0f;
	float m_fzRotation = 0.0f;

	//카메라의 시야각, 투영 사각형까지의 거리
	float m_fFOVAngle = 90.0f;
	float m_fProjectRectDistance = 1.0f;

	//뷰포트
	CViewport* m_pViewport = nullptr;

	//종횡비
	float m_fAspectRatio = FRAMEBUFFER_WIDTH / FRAMEBUFFER_HEIGHT;
};

