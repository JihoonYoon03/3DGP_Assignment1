#include "framework.h"
#include "Scene.h"
#include "GraphicsPipeline.h"
#include "Player.h"
#include "GameVar.h"
#include "Timer.h"
#include "GameFramework.h"

CScene::CScene(CGameFramework* pFramework, CPlayer* pPlayer)
{
	m_pPlayer = pPlayer;
	m_pFramework = pFramework;

	if (pPlayer) m_mapObjects.emplace(eObjType::Player, std::vector<CGameObject*>{pPlayer});
}

CScene::~CScene()
{
	ReleaseObjects();
}

void CScene::CheckCollision(const eObjType typeA, const eObjType typeB)
{
	for (auto& objectA : m_mapObjects[typeA]) {
		for (auto& objectB : m_mapObjects[typeB]) {
			if (objectA->isActive() && objectB->isActive() && objectA->GetOOBB().Intersects(objectB->GetOOBB())) {
				objectA->EventCollision(objectB, typeB);
				objectB->EventCollision(objectA, typeA);
			}
		}
	}
}

CGameObject* CScene::PickObjectPointedByCursor(int xClient, int yClient, CCamera* pCamera)
{
	XMFLOAT3 xmf3PickPosition;
	xmf3PickPosition.x = (((2.0f * xClient) / (float)pCamera->m_Viewport.m_nWidth) - 1) / pCamera->m_xmf4x4PerspectiveProject._11;
	xmf3PickPosition.y = -(((2.0f * yClient) / (float)pCamera->m_Viewport.m_nHeight) - 1) / pCamera->m_xmf4x4PerspectiveProject._22;
	xmf3PickPosition.z = 1.0f;

	XMVECTOR xmvPickPosition = XMLoadFloat3(&xmf3PickPosition);
	XMMATRIX xmmtxView = XMLoadFloat4x4(&pCamera->m_xmf4x4View);

	bool nIntersected = false;
	float fNearestHitDistance = FLT_MAX;
	CGameObject* pNearestObject = nullptr;

	for (auto& vObject : m_mapObjects) {
		for (auto& object : vObject.second) {

			float fHitDistance = FLT_MAX;
			nIntersected = object->PickObjectByRayIntersection(xmvPickPosition, xmmtxView, fHitDistance);

			if (nIntersected && fHitDistance < fNearestHitDistance) {
				fNearestHitDistance = fHitDistance;
				pNearestObject = object;
			}
		}
	}

	return pNearestObject;
}

void CScene::OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	switch (nMessageID) {
	case WM_RBUTTONDOWN:
		break;
	case WM_LBUTTONDOWN:
		break;
	case WM_LBUTTONUP:
		break;
	case WM_RBUTTONUP:
		break;
	case WM_MOUSEMOVE:
		break;
	default:
		break;
	}
}

void CScene::OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
}

void CScene::EnterScene()
{
}

void CScene::ExitScene()
{
	m_fSceneChangeElapsed = 0.f;
}

void CScene::Animate(float fElapsedTime)
{
	for (auto& vObject : m_mapObjects) {
		for (auto& object : vObject.second) {
			if (object->isActive())
				object->Animate(fElapsedTime);
		}
	}

	CheckCollision(eObjType::Bullet, eObjType::Explosive);
}

void CScene::Render(HDC hDCFrameBuffer, CCamera* pCamera)
{
	CGraphicsPipeline::SetViewport(&pCamera->m_Viewport);
	CGraphicsPipeline::SetViewPerspectiveProjectTransform(&pCamera->m_xmf4x4ViewPerspectiveProject);

	std::vector<CGameObject*> cullPassed;
	size_t objcnt = 0;
	for (auto& vec : m_mapObjects) {
		objcnt += vec.second.size();
	}

	cullPassed.reserve(objcnt);
	for (auto& vObj : m_mapObjects) {
		for (auto& obj : vObj.second) {
			if (obj->FrustumCullingTest(pCamera))
				cullPassed.emplace_back(obj);
		}
	}

	XMFLOAT4X4 viewMatrix = pCamera->GetViewMatrix();

	// 거리 기준 렌더링 순서 정렬
	std::sort(cullPassed.begin(), cullPassed.end(),
		[&viewMatrix](const CGameObject* a, const CGameObject* b) {
			float aZ = Vector3::TransformCoord(XMFLOAT3{}, Matrix4x4::Multiply(a->GetWorldMatrix(), viewMatrix)).z;
			float bZ = Vector3::TransformCoord(XMFLOAT3{}, Matrix4x4::Multiply(b->GetWorldMatrix(), viewMatrix)).z;

			return aZ > bZ;
		});

	for (const auto& object : cullPassed) {
		if (object->isActive())
			object->Render(hDCFrameBuffer, pCamera);
	}
}

// ===============================================================
CSceneTitle::CSceneTitle(CGameFramework* pFramework, CCamera* pCamera)
	: CScene(pFramework)
	// ===============================================================
{
	m_pCamera = pCamera;
}

void CSceneTitle::Animate(float fElapsedTime)
{
	for (auto& vObject : m_mapObjects) {
		for (auto& object : vObject.second) {
			if (object->isActive())
				object->Animate(fElapsedTime);
		}
	}

	CheckCollision(eObjType::Bullet, eObjType::Explosive);
}

void CSceneTitle::BuildObjects()
{
	CMesh* pAirplaneMesh = new CMesh(L"../Resources/F22_low.obj", 2.0f);
	CCubeMesh* cubeMesh = new CCubeMesh(1.5f, 1.5f, 1.5f);

	std::vector<CGameObject*> objects;

	CUIObject* newObject = new CUIObject();
	newObject->SetMesh(pAirplaneMesh);
	newObject->SetColor(RGB(60, 60, 70));
	newObject->SetPosition(0.0f, 1.0f, 0.0f);
	newObject->SetRotationAxis(XMFLOAT3(0.0f, 1.0f, 0.0f));
	//newObject->SetRotationSpeed(90.0f);
	objects.push_back(newObject);

	newObject = new CUIObject();
	newObject->SetMesh(cubeMesh);
	newObject->SetColor(RGB(255, 0, 0));
	newObject->SetPosition(0.0f, -7.0f, 0.0f);
	newObject->SetRotationAxis(XMFLOAT3(0.f, 1.f, 0.f));
	newObject->SetRotationSpeed(-20.0f);
	newObject->LookAt(m_pCamera->GetPosition(), XMFLOAT3(0.f, 1.f, 0.f));
	newObject->setCheckMouseHover(true);
	newObject->SetOnClickCallback([this]() {
		m_pFramework->ChangeSceneTo(SceneType::stage);
		});
	objects.push_back(newObject);

	m_mapObjects.emplace(eObjType::UI, std::move(objects));
	objects.clear();
}

void CSceneTitle::OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	switch (nMessageID)
	{
	case WM_MOUSEMOVE:
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
	{
		::SetCapture(hWnd);
		::GetCursorPos(&oldCursorPos);
		CGameObject* hovered = PickObjectPointedByCursor(LOWORD(lParam), HIWORD(lParam), m_pCamera);
		if (hovered) {
			if (nMessageID == WM_LBUTTONDOWN && m_pCamera) {
				hovered->EventPicking();
			}
			else {
				static_cast<CUIObject*>(hovered)->EventBeginMouseHovering();
			}
		}
		else {
			for (auto& obj : m_mapObjects.at(eObjType::UI)) {
				static_cast<CUIObject*>(obj)->EventEndMouseHovering();
			}
		}
	}
	break;
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
		::ReleaseCapture();
		break;
	default:
		break;
	}
}

void CSceneTitle::OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	switch (nMessageID) {
	default:
		break;
	}
}

// ===============================================================
CSceneStage::CSceneStage(CGameFramework* pFramework, CPlayer* player)
	: CScene(pFramework, player)
	// ===============================================================
{
}

void CSceneStage::BuildObjects()
{
	CExplosiveObject::PrepareExplosion();

	CCubeMesh* pCubeMesh = new CCubeMesh(4.0f, 4.0f, 4.0f);

	std::vector<CGameObject*> objects;
	objects.reserve(6);

	CGameObject* newObject = new CExplosiveObject();
	newObject->SetMesh(pCubeMesh);
	newObject->SetColor(RGB(255, 0, 0));
	newObject->SetPosition(-13.5f, 0.0f, +14.0f);
	newObject->SetRotationAxis(XMFLOAT3(1.0f, 1.0f, 0.0f));
	newObject->SetRotationSpeed(90.0f);
	newObject->SetMovingDirection(XMFLOAT3(1.0f, 0.0f, 0.0f));
	newObject->SetMovingSpeed(0.0f);
	objects.push_back(newObject);

	newObject = new CExplosiveObject();
	newObject->SetMesh(pCubeMesh);
	newObject->SetColor(RGB(0, 0, 255));
	newObject->SetPosition(+13.5f, 0.0f, +14.0f);
	newObject->SetRotationAxis(XMFLOAT3(0.0f, 1.0f, 1.0f));
	newObject->SetRotationSpeed(180.0f);
	newObject->SetMovingDirection(XMFLOAT3(-1.0f, 0.0f, 0.0f));
	newObject->SetMovingSpeed(0.0f);
	objects.push_back(newObject);

	newObject = new CExplosiveObject();
	newObject->SetMesh(pCubeMesh);
	newObject->SetColor(RGB(0, 255, 0));
	newObject->SetPosition(0.0f, +5.0f, 20.0f);
	newObject->SetRotationAxis(XMFLOAT3(1.0f, 0.0f, 1.0f));
	newObject->SetRotationSpeed(30.15f);
	newObject->SetMovingDirection(XMFLOAT3(1.0f, -1.0f, 0.0f));
	newObject->SetMovingSpeed(0.0f);
	objects.push_back(newObject);

	newObject = new CExplosiveObject();
	newObject->SetMesh(pCubeMesh);
	newObject->SetColor(RGB(0, 255, 255));
	newObject->SetPosition(0.0f, 0.0f, 40.0f);
	newObject->SetRotationAxis(XMFLOAT3(0.0f, 0.0f, 1.0f));
	newObject->SetRotationSpeed(40.6f);
	newObject->SetMovingDirection(XMFLOAT3(0.0f, 0.0f, 1.0f));
	newObject->SetMovingSpeed(0.0f);
	objects.push_back(newObject);

	newObject = new CExplosiveObject();
	newObject->SetMesh(pCubeMesh);
	newObject->SetColor(RGB(128, 0, 255));
	newObject->SetPosition(10.0f, 10.0f, 50.0f);
	newObject->SetRotationAxis(XMFLOAT3(0.0f, 1.0f, 1.0f));
	newObject->SetRotationSpeed(50.06f);
	newObject->SetMovingDirection(XMFLOAT3(0.0f, 1.0f, 1.0f));
	newObject->SetMovingSpeed(0.0f);
	objects.push_back(newObject);

	m_mapObjects.emplace(eObjType::Explosive, std::move(objects));
	objects.clear();

	// 총알 장전
	unsigned int ammo = static_cast<CAirplanePlayer*>(m_pPlayer)->getMaxAmmo();
	float range = static_cast<CAirplanePlayer*>(m_pPlayer)->getRange();

	objects.reserve(ammo);

	CCubeMesh* pBulletMesh = new CCubeMesh(1.0f, 1.0f, 4.0f);

	for (int i = 0; i < ammo; i++) {
		CBulletObject* bullet = new CBulletObject{ range };
		bullet->SetMesh(pBulletMesh);
		bullet->SetRotationAxis(XMFLOAT3(0.0f, 0.0f, 1.0f));
		bullet->SetRotationSpeed(360.0f);
		bullet->SetMovingSpeed(120.0f);
		bullet->SetActive(false);
		objects.push_back(bullet);
	}

	m_mapObjects.emplace(eObjType::Bullet, std::move(objects));
	objects.clear();
}

void CSceneStage::FireBullet(CGameObject* pLockedObject)
{
	static_cast<CAirplanePlayer*>(m_pPlayer)->FireBullet(pLockedObject, m_mapObjects.at(eObjType::Bullet));
}

void CSceneStage::OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	switch (nMessageID)
	{
	case WM_RBUTTONDOWN:
	case WM_LBUTTONDOWN:
		::SetCapture(hWnd);
		::GetCursorPos(&oldCursorPos);
		if (nMessageID == WM_LBUTTONDOWN) {
			FireBullet(nullptr);
		}
		break;
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
		::ReleaseCapture();
		break;
	case WM_MOUSEMOVE:
		break;
	default:
		break;
	}
}

void CSceneStage::OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	switch (nMessageID)
	{
	case WM_KEYDOWN:
		switch (wParam)
		{
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
		{
			break;
		}
		case 'A':
			break;
		default:
			break;
		}
		break;
	default:
		break;
	}
}

void CSceneStage::ProcessInput(HWND& hWnd, UCHAR* pKeyBuffer, CGameTimer& timer)
{
	if (GetKeyboardState(pKeyBuffer))
	{
		DWORD dwDirection = 0;
		if (pKeyBuffer['W'] & 0xF0) dwDirection |= DIR_FORWARD;
		if (pKeyBuffer['S'] & 0xF0) dwDirection |= DIR_BACKWARD;
		if (pKeyBuffer['A'] & 0xF0) dwDirection |= DIR_LEFT;
		if (pKeyBuffer['D'] & 0xF0) dwDirection |= DIR_RIGHT;
		if (pKeyBuffer[VK_SPACE] & 0xF0) dwDirection |= DIR_UP;
		if (pKeyBuffer[VK_CONTROL] & 0xF0) dwDirection |= DIR_DOWN;

		if (dwDirection) m_pPlayer->Move(dwDirection, timer.GetTimeElapsed());
	}

	if (GetCapture() == hWnd)
	{
		SetCursor(NULL);
		POINT ptCursorPos;
		GetCursorPos(&ptCursorPos);
		float cxMouseDelta = (float)(ptCursorPos.x - oldCursorPos.x) / 3.0f;
		float cyMouseDelta = (float)(ptCursorPos.y - oldCursorPos.y) / 3.0f;
		SetCursorPos(oldCursorPos.x, oldCursorPos.y);
		if (cxMouseDelta || cyMouseDelta)
		{
			if (pKeyBuffer[VK_RBUTTON] & 0xF0)
				m_pPlayer->Rotate(cyMouseDelta, 0.0f, -cxMouseDelta);
			else
				m_pPlayer->Rotate(cyMouseDelta, cxMouseDelta, 0.0f);
		}
	}

	m_pPlayer->Update(timer.GetTimeElapsed());
}