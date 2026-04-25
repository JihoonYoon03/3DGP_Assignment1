#include "framework.h"
#include "Player.h"

CPlayer::~CPlayer()
{
	if (m_pCamera)
		delete m_pCamera;
}

void CPlayer::SetPosition(float x, float y, float z)
{
	// 플레이어 객체의 위치와 카메라의 위치를 설정한다. 
	m_xmf3Position = XMFLOAT3(x, y, z);
	CGameObject::SetPosition(x, y, z);
}

void CPlayer::LookAt(XMFLOAT3& xmf3LookAt, XMFLOAT3& xmf3Up)
{
	XMFLOAT4X4 xmf4x4View;
	XMStoreFloat4x4(&xmf4x4View, XMMatrixLookAtLH(XMLoadFloat3(&m_xmf3Position), XMLoadFloat3(&xmf3LookAt), XMLoadFloat3(&xmf3Up)));

	XMVECTORF32 xmf32vRight = { xmf4x4View._11, xmf4x4View._21, xmf4x4View._31, 0.0f };
	XMVECTORF32 xmf32vUp = { xmf4x4View._12, xmf4x4View._22, xmf4x4View._32, 0.0f };
	XMVECTORF32 xmf32vLook = { xmf4x4View._13, xmf4x4View._23, xmf4x4View._33, 0.0f };

	XMStoreFloat3(&m_xmf3Right, XMVector3Normalize(xmf32vRight));
	XMStoreFloat3(&m_xmf3Up, XMVector3Normalize(xmf32vUp));
	XMStoreFloat3(&m_xmf3Look, XMVector3Normalize(xmf32vLook));
}

void CPlayer::Move(DWORD dwDirection, float elapsedTime)
{
	if (dwDirection)
	{
		XMFLOAT3 xmf3Shift = XMFLOAT3(0, 0, 0);
		if (dwDirection & DIR_FORWARD) XMStoreFloat3(&xmf3Shift, XMVectorAdd(XMLoadFloat3(&xmf3Shift), XMVectorScale(XMLoadFloat3(&m_xmf3Look), m_maxSpeed * elapsedTime)));
		//if (dwDirection & DIR_BACKWARD) XMStoreFloat3(&xmf3Shift, XMVectorAdd(XMLoadFloat3(&xmf3Shift), XMVectorScale(XMLoadFloat3(&m_xmf3Look), -m_maxSpeed * elapsedTime)));
		//if (dwDirection & DIR_RIGHT) XMStoreFloat3(&xmf3Shift, XMVectorAdd(XMLoadFloat3(&xmf3Shift), XMVectorScale(XMLoadFloat3(&m_xmf3Right), m_maxSpeed * elapsedTime)));
		//if (dwDirection & DIR_LEFT) XMStoreFloat3(&xmf3Shift, XMVectorAdd(XMLoadFloat3(&xmf3Shift), XMVectorScale(XMLoadFloat3(&m_xmf3Right), -m_maxSpeed * elapsedTime)));
		//if (dwDirection & DIR_UP) XMStoreFloat3(&xmf3Shift, XMVectorAdd(XMLoadFloat3(&xmf3Shift), XMVectorScale(XMLoadFloat3(&m_xmf3Up), m_maxSpeed * elapsedTime)));
		//if (dwDirection & DIR_DOWN) XMStoreFloat3(&xmf3Shift, XMVectorAdd(XMLoadFloat3(&xmf3Shift), XMVectorScale(XMLoadFloat3(&m_xmf3Up), -m_maxSpeed * elapsedTime)));

		Move(xmf3Shift, true);
	}
}

void CPlayer::Move(const XMFLOAT3& xmf3Shift, bool bUpdateVelocity)
{
	if (bUpdateVelocity)
	{
		XMStoreFloat3(&m_xmf3Velocity, XMVectorAdd(XMLoadFloat3(&m_xmf3Velocity), XMLoadFloat3(&xmf3Shift)));
	}
	else
	{
		XMStoreFloat3(&m_xmf3Position, XMVectorAdd(XMLoadFloat3(&m_xmf3Position), XMLoadFloat3(&xmf3Shift)));
		m_pCamera->Move(xmf3Shift);
	}
}

void CPlayer::Move(float x, float y, float z)
{
	Move(XMFLOAT3(x, y, z), false);
}

void CPlayer::Rotate(float fPitch, float fYaw, float fRoll)
{
	// 플레이어 객체와 카메라를 회전한다.
	m_pCamera->Rotate(fPitch, fYaw, fRoll);
	if (fPitch != 0.0f)
	{
		XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&m_xmf3Right), XMConvertToRadians(fPitch));
		XMStoreFloat3(&m_xmf3Look, XMVector3TransformNormal(XMLoadFloat3(&m_xmf3Look), xmmtxRotate));
		XMStoreFloat3(&m_xmf3Up, XMVector3TransformNormal(XMLoadFloat3(&m_xmf3Up), xmmtxRotate));
	}
	if (fYaw != 0.0f)
	{
		XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&m_xmf3Up), XMConvertToRadians(fYaw));
		XMStoreFloat3(&m_xmf3Look, XMVector3TransformNormal(XMLoadFloat3(&m_xmf3Look), xmmtxRotate));
		XMStoreFloat3(&m_xmf3Right, XMVector3TransformNormal(XMLoadFloat3(&m_xmf3Right), xmmtxRotate));
	}
	if (fRoll != 0.0f)
	{
		XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&m_xmf3Look), XMConvertToRadians(fRoll));
		XMStoreFloat3(&m_xmf3Up, XMVector3TransformNormal(XMLoadFloat3(&m_xmf3Up), xmmtxRotate));
		XMStoreFloat3(&m_xmf3Right, XMVector3TransformNormal(XMLoadFloat3(&m_xmf3Right), xmmtxRotate));
	}

	// 축 정규화
	XMVECTOR xmvLook = XMVector3Normalize(XMLoadFloat3(&m_xmf3Look));
	XMVECTOR xmvUp = XMVector3Normalize(XMLoadFloat3(&m_xmf3Up));
	XMVECTOR xmvRight = XMVector3Normalize(XMVector3Cross(xmvUp, xmvLook));
	xmvUp = XMVector3Normalize(XMVector3Cross(xmvLook, xmvRight));

	XMStoreFloat3(&m_xmf3Right, xmvRight);
	XMStoreFloat3(&m_xmf3Up, xmvUp);
	XMStoreFloat3(&m_xmf3Look, xmvLook);
}

void CPlayer::SetCameraOffset(const XMFLOAT3& xmf3CameraOffset)
{
	m_xmf3CameraOffset = xmf3CameraOffset;
	XMFLOAT3 xmf3CameraPosition;
	XMStoreFloat3(&xmf3CameraPosition, XMVectorAdd(XMLoadFloat3(&m_xmf3Position), XMLoadFloat3(&m_xmf3CameraOffset)));
	m_pCamera->SetLookAt(xmf3CameraPosition, m_xmf3Position, m_xmf3Up);

	m_pCamera->GenerateViewMatrix();
}

void CPlayer::Update(float fTimeElapsed)
{
	Move(m_xmf3Velocity, false);

	m_pCamera->Update(this, m_xmf3Position, fTimeElapsed);
	m_pCamera->GenerateViewMatrix();

	// 마찰 계수에 따른 감속 구현
	XMVECTOR xmvVelocity = XMLoadFloat3(&m_xmf3Velocity);
	XMVECTOR xmvDeceleration = XMVector3Normalize(XMVectorScale(xmvVelocity, -1.0f));
	float fLength = XMVectorGetX(XMVector3Length(xmvVelocity));
	float fDeceleration = m_fFriction * fTimeElapsed;
	if (fDeceleration > fLength) fDeceleration = fLength;
	XMStoreFloat3(&m_xmf3Velocity, XMVectorAdd(xmvVelocity, XMVectorScale(xmvDeceleration, fDeceleration)));
}

void CPlayer::OnUpdateTransform()
{
	m_xmf4x4World._11 = m_xmf3Right.x; m_xmf4x4World._12 = m_xmf3Right.y; m_xmf4x4World._13 = m_xmf3Right.z;
	m_xmf4x4World._21 = m_xmf3Up.x; m_xmf4x4World._22 = m_xmf3Up.y; m_xmf4x4World._23 = m_xmf3Up.z;
	m_xmf4x4World._31 = m_xmf3Look.x; m_xmf4x4World._32 = m_xmf3Look.y; m_xmf4x4World._33 = m_xmf3Look.z;
	m_xmf4x4World._41 = m_xmf3Position.x; m_xmf4x4World._42 = m_xmf3Position.y; m_xmf4x4World._43 = m_xmf3Position.z;
}

void CPlayer::Animate(float fElapsedTime)
{
	OnUpdateTransform();

	CGameObject::Animate(fElapsedTime);
}



//=============================
CAirplanePlayer::CAirplanePlayer()
//=============================
{

}

CAirplanePlayer::~CAirplanePlayer()
{

}


void CAirplanePlayer::OnUpdateTransform()
{
	m_xmf4x4World._41 = m_xmf3Position.x; m_xmf4x4World._42 = m_xmf3Position.y; m_xmf4x4World._43 = m_xmf3Position.z;
}

void CAirplanePlayer::Animate(float fElapsedTime)
{
	CPlayer::Animate(fElapsedTime);

	/*
	for (auto& bullet : m_vBullets)	{
		if (bullet->isActive()) bullet->Animate(fElapsedTime);
	}
	*/
}

void CAirplanePlayer::Render(HDC hDCFrameBuffer, CCamera* pCamera)
{
	CPlayer::Render(hDCFrameBuffer, pCamera);

	/*
	for (auto& bullet : m_vBullets) {
		if (bullet->isActive())
			bullet->Render(hDCFrameBuffer, pCamera);
	}
	*/
}


void CAirplanePlayer::FireBullet(CGameObject* pLockedObject, std::vector<CGameObject*>& vBullets)
{
	/*
		if (pLockedObject)
		{
			LookAt(pLockedObject->GetPosition(), XMFLOAT3(0.0f, 1.0f, 0.0f));
			OnUpdateTransform();
		}
	*/

	CBulletObject* pBulletObject = nullptr;
	for (const auto& bullet: vBullets)
	{
		if (not bullet->isActive())
		{
			pBulletObject = static_cast<CBulletObject*>(bullet);
			break;
		}
	}

	if (pBulletObject)
	{
		XMFLOAT3 xmf3Position = GetPosition();
		XMFLOAT3 xmf3Direction = GetLook();
		XMFLOAT3 xmf3FirePosition = Vector3::Add(xmf3Position, Vector3::ScalarProduct(xmf3Direction, 6.0f, false));

		pBulletObject->SetWorldMatrix(Matrix4x4::Multiply(XMMatrixRotationRollPitchYaw(0.f, 0.f, 0.f), m_xmf4x4World));

		pBulletObject->SetFirePosition(xmf3FirePosition);
		pBulletObject->SetMovingDirection(xmf3Direction);
		pBulletObject->SetColor(RGB(255, 0, 0));
		pBulletObject->SetActive(true);

		if (pLockedObject)
		{
			pBulletObject->SetLockedObject(pLockedObject);
			pBulletObject->SetColor(RGB(0, 0, 255));
		}
	}
}

void CAirplanePlayer::Rotate(float MouseDeltaX, float MouseDeltaY)
{
	if (MouseDeltaX > m_fRollSpeed) MouseDeltaX = m_fRollSpeed;
	if (MouseDeltaY > m_fPitchSpeed) MouseDeltaY = m_fPitchSpeed;

	// 회전이 누적될 항등 쿼터니언
	static XMVECTOR curQuat = XMQuaternionIdentity();
	
	// 기본 축 생성
	XMVECTOR baseForward	{ 0.f, 0.f, 1.f, 0.f };
	XMVECTOR baseRight		{ 1.f, 0.f, 0.f, 0.f };
	XMVECTOR baseUp			{ 0.f, 1.f, 0.f, 0.f };

	// 기본 축에서 누적된 회전을 적용해 현재 오브젝트 방향 구하기
	XMVECTOR curForward = XMVector3Rotate(baseForward, curQuat);
	XMVECTOR curRight = XMVector3Rotate(baseRight, curQuat);

	// 오브젝트 기준 좌표축을 회전축으로 하여 마우스 델타만큼 회전하는 쿼터니언 구하기
	XMVECTOR qPitch = XMQuaternionRotationAxis(curRight, MouseDeltaY);
	XMVECTOR qRoll = XMQuaternionRotationAxis(curForward, MouseDeltaX);

	// 최종 쿼터니언 구하기
	curQuat = XMQuaternionMultiply(curQuat, qPitch);
	curQuat = XMQuaternionMultiply(curQuat, qRoll);

	XMStoreFloat3(&m_xmf3Look, XMVector3Rotate(baseForward, curQuat));
	XMStoreFloat3(&m_xmf3Up, XMVector3Rotate(baseUp, curQuat));
	XMStoreFloat3(&m_xmf3Right, XMVector3Rotate(baseRight, curQuat));
	
	CGameObject::Rotate(curQuat);
	m_pCamera->Rotate(curQuat);
}



// =====================================================================================
// =====================================================================================

CEnemyAirplane::CEnemyAirplane()
{
}

CEnemyAirplane::CEnemyAirplane(CPlayer* pPlayer)
{
	m_pPlayer = pPlayer;
	m_fCurSpeed = m_fMaxSpeed;
}

void CEnemyAirplane::SetPosition(float x, float y, float z)
{
	// 플레이어 객체의 위치와 카메라의 위치를 설정한다. 
	m_xmf3Position = XMFLOAT3(x, y, z);
	CGameObject::SetPosition(x, y, z);
}

void CEnemyAirplane::SmoothTurn(const XMFLOAT3& xmf3LookAt, const XMFLOAT3& xmf3Up)
{
	// 영벡터 방지용 코드
	if (Vector3::Equal(GetPosition(), xmf3LookAt)) return;

	XMFLOAT4X4 xmf4x4View;
	xmf4x4View = Matrix4x4::LookAtLH(GetPosition(), xmf3LookAt, xmf3Up);

	XMVECTORF32 xmf32vRight = { xmf4x4View._11, xmf4x4View._21, xmf4x4View._31, 0.0f };
	XMVECTORF32 xmf32vUp = { xmf4x4View._12, xmf4x4View._22, xmf4x4View._32, 0.0f };
	XMVECTORF32 xmf32vLook = { xmf4x4View._13, xmf4x4View._23, xmf4x4View._33, 0.0f };

	XMStoreFloat3(&m_xmf3Look, XMVector3Normalize(XMVectorLerp(XMVector3Normalize(XMLoadFloat3(&m_xmf3Look)), XMVector3Normalize(xmf32vLook), m_fTurnLerp)));
	m_xmf3Right = Vector3::CrossProduct(XMFLOAT3(0.f, 1.f, 0.f), m_xmf3Look, true);
	m_xmf3Up	= Vector3::CrossProduct(m_xmf3Look, m_xmf3Right, true);
}

void CEnemyAirplane::Move(float elapsedTime)
{
	// deltatime 반영해서 속도만큼 이동
	XMStoreFloat3(&m_xmf3Position, XMVectorAdd(XMLoadFloat3(&m_xmf3Position), XMLoadFloat3(&m_xmf3Look) * m_fCurSpeed * elapsedTime));
}

void CEnemyAirplane::Update(float fTimeElapsed)
{
	float distance = Vector3::Length(Vector3::Subtract(m_pPlayer->GetPosition(), GetPosition()));
	
	if (distance > m_fRange) {
		// 타깃을 향해 회전
		SmoothTurn(m_pPlayer->GetPosition(), XMFLOAT3(0.0f, 1.0f, 0.0f));
	}
	else {
		XMFLOAT3 avoid = m_xmOOBB.Extents;
		avoid.x *= 4;
		avoid.z = 0;
		avoid.y *= 4;
		SmoothTurn(Vector3::Add(m_pPlayer->GetPosition(), avoid), XMFLOAT3(0.0f, 1.0f, 0.0f));
	}

	// 타깃과의 거리가 먼 상태면, 최대 속도로 이동
	OutputDebugStringW(
		(std::to_wstring(Vector3::Length(Vector3::Subtract(m_pPlayer->GetPosition(), GetPosition()))) + L"\n").c_str());
	if (distance > m_fRange) {
		m_fCurSpeed = m_fMaxSpeed;
	}
	else {
		float fDeceleration = m_fFriction * fTimeElapsed;
		if (fDeceleration > m_fCurSpeed) fDeceleration = m_fCurSpeed;
		if (m_fCurSpeed > m_fMinSpeed) {
			m_fCurSpeed -= fDeceleration;
		}
		else {
			m_fCurSpeed = m_fMinSpeed;
		}
	}
	Move(fTimeElapsed);
}

void CEnemyAirplane::OnUpdateTransform()
{
	m_xmf4x4World._11 = m_xmf3Right.x; m_xmf4x4World._12 = m_xmf3Right.y; m_xmf4x4World._13 = m_xmf3Right.z;
	m_xmf4x4World._21 = m_xmf3Up.x; m_xmf4x4World._22 = m_xmf3Up.y; m_xmf4x4World._23 = m_xmf3Up.z;
	m_xmf4x4World._31 = m_xmf3Look.x; m_xmf4x4World._32 = m_xmf3Look.y; m_xmf4x4World._33 = m_xmf3Look.z;
	m_xmf4x4World._41 = m_xmf3Position.x; m_xmf4x4World._42 = m_xmf3Position.y; m_xmf4x4World._43 = m_xmf3Position.z;
}

void CEnemyAirplane::Animate(float fElapsedTime)
{
	Update(fElapsedTime);
	OnUpdateTransform();

	CExplosiveObject::Animate(fElapsedTime);
}
