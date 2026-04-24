#pragma once

#include "GameObject.h"
#include "Camera.h"

class CGameTimer;
class CGameFramework;

enum class SceneType { title, stage, exit, TypeMax };

// ===============================================================
class CScene {
// ===============================================================
public:
	CScene() {}
	CScene(CGameFramework* pFramework, CPlayer* pPlayer = nullptr);
	virtual ~CScene();

	// 게임 객체들을 생성하고 소멸한다. 
	virtual void BuildObjects() = 0;
	virtual void ReleaseObjects() {};

	virtual void FireBullet(CGameObject* pLockedObject) {}

	void CheckCollision(const eObjType, const eObjType);
	CGameObject* PickObjectPointedByCursor(int xClient, int yClient, CCamera* pCamera);
	
	// 게임 객체들을 애니메이션한다. 
	virtual void Animate(float fElapsedTime);
	
	// 게임 객체들을 렌더링한다.
	virtual void Render(HDC hDCFrameBuffer, CCamera* pCamera);

	// 입력 처리
	virtual void OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	virtual void OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	virtual void ProcessInput(HWND& hWnd, UCHAR* pKeyBuffer, CGameTimer& timer) {}

	// 장면 전환 처리
	// 프레임워크의 장면 전환 함수 호출 시 동작하는 함수들임.
	virtual void EnterScene();	// 씬 진입
	virtual void ExitScene();	// 씬 전환

protected:
	// 게임 객체들의 개수와 게임 객체들의 리스트(배열)이다.
	std::unordered_map<eObjType, std::vector<CGameObject*>> m_mapObjects;

	CPlayer* m_pPlayer = nullptr;
	CGameFramework* m_pFramework = nullptr;

	float m_fSceneChangeInputDelay = 0.2f;
	float m_fSceneChangeElapsed = 0.f;
};

// ===============================================================
class CSceneTitle : public CScene {
// ===============================================================
public:
	CSceneTitle() : CScene() {}
	CSceneTitle(CGameFramework* pFramework, CCamera* pCamera);
	~CSceneTitle() override {}

	virtual void BuildObjects() override;

	virtual void Animate(float fElapsedTime) override;

	virtual void OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam) override;
	virtual void OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam) override;
private:
	CCamera* m_pCamera = nullptr;
};

// ===============================================================
class CSceneStage : public CScene {
// ===============================================================
public:
	CSceneStage() : CScene() {}
	CSceneStage(CGameFramework* pFramework, CPlayer* pPlayer);
	~CSceneStage() override {}

	virtual void BuildObjects() override;

	void FireBullet(CGameObject* pLockedObject);

	virtual void Animate(float fElapsedTime) override;

	virtual void OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam) override;
	virtual void OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam) override;

	virtual void ProcessInput(HWND& hWnd, UCHAR* pKeyBuffer, CGameTimer& timer) override;
};