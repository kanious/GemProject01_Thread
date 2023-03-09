#ifndef _MAZEMANAGER_H_
#define _MAZEMANAGER_H_

#include "Base.h"
#include <Windows.h>
#include "MapTileInfo.h"
#include "Enums.h"

class SceneDungeon;
class MapTile;
class DefaultCamera;
class Beholder;
class MazeManager : public Engine::CBase
{
	SINGLETON(MazeManager)

private:
	std::vector<std::vector<MapTileInfo>>	m_vecTiles;
	std::vector<Beholder*>					m_vecBeholders;
	std::vector<glm::vec3>					m_vecPos;
	SceneDungeon*				m_pScene;
	DefaultCamera*				m_pCamera;

	// Map Tile for rendering
	MapTile*					m_pFloor;
	MapTile*					m_pWall;

	// Thread for map
	HANDLE						m_hCreateMap;
	CRITICAL_SECTION			m_CSMapKey;
	_bool						m_bCreateDone;

	// Thread for beholder
	HANDLE						m_hCreateBeholder;
	CRITICAL_SECTION			m_CSBeholderKey;

	// Culling Index
	_int						m_iX;
	_int m_iMinX;
	_int m_iMaxX;
	_int						m_iZ;
	_int m_iMinZ;
	_int m_iMaxZ;
	

private:
	explicit MazeManager();
	~MazeManager();
public:
	void Destroy();
	RESULT Ready(SceneDungeon* pScene);

public:
	void Update(const _float& dt);
	void Render();

public:
	static _ulong WINAPI MakeMazeThread(LPVOID param);
	static _ulong WINAPI MakeBeholderThread(LPVOID param);
	void SetCamera(DefaultCamera* pCamera)		{ m_pCamera = pCamera; }
private:
	void GetCameraEyePosIndex();
	void CalculateCullingIndex();
	void GetIDByPos(glm::vec3 pos, int& iX, int& iZ);
	_bool CheckBlockBetweenPositions(glm::vec3 srcPos, glm::vec3 destPos, eDir dir);
public:
	_bool CheckPrevDir(int iX, int iZ, eDir dir);
	void GetDirToRoaming(int iX, int iZ, std::vector<eDir>& vector);
	MapTileInfo GetTargetTileInfo(int iX, int iZ, eDir dir);
	MapTileInfo GetTileInfo(int iX, int iZ);
	_bool CheckSight(Beholder* me, eDir dir, Beholder*& target, _float& dist);
};

#endif //_MAZEMANAGER_H_