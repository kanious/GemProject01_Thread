#ifndef _MAPTILE_H_
#define _MAPTILE_H_

#include "GameObject.h"
#include "EngineStruct.h"

namespace Engine
{
	class CMesh;
}

// Map Tile class
class MapTile : public Engine::CGameObject
{
private:
	Engine::CMesh*			m_pMesh;

private:
	explicit MapTile();
	virtual ~MapTile();

public:
	virtual void Update(const _float& dt);
	virtual void Render();
private:
	virtual void Destroy();
	RESULT Ready(std::string meshID, glm::vec3 vPos, glm::vec3 vRot, glm::vec3 vScale);
public:
	static MapTile* Create(std::string meshID, glm::vec3 vPos, glm::vec3 vRot, glm::vec3 vScale);
};

#endif //_MAPTILE_H_