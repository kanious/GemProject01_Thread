#include "MazeManager.h"
#include "Define.h"
#include "MapTile.h"
#include "Maze\cMazeMaker_W2023.h"
#include "ObjectFactory.h"
#include "CollisionMaster.h"
#include "DefaultCamera.h"
#include "Beholder.h"
#include "SceneDungeon.h"
#include <algorithm>

SINGLETON_FUNCTION(MazeManager)
USING(Engine)
USING(std)
USING(glm)

MazeManager::MazeManager()
	: m_pFloor(nullptr), m_pWall(nullptr), m_pCamera(nullptr), m_bCreateDone(false), m_pScene(nullptr)
{
	m_vecTiles.clear();
	m_vecBeholders.clear();
	m_vecPos.clear();
}

MazeManager::~MazeManager()
{
}

void MazeManager::Destroy()
{
	TerminateThread(m_hCreateMap, 1);
	TerminateThread(m_hCreateBeholder, 1);

	DeleteCriticalSection(&m_CSMapKey);
	DeleteCriticalSection(&m_CSBeholderKey);

	SafeDestroy(m_pFloor);
	SafeDestroy(m_pWall);

	for (_uint i = 0; i < m_vecTiles.size(); ++i)
		m_vecTiles[i].clear();
	m_vecTiles.clear();

	m_vecBeholders.clear();
	m_vecPos.clear();
}

RESULT MazeManager::Ready(SceneDungeon* pScene)
{
	m_pScene = pScene;

	InitializeCriticalSection(&m_CSMapKey);
	InitializeCriticalSection(&m_CSBeholderKey);

	// Meshes
	m_pFloor = ObjectFactory::CreateMapTile("floor", vec3(0.f), vec3(0.f), vec3(0.01f));
	m_pWall = ObjectFactory::CreateMapTile("floor2", vec3(0.f), vec3(0.f), vec3(0.01f, 0.2f, 0.01f));

	// Thread
	m_hCreateMap = CreateThread(NULL, 0, MakeMazeThread, this, 0, nullptr);
	m_hCreateBeholder = CreateThread(NULL, 0, MakeBeholderThread, this, 0, nullptr);

	return PK_NOERROR;
}

void MazeManager::Update(const _float& dt)
{
	GetCameraEyePosIndex();
	CalculateCullingIndex();
}

void MazeManager::Render()
{
	if (nullptr == m_pFloor || nullptr == m_pWall)
		return;

	for (int x = m_iMinX; x <= m_iMaxX; ++x)
	{
		if (m_vecTiles.size() <= x)
			break;

		for (int z = m_iMinZ; z <= m_iMaxZ; ++z)
		{
			if (m_vecTiles[x].size() <= z)
				break;

			EnterCriticalSection(&m_CSMapKey);

			vec3 vPos = m_vecTiles[x][z].m_vPos;
			if (m_vecTiles[x][z].m_bBlock)
			{
				m_pWall->SetPosition(vPos);
				m_pWall->Update(0.f);
				m_pWall->Render();
			}
			else
			{
				m_pFloor->SetPosition(vPos);
				m_pFloor->Update(0.f);
				m_pFloor->Render();
			}

			LeaveCriticalSection(&m_CSMapKey);
		}
	}
}

_ulong MazeManager::MakeMazeThread(LPVOID param)
{
	MazeManager* pManager = (MazeManager*)param;

	unsigned int size = TILE_COUNT;
	vec3 vPos = vec3((size / 2) * TILE_SIZE, 0.f, (size / 2) * TILE_SIZE);
	pManager->m_pCamera->SetCameraTarget(vPos);
	vPos.y += 100.f;
	vPos.z += 75.f;
	pManager->m_pCamera->SetCameraEye(vPos);

	cout << "Generating Maze...";
	cMazeMaker_W2023 theMM;
	theMM.GenerateMaze(size, size);
	cout << "Done!" << endl;

	vector<vector<vector<bool>>> maze = theMM.maze;
	//theMM.PrintMaze();

	cout << "Making Map Info...";
	for (unsigned int i = 0; i < theMM.maze.size(); ++i)
	{
		vector<MapTileInfo> newVector;
		newVector.clear();
		for (unsigned int j = 0; j < theMM.maze[i].size(); ++j)
		{
			MapTileInfo newInfo;
			newInfo.m_iTileX = j;
			newInfo.m_iTileZ = i;
			newInfo.m_vPos = vec3(j * TILE_SIZE, 0.f, i * TILE_SIZE);
			newInfo.m_bBlock = theMM.maze[i][j][0];
			newVector.push_back(newInfo);

			if (!newInfo.m_bBlock && 
				newInfo.m_vPos.x > vPos.x - BEHOLDER_SPAWN_RANGE && newInfo.m_vPos.x < vPos.x + BEHOLDER_SPAWN_RANGE &&
				newInfo.m_vPos.z > vPos.z - BEHOLDER_SPAWN_RANGE && newInfo.m_vPos.z < vPos.z + BEHOLDER_SPAWN_RANGE)
			{
				pManager->m_vecPos.push_back(newInfo.m_vPos);
			}
		}
		EnterCriticalSection(&pManager->GetInstance()->m_CSMapKey);
		pManager->m_vecTiles.push_back(newVector);
		LeaveCriticalSection(&pManager->GetInstance()->m_CSMapKey);
	}
	cout << "Done!" << endl;

	pManager->m_bCreateDone = true;

	return 0;
}

_ulong __stdcall MazeManager::MakeBeholderThread(LPVOID param)
{
	MazeManager* pManager = (MazeManager*)param;

	if (nullptr == pManager->m_pScene)
		return 0;

	while (!pManager->m_bCreateDone)
	{
		Sleep(500);
	}

	srand((unsigned int)time(NULL));
	random_shuffle(pManager->m_vecPos.begin(), pManager->m_vecPos.end());

	_uint maxCount = BEHOLDER_COUNT;
	if (pManager->m_vecPos.size() < BEHOLDER_COUNT)
		maxCount = pManager->m_vecPos.size() - 1;

	cout << "Making Beholders...";
	for (_uint i = 0; i < maxCount; ++i)
	{
		vec3 vPos = pManager->m_vecPos[i];
		Beholder* newBeholder = ObjectFactory::CreateBeholder(
			(_uint)SCENE_3D, (_uint)LAYER_CHARACTER, (_uint)OBJ_CHARACTER,
			pManager->m_pScene->GetLayer((_uint)LAYER_CHARACTER),
			"beholder", vPos, vec3(0.f), vec3(1.0f));

		if (nullptr != newBeholder)
		{
			newBeholder->SetCamera(pManager->m_pCamera);
			newBeholder->SetThread();
			newBeholder->SetBeholderID(i);

			EnterCriticalSection(&pManager->GetInstance()->m_CSBeholderKey);
			pManager->m_vecBeholders.push_back(newBeholder);
			LeaveCriticalSection(&pManager->GetInstance()->m_CSBeholderKey);
		}
	}
	cout << "Done!" << endl;

	return 0;
}

void MazeManager::GetCameraEyePosIndex()
{
	if (nullptr == m_pCamera)
		return;

	vec3 vCameraPos = m_pCamera->GetCameraEye();
	vec3 vCameraTarget = m_pCamera->GetCameraTarget();
	vec3 vDir = vCameraTarget - vCameraPos;
	vDir = normalize(vDir);

	vec3 vDest = vec3(0.f);
	if (CCollisionMaster::GetInstance()->IntersectRayToVirtualPlane(100000.f, vCameraPos, vDir, vDest))
	{
		m_iZ = (int)(vDest.x / TILE_SIZE);
		if (HALF_TILE_SIZE <= vDest.x - (m_iZ * TILE_SIZE))
			++m_iZ;
		if (0 > m_iZ)
			m_iZ = 0;

		m_iX = (int)(vDest.z / TILE_SIZE);
		if (HALF_TILE_SIZE <= vDest.z - (m_iX * TILE_SIZE))
			++m_iX;
		if (0 > m_iX)
			m_iX = 0;
	}

}

void MazeManager::CalculateCullingIndex()
{
	if (m_iX >= m_vecTiles.size())
		m_iMinX = m_vecTiles.size() - PRINT_COUNT - 1;
	else
	{
		m_iMinX = m_iX - PRINT_COUNT;
		if (m_iMinX < 0)
			m_iMinX = 0;
	}

	m_iMaxX = m_iX + PRINT_COUNT;
	if (m_iMaxX >= m_vecTiles.size())
	{
		m_iMaxX = m_vecTiles.size() - 1;
		if (0 > m_iMaxX)
		{
			if (m_vecTiles.size() < PRINT_COUNT)
				m_iMaxX = m_vecTiles.size();
			else
				m_iMaxX = PRINT_COUNT;
		}
	}

	if (m_iZ >= TILE_COUNT)
		m_iMinZ = TILE_COUNT - PRINT_COUNT - 1;
	else
	{
		m_iMinZ = m_iZ - PRINT_COUNT;
		if (m_iMinZ < 0)
			m_iMinZ = 0;
	}

	m_iMaxZ = m_iZ + PRINT_COUNT;
	if (m_iMaxZ >= TILE_COUNT)
		m_iMaxZ = TILE_COUNT - 1;
}

void MazeManager::GetIDByPos(glm::vec3 pos, int& iX, int& iZ)
{
	iZ = (int)(pos.x / TILE_SIZE);
	if (HALF_TILE_SIZE <= pos.x - (iZ * TILE_SIZE))
		++iZ;
	if (0 > iZ)
		iZ = 0;

	iX = (int)(pos.z / TILE_SIZE);
	if (HALF_TILE_SIZE <= pos.z - (iX * TILE_SIZE))
		++iX;
	if (0 > iX)
		iX = 0;
}

_bool MazeManager::CheckBlockBetweenPositions(glm::vec3 srcPos, glm::vec3 destPos, eDir dir)
{
	_int iSrcX, iSrcZ;
	_int iDestX, iDestZ;

	GetIDByPos(srcPos, iSrcX, iSrcZ);
	GetIDByPos(destPos, iDestX, iDestZ);

	switch (dir)
	{
	case DIR_LEFT:
		for (int i = iDestZ; i <= iSrcZ; ++i)
		{
			if (m_vecTiles[iSrcX][i].m_bBlock)
				return false;
		}
		break;

	case DIR_RIGHT:
		for (int i = iSrcZ; i <= iDestZ; ++i)
		{
			if (m_vecTiles[iSrcX][i].m_bBlock)
				return false;
		}
		break;

	case DIR_UP:
		for (int i = iDestX; i <= iSrcX; ++i)
		{
			if (m_vecTiles[i][iSrcZ].m_bBlock)
				return false;
		}
		break;

	case DIR_DOWN:
		for (int i = iSrcX; i <= iDestX; ++i)
		{
			if (m_vecTiles[i][iSrcZ].m_bBlock)
				return false;
		}
		break;
	}

	return true;
}

_bool MazeManager::CheckPrevDir(int iX, int iZ, eDir dir)
{
	switch (dir)
	{
	case DIR_LEFT:
		if (iZ - 1 >= 0)
		{
			if (!m_vecTiles[iX][iZ - 1].m_bBlock)
				return true;
		}
		break;

	case DIR_RIGHT:
		if (iZ + 1 < m_vecTiles[iX].size())
		{
			if (!m_vecTiles[iX][iZ + 1].m_bBlock)
				return true;
		}
		break;

	case DIR_UP:
		if (iX - 1 >= 0)
		{
			if (!m_vecTiles[iX - 1][iZ].m_bBlock)
				return true;
		}
		break;

	case DIR_DOWN:
		if (iX + 1 < m_vecTiles.size())
		{
			if (!m_vecTiles[iX + 1][iZ].m_bBlock)
				return true;
		}
		break;
	}

	return false;
}

void MazeManager::GetDirToRoaming(int iX, int iZ, vector<eDir>& vector)
{
	if (iX < 0 || iX >= m_vecTiles.size())
		return;

	if (iZ < 0 || iZ >= m_vecTiles[iX].size())
		return;

	//Left
	if (iZ - 1 >= 0)
	{
		if (!m_vecTiles[iX][iZ - 1].m_bBlock)
			vector.push_back(DIR_LEFT);
	}

	//Right
	if (iZ + 1 < m_vecTiles[iX].size())
	{
		if (!m_vecTiles[iX][iZ + 1].m_bBlock)
			vector.push_back(DIR_RIGHT);
	}

	//Up
	if (iX - 1 >= 0)
	{
		if (!m_vecTiles[iX - 1][iZ].m_bBlock)
			vector.push_back(DIR_UP);
	}

	//Down
	if (iX + 1 < m_vecTiles.size())
	{
		if (!m_vecTiles[iX + 1][iZ].m_bBlock)
			vector.push_back(DIR_DOWN);
	}
}

MapTileInfo MazeManager::GetTargetTileInfo(int iX, int iZ, eDir dir)
{
	switch (dir)
	{
	case DIR_LEFT:	--iZ;	break;
	case DIR_RIGHT:	++iZ;	break;
	case DIR_UP:	--iX;	break;
	case DIR_DOWN:	++iX;	break;
	}

	return m_vecTiles[iX][iZ];
}

MapTileInfo MazeManager::GetTileInfo(int iX, int iZ)
{
	return m_vecTiles[iX][iZ];
}

_bool MazeManager::CheckSight(Beholder* me, eDir dir, Beholder*& target, _float& dist)
{
	vec3 myPos = me->GetPosition();

	Beholder* newTarget = nullptr;
	_float closestDist = 999.f;

	for (int i = 0; i < m_vecBeholders.size(); ++i)
	{
		if (m_vecBeholders[i]->GetKilled())
			continue;

		if (me->GetBeholderID() == m_vecBeholders[i]->GetBeholderID())
			continue;

		vec3 targetPos = m_vecBeholders[i]->GetPosition();

		// distance check first
		_float dist2 = distance(myPos, targetPos);
		if (30.f < dist2)
			continue;

		// direction check
		_bool isInSight = false;
		switch (dir)
		{
		case DIR_LEFT:
			if (targetPos.x <= myPos.x &&
				(targetPos.z > myPos.z - (HALF_TILE_SIZE / 2) && targetPos.z < myPos.z + (HALF_TILE_SIZE / 2)))
				isInSight = CheckBlockBetweenPositions(myPos, targetPos, dir);
			break;

		case DIR_RIGHT:
			if (targetPos.x >= myPos.x &&
				(targetPos.z > myPos.z - (HALF_TILE_SIZE / 2) && targetPos.z < myPos.z + (HALF_TILE_SIZE / 2)))
				isInSight = CheckBlockBetweenPositions(myPos, targetPos, dir);
			break;

		case DIR_UP:
			if (targetPos.z <= myPos.z &&
				(targetPos.x > myPos.x - (HALF_TILE_SIZE / 2) && targetPos.x < myPos.x + (HALF_TILE_SIZE / 2)))
				isInSight = CheckBlockBetweenPositions(myPos, targetPos, dir);
			break;

		case DIR_DOWN:
			if (targetPos.z >= myPos.z &&
				(targetPos.x > myPos.x - (HALF_TILE_SIZE / 2) && targetPos.x < myPos.x + (HALF_TILE_SIZE / 2)))
				isInSight = CheckBlockBetweenPositions(myPos, targetPos, dir);
			break;
		}

		if (isInSight)
		{
			if (dist2 < closestDist)
			{
				closestDist = dist2;
				newTarget = m_vecBeholders[i];
			}
		}
	}

	if (nullptr != newTarget)
	{
		dist = closestDist;
		target = newTarget;

		return true;
	}

	return false;
}
