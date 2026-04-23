#include "framework.h"
#include "Scene.h"
#include "GraphicsPipeline.h"
#include "Player.h"
#include "GameVar.h"

CScene::CScene(CPlayer* pPlayer)
{
	m_pPlayer = pPlayer;

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

			if (nIntersected && fHitDistance < fNearestHitDistance)	{
				fNearestHitDistance = fHitDistance;
				pNearestObject = object;
			}
		}
	}

	return pNearestObject;
}

void CScene::OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	switch (nMessageID)	{
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
	for (auto& vObj : m_mapObjects) {
		for (auto& obj : vObj.second) {
			if (obj->FrustumCullingTest(pCamera))
				cullPassed.push_back(obj);
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
CSceneTitle::CSceneTitle(CCamera* pCamera)
	: CScene()
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
	CMesh* pAirplaneMesh = new CMesh(L"../Resources/Obj/LowPolyF22.obj", 0.05f);
	CCubeMesh* cubeMesh = new CCubeMesh(1.5f, 1.5f, 1.5f);

	std::vector<CGameObject*> objects;

	CUIObject* newObject = new CUIObject();
	newObject->SetMesh(pAirplaneMesh);
	newObject->SetColor(RGB(60, 60, 70));
	newObject->SetPosition(0.0f, 0.0f, 0.0f);
	newObject->SetWorldMatrix(Matrix4x4::RotationYawPitchRoll(0.f, -90.f, 90.f));
	newObject->SetRotationAxis(XMFLOAT3(1.0f, 0.0f, 0.0f));
	newObject->SetRotationSpeed(90.0f);
	newObject->SetMovingDirection(XMFLOAT3(1.0f, 0.0f, 0.0f));
	newObject->SetMovingSpeed(0.0f);
	objects.push_back(newObject);

	newObject = new CUIObject();
	newObject->SetMesh(cubeMesh);
	newObject->SetColor(RGB(255, 0, 0));
	newObject->SetPosition(0.0f, -7.0f, 0.0f);
	newObject->SetRotationAxis(XMFLOAT3(0.f, 1.f, 0.f));
	newObject->SetRotationSpeed(-20.0f);
	newObject->LookAt(m_pCamera->GetPosition(), XMFLOAT3(0.f, 1.f, 0.f ));
	objects.push_back(newObject);

	m_mapObjects.emplace(eObjType::UI, std::move(objects));
	objects.clear();
}

void CSceneTitle::OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	switch (nMessageID)
	{
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
		::SetCapture(hWnd);
		::GetCursorPos(&oldCursorPos);
		if (nMessageID == WM_LBUTTONDOWN && m_pCamera) {
			CGameObject* picked = PickObjectPointedByCursor(LOWORD(lParam), HIWORD(lParam), m_pCamera);
			if (picked)	picked->EventPicking();
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

void CSceneTitle::OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	switch (nMessageID) {
	default:
		break;
	}
}

// ===============================================================
CSceneStage::CSceneStage(CPlayer* player)
	: CScene(player)
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