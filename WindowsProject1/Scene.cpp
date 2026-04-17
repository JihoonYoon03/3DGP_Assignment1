#include "framework.h"
#include "Scene.h"
#include "GraphicsPipeline.h"
#include "Player.h"

CScene::CScene(CPlayer* pPlayer)
{
	m_pPlayer = pPlayer;
	m_vObjects.push_back(pPlayer);
}

CScene::~CScene()
{
}

void CScene::BuildObjects()
{
	CCubeMesh* pCubeMesh = new CCubeMesh(4.0f, 4.0f, 4.0f);

	CGameObject* newObject = new CGameObject();
	newObject->SetMesh(pCubeMesh);
	newObject->SetColor(RGB(255, 0, 0));
	newObject->SetPosition(-13.5f, 0.0f, +14.0f);
	newObject->SetRotationAxis(XMFLOAT3(1.0f, 1.0f, 0.0f));
	newObject->SetRotationSpeed(90.0f);
	newObject->SetMovingDirection(XMFLOAT3(1.0f, 0.0f, 0.0f));
	newObject->SetMovingSpeed(0.0f);
	m_vObjects.push_back(newObject);

	newObject = new CGameObject();
	newObject->SetMesh(pCubeMesh);
	newObject->SetColor(RGB(0, 0, 255));
	newObject->SetPosition(+13.5f, 0.0f, +14.0f);
	newObject->SetRotationAxis(XMFLOAT3(0.0f, 1.0f, 1.0f));
	newObject->SetRotationSpeed(180.0f);
	newObject->SetMovingDirection(XMFLOAT3(-1.0f, 0.0f, 0.0f));
	newObject->SetMovingSpeed(0.0f);
	m_vObjects.push_back(newObject);

	newObject = new CGameObject();
	newObject->SetMesh(pCubeMesh);
	newObject->SetColor(RGB(0, 255, 0));
	newObject->SetPosition(0.0f, +5.0f, 20.0f);
	newObject->SetRotationAxis(XMFLOAT3(1.0f, 0.0f, 1.0f));
	newObject->SetRotationSpeed(30.15f);
	newObject->SetMovingDirection(XMFLOAT3(1.0f, -1.0f, 0.0f));
	newObject->SetMovingSpeed(0.0f);
	m_vObjects.push_back(newObject);

	newObject = new CGameObject();
	newObject->SetMesh(pCubeMesh);
	newObject->SetColor(RGB(0, 255, 255));
	newObject->SetPosition(0.0f, 0.0f, 40.0f);
	newObject->SetRotationAxis(XMFLOAT3(0.0f, 0.0f, 1.0f));
	newObject->SetRotationSpeed(40.6f);
	newObject->SetMovingDirection(XMFLOAT3(0.0f, 0.0f, 1.0f));
	newObject->SetMovingSpeed(0.0f);
	m_vObjects.push_back(newObject);

	newObject = new CGameObject();
	newObject->SetMesh(pCubeMesh);
	newObject->SetColor(RGB(128, 0, 255));
	newObject->SetPosition(10.0f, 10.0f, 50.0f);
	newObject->SetRotationAxis(XMFLOAT3(0.0f, 1.0f, 1.0f));
	newObject->SetRotationSpeed(50.06f);
	newObject->SetMovingDirection(XMFLOAT3(0.0f, 1.0f, 1.0f));
	newObject->SetMovingSpeed(0.0f);
	m_vObjects.push_back(newObject);


}

void CScene::ReleaseObjects()
{

}

void CScene::OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
}

void CScene::OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
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

void CScene::Animate(float fElapsedTime)
{
	for (auto& object : m_vObjects)
		object->Animate(fElapsedTime);
}

void CScene::Render(HDC hDCFrameBuffer, CCamera* pCamera)
{
	CGraphicsPipeline::SetViewport(&pCamera->m_Viewport);
	CGraphicsPipeline::SetViewPerspectiveProjectTransform(&pCamera->m_xmf4x4ViewPerspectiveProject);

	XMFLOAT4X4 viewMatrix = pCamera->GetViewMatrix();

	// 거리 기준 렌더링 순서 정렬
	std::sort(m_vObjects.begin(), m_vObjects.end(),
		[&viewMatrix](const CGameObject* a, const CGameObject* b) {
			float aZ = Vector3::TransformCoord(XMFLOAT3{}, Matrix4x4::Multiply(a->GetWorldMatrix(), viewMatrix)).z;
			float bZ = Vector3::TransformCoord(XMFLOAT3{}, Matrix4x4::Multiply(b->GetWorldMatrix(), viewMatrix)).z;

			return aZ > bZ;
		});

	for (const auto& object : m_vObjects)
		object->Render(hDCFrameBuffer, pCamera);
}
