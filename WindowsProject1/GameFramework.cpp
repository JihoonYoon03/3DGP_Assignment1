#include "framework.h"
#include "GameFramework.h"
#include "GameVar.h"

void CGameFramework::OnCreate(HINSTANCE hInstance, HWND hMainWnd)
{
	m_hInstance = hInstance;
	m_hWnd = hMainWnd;
	
	// 렌더링 화면을 생성한다.
	BuildFrameBuffer();

	// 텍스트 출력용 버퍼 생성
	BuildTextBuffer();
	
	// 플레이어와 게임 세계(씬)을 생성한다.
	BuildObjects();

	m_pszFrameRate = L"LabProject (";
}

void CGameFramework::OnDestroy()
{
	ReleaseObjects();
	if (m_hBitmapFrameBuffer) ::DeleteObject(m_hBitmapFrameBuffer);
	if (m_hDCFrameBuffer) ::DeleteDC(m_hDCFrameBuffer);
	if (m_hBitmapTextBuffer) ::DeleteObject(m_hBitmapTextBuffer);
	if (m_hDCTextBuffer) ::DeleteDC(m_hDCTextBuffer);
}

void CGameFramework::BuildFrameBuffer()
{
	::GetClientRect(m_hWnd, &m_rcClient);

	HDC hDC = ::GetDC(m_hWnd);

	m_hDCFrameBuffer = ::CreateCompatibleDC(hDC);
	m_hBitmapFrameBuffer =
		::CreateCompatibleBitmap(hDC,
			m_rcClient.right - m_rcClient.left,
			m_rcClient.bottom - m_rcClient.top);

	::SelectObject(m_hDCFrameBuffer, m_hBitmapFrameBuffer);

	::ReleaseDC(m_hWnd, hDC);
	::SetBkMode(m_hDCFrameBuffer, TRANSPARENT);
}

void CGameFramework::ClearFrameBuffer(DWORD dwColor)
{
	HPEN hPen = ::CreatePen(PS_SOLID, 0, dwColor);
	HPEN hOldPen = (HPEN)::SelectObject(m_hDCFrameBuffer, hPen);
	HBRUSH hBrush = ::CreateSolidBrush(dwColor);
	HBRUSH hOldBrush = (HBRUSH)::SelectObject(m_hDCFrameBuffer,	hBrush);
	::Rectangle(
		m_hDCFrameBuffer,
		m_rcClient.left, m_rcClient.top,
		m_rcClient.right, m_rcClient.bottom);

	::SelectObject(m_hDCFrameBuffer, hOldBrush);
	::SelectObject(m_hDCFrameBuffer, hOldPen);
	::DeleteObject(hPen);
	::DeleteObject(hBrush);
}

void CGameFramework::PresentFrameBuffer()
{
	HDC hDC = ::GetDC(m_hWnd);
	::BitBlt(hDC,
		m_rcClient.left, m_rcClient.top,
		m_rcClient.right - m_rcClient.left,
		m_rcClient.bottom - m_rcClient.top,
		m_hDCFrameBuffer,
		m_rcClient.left, m_rcClient.top,
		SRCCOPY);

	::ReleaseDC(m_hWnd, hDC);
}

void CGameFramework::BuildTextBuffer()
{
	::GetClientRect(m_hWnd, &m_rcClient);

	HDC hDC = ::GetDC(m_hWnd);

	m_hDCTextBuffer = ::CreateCompatibleDC(hDC);
	m_hBitmapTextBuffer =
		::CreateCompatibleBitmap(hDC,
			m_rcClient.right - m_rcClient.left,
			m_rcClient.bottom - m_rcClient.top);

	::SelectObject(m_hDCTextBuffer, m_hBitmapTextBuffer);

	::ReleaseDC(m_hWnd, hDC);
	::SetBkMode(m_hDCTextBuffer, TRANSPARENT);

	SetTextColor(m_hDCTextBuffer, RGB(255, 255, 255)); // 흰색 글씨
	std::wstring out = L"Fighter Jet";
	RECT rc = m_rcClient;
	::DrawText(m_hDCTextBuffer, out.c_str(), -1, &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

	out = L"Game Start";
	rc.top += FRAMEBUFFER_HEIGHT / 1.6;
	::DrawText(m_hDCTextBuffer, out.c_str(), -1, &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
}

void CGameFramework::BuildObjects()
{
	CCamera* pCamera = new CCamera();
	pCamera->SetViewport(0, 0, FRAMEBUFFER_WIDTH, FRAMEBUFFER_HEIGHT);
	pCamera->GeneratePerspectiveProjectionMatrix(1.01f, 500.0f, 60.0f);
	pCamera->SetFOVAngle(60.0f);

	pCamera->GenerateOrthographicProjectionMatrix(1.01f, 50.0f, FRAMEBUFFER_WIDTH, FRAMEBUFFER_HEIGHT);

	CMesh* pAirplaneMesh = new CMesh(L"../Resources/F22_low.obj", 2.0f);

	m_pPlayer = new CAirplanePlayer();
	m_pPlayer->SetPosition(0.0f, 0.0f, 0.0f);
	m_pPlayer->SetMesh(pAirplaneMesh);
	m_pPlayer->SetColor(RGB(60, 60, 70));
	m_pPlayer->SetCamera(pCamera);
	m_pPlayer->SetCameraOffset(XMFLOAT3(0.0f, 5.0f, -15.0f));

	m_pSceneTitle = std::make_shared<CSceneTitle>(pCamera);
	m_pSceneStage = std::make_shared<CSceneStage>(m_pPlayer);
	m_pSceneTitle->BuildObjects();
	m_pSceneStage->BuildObjects();

	m_pCurrentScene = m_pSceneTitle;
}
void CGameFramework::ReleaseObjects()
{
	if (m_pPlayer) delete m_pPlayer;
}

void CGameFramework::OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	if (m_pCurrentScene)
		m_pCurrentScene->OnProcessingMouseMessage(hWnd, nMessageID, wParam, lParam);

	switch (nMessageID)
	{
	case WM_RBUTTONDOWN:
	case WM_LBUTTONDOWN:
		::SetCapture(hWnd);
		::GetCursorPos(&oldCursorPos);
		if (nMessageID == WM_LBUTTONDOWN) {
			m_pCurrentScene->FireBullet(m_pLockedObject);
			m_pLockedObject = nullptr;
		}
		//if (nMessageID == WM_RBUTTONDOWN) m_pLockedObject = m_pCurrentScene->PickObjectPointedByCursor(LOWORD(lParam), HIWORD(lParam), m_pPlayer->m_pCamera);
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

void CGameFramework::OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	if (m_pCurrentScene)
		m_pCurrentScene->OnProcessingKeyboardMessage(hWnd, nMessageID, wParam, lParam);

	switch (nMessageID)
	{
	case WM_KEYDOWN:
		switch (wParam)
		{
		case VK_ESCAPE:
			::PostQuitMessage(0);
			break;
		case VK_RETURN:
			break;
		case VK_CONTROL:
			break;
		default:
			m_pCurrentScene->OnProcessingKeyboardMessage(hWnd, nMessageID, wParam, lParam);
			break;
		}
		break;
	default:
		break;
	}
}

LRESULT CALLBACK CGameFramework::OnProcessingWindowMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	switch (nMessageID)
	{
	case WM_ACTIVATE:
	{
		if (LOWORD(wParam) == WA_INACTIVE)
			m_GameTimer.Stop();
		else
			m_GameTimer.Start();
		break;
	}
	case WM_SIZE:
		break;
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
	case WM_MOUSEMOVE:
		OnProcessingMouseMessage(hWnd, nMessageID, wParam, lParam);
		break;
	case WM_KEYDOWN:
	case WM_KEYUP:
		OnProcessingKeyboardMessage(hWnd, nMessageID, wParam, lParam);
		break;
	}
	return(0);
}

void CGameFramework::ProcessInput()
{
	static UCHAR pKeyBuffer[256];
	m_pCurrentScene->ProcessInput(m_hWnd, pKeyBuffer, m_GameTimer);
}

void CGameFramework::AnimateObjects()
{
	float fTimeElapsed = m_GameTimer.GetTimeElapsed();
	if (m_pCurrentScene) 
		m_pCurrentScene->Animate(fTimeElapsed);
}

void CGameFramework::FrameAdvance()
{
	m_GameTimer.Tick(60.0f);

	ProcessInput();

	AnimateObjects();

	ClearFrameBuffer(RGB(108, 175, 220));

	CCamera* pCamera = m_pPlayer->GetCamera();
	if (m_pCurrentScene) {
		m_pCurrentScene->Render(m_hDCFrameBuffer, pCamera);

		if (dynamic_cast<CSceneTitle*>(m_pCurrentScene.get())) {
			::TransparentBlt(
				m_hDCFrameBuffer,
				0, 0,
				m_rcClient.right, m_rcClient.bottom,
				m_hDCTextBuffer,
				0, 0,
				m_rcClient.right, m_rcClient.bottom,
				RGB(0, 0, 0));
		}
	}

	PresentFrameBuffer();

	m_pszFrameRate = std::format(L"LabProject ({} FPS)", m_GameTimer.GetFrameRate());
	::SetWindowText(m_hWnd, m_pszFrameRate.c_str());
}