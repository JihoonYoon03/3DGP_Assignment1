#pragma once

#include "GameObject.h"
#include "Camera.h"

// ===============================================================
class CScene {
// ===============================================================
public:
	CScene() {}
	CScene(CPlayer* pPlayer);
	virtual ~CScene();

protected:
	// 게임 객체들의 개수와 게임 객체들의 리스트(배열)이다.
	std::unordered_map<eObjType, std::vector<CGameObject*>> m_mapObjects;

	CPlayer* m_pPlayer = nullptr;
public:
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

	virtual void OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	virtual void OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
};

// ===============================================================
class CSceneTitle : public CScene {
// ===============================================================
public:
	CSceneTitle() : CScene() {}
	CSceneTitle(CCamera* pCamera);
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
	CSceneStage(CPlayer* pPlayer);
	~CSceneStage() override {}

	virtual void BuildObjects() override;

	void FireBullet(CGameObject* pLockedObject);

	virtual void OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam) override;
	virtual void OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam) override;
};