#include "ObjectFactory.h"
#include "GameObject.h"
#include "Layer.h"
#include "Enums.h"
#include "DefaultCamera.h"
#include "MapTile.h"
#include "Beholder.h"


USING(Engine)
USING(glm)
USING(std)

// Create GameObject
Beholder* ObjectFactory::CreateBeholder(_uint sTag, _uint lTag, _uint oTag, CLayer* pLayer,
    string meshID, vec3 vPos, vec3 vRot, vec3 vScale)
{
    CGameObject* pGameObject = Beholder::Create(sTag, lTag, oTag, pLayer, meshID, vPos, vRot, vScale);

    if (nullptr != pGameObject)
        pLayer->AddGameObject(pGameObject);

    return dynamic_cast<Beholder*>(pGameObject);
}

// Create camera
DefaultCamera* ObjectFactory::CreateCamera(_uint sTag, _uint lTag, _uint oTag, CLayer* pLayer,
    vec3 vPos, vec3 vRot, vec3 vScale, _float fov, _float fNear, _float fFar)
{
    CGameObject* pGameObject = DefaultCamera::Create(sTag, lTag, oTag, pLayer, vPos, vRot, vScale, fov, fNear, fFar);

    if (nullptr != pGameObject)
    {
        pLayer->AddGameObject(pGameObject);
        return dynamic_cast<DefaultCamera*>(pGameObject);
    }

    return nullptr;
}

MapTile* ObjectFactory::CreateMapTile(std::string meshID, vec3 vPos, vec3 vRot, vec3 vScale)
{
    return MapTile::Create(meshID, vPos, vRot, vScale);
}
