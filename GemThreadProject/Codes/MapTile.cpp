#include "MapTile.h"
#include "ComponentMaster.h"
#include "Component.h"
#include "GameObject.h"
#include "Transform.h"
#include "Mesh.h"
#include "Renderer.h"

USING(Engine)
USING(glm)
USING(std)

MapTile::MapTile()
	: m_pMesh(nullptr)
{
}

MapTile::~MapTile()
{
}

void MapTile::Update(const _float& dt)
{
	CGameObject::Update(dt);

	//if (nullptr != m_pRenderer)
	//	m_pRenderer->AddRenderObj(this);
}

void MapTile::Render()
{
	CGameObject::Render();
}

void MapTile::Destroy()
{
	CGameObject::Destroy();
}

RESULT MapTile::Ready(string meshID, vec3 vPos, vec3 vRot, vec3 vScale)
{
	//Clone.Mesh
	m_pMesh = CloneComponent<CMesh*>(meshID);
	if (nullptr != m_pMesh)
	{
		AttachComponent("Mesh", m_pMesh);
		m_pMesh->SetTransform(m_pTransform);
	}

	if (nullptr != m_pTransform)
	{
		m_pTransform->SetPosRotScale(vPos, vRot, vScale);
		m_pTransform->Update(0);
	}

	return PK_NOERROR;
}

MapTile* MapTile::Create(string meshID, vec3 vPos, vec3 vRot, vec3 vScale)
{
	MapTile* pInstance = new MapTile();
	if (PK_NOERROR != pInstance->Ready(meshID, vPos, vRot, vScale))
	{
		pInstance->Destroy();
		pInstance = nullptr;
	}

	return pInstance;
}
