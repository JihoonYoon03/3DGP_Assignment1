#pragma once

#include "GameObject.h"
#include "Camera.h"

class CScene
{
public:
	CScene() {}
	CScene(CPlayer* pPlayer);
	virtual ~CScene();

private:
	// 게임 객체들의 개수와 게임 객체들의 리스트(배열)이다.
	std::unordered_map<eObjType, std::vector<CGameObject*>> m_mapObjects;

	CPlayer* m_pPlayer = NULL;
public:
	// 게임 객체들을 생성하고 소멸한다. 
	virtual void BuildObjects();
	virtual void ReleaseObjects();

	void FireBullet(CGameObject* pLockedObject);

	void CheckCollision(const eObjType, const eObjType);
	
	// 게임 객체들을 애니메이션한다. 
	virtual void Animate(float fElapsedTime);
	
	// 게임 객체들을 렌더링한다.
	virtual void Render(HDC hDCFrameBuffer, CCamera* pCamera);

	virtual void OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	virtual void OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
};
