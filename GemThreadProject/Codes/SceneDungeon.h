#ifndef _SCENEDUNGEON_H_
#define _SCENEDUNGEON_H_

#include "Scene.h"
#include "glm\vec3.hpp"

namespace Engine
{
	class CInputDevice;
	class CLayer;
	class CSkyBox;
}

class DefaultCamera;
class BGObject;
class MazeManager;

// A game scene class that inherits from the engine's CScene class
class SceneDungeon : public Engine::CScene
{
private:
	Engine::CInputDevice*		m_pInputDevice;
	Engine::CLayer*				m_pCharacterLayer;
	Engine::CSkyBox*			m_pSkyBox;
	DefaultCamera*				m_pDefaultCamera;
	MazeManager*				m_pMazeManager;

	glm::vec3					m_vCameraSavedPos;
	glm::vec3					m_vCameraSavedRot;
	glm::vec3					m_vCameraSavedTarget;

	_bool						m_bFollowingMode;

private:
	explicit SceneDungeon();
	virtual ~SceneDungeon();
	virtual void Destroy();
public:
	virtual void Update(const _float& dt);
	virtual void Render();

public:
	glm::vec3 GetCameraPos();
private:
	void KeyCheck();
	void SetDefaultCameraSavedPosition(glm::vec3 vPos, glm::vec3 vRot, glm::vec3 target);
	void ResetDefaultCameraPos();

private:
	RESULT Ready(std::string dataPath);
	RESULT ReadyLayerAndGameObject();
public:
	static SceneDungeon* Create(std::string dataPath);

};

#endif //_SCENEDUNGEON_H_