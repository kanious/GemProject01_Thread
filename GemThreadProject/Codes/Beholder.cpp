#include "Beholder.h"
#include "ComponentMaster.h"
#include "Layer.h"
#include "Function.h"
#include "Component.h"
#include "Transform.h"
#include "Mesh.h"
#include "Renderer.h"
#include "BoundingBox.h"
#include "OpenGLDefines.h"
#include "OpenGLDevice.h"
#include "SoundMaster.h"
#include "LightMaster.h"
#include "Shader.h"
#include "LightMaster.h"
#include "Light.h"
#include "DefaultCamera.h"
#include "MazeManager.h"
#include "Define.h"


USING(Engine)
USING(glm)
USING(std)

Beholder::Beholder()
	: m_pMesh(nullptr), m_bCanRender(false), m_eState(STATE_NONE), m_eDir(DIR_DOWN), m_eTargetDir(DIR_NONE)
	, m_fMoveSpeed(3.f), m_fRotSpeed(3.f), m_bKilled(false)
{
	m_pMazeManager = MazeManager::GetInstance();
	m_bDebug = false;
}

Beholder::~Beholder()
{
}

void Beholder::SetThread()
{
	m_hActionBeholder = CreateThread(NULL, 0, ActionBeholderThread, this, 0, nullptr);
}

void Beholder::Kill()
{
	m_bKilled = true;

	m_eState = STATE_DYING;

	if (nullptr != m_pMesh)
		m_pMesh->SetTexture("beholder_d_Tex");
}

_ulong __stdcall Beholder::ActionBeholderThread(LPVOID param)
{
	Beholder* beholder = (Beholder*)param;

	if (nullptr == beholder->m_pMazeManager)
		beholder->m_pMazeManager = MazeManager::GetInstance();

	srand((unsigned int)time(NULL));

	while (true)
	{
		Sleep(100);

		if (beholder->m_bKilled)
			return 0;

		// Check Target
		switch (beholder->m_eState)
		{
		case STATE_CHASING_MOVE:
		case STATE_CHASING_TURN:
		case STATE_CHASING_ATTACK:
			if (nullptr != beholder->m_pTarget)
			{
				if (beholder->m_pTarget->GetKilled())
				{
					beholder->m_eState = STATE_NONE;
					beholder->m_pTarget = nullptr;
				}
			}
			break;
		}

		// Check Sight
		Beholder* newTarget = nullptr;
		_float dist = 0.f;
		if (beholder->m_pMazeManager->CheckSight(beholder, beholder->m_eDir, newTarget, dist))
		{
			int iX, iZ;
			beholder->m_pTarget = newTarget;
			beholder->GetIDByPos(newTarget->GetPosition(), iX, iZ);
			MapTileInfo tile = beholder->m_pMazeManager->GetTileInfo(iX, iZ);
			beholder->m_vTargetPos = tile.m_vPos;
			beholder->m_eTargetDir = newTarget->GetDir();
			if (dist < 5.f)
				beholder->m_eState = STATE_CHASING_ATTACK;
			else
				beholder->m_eState = STATE_CHASING_MOVE;
		}

		// Find a way to roam
		if (STATE_NONE == beholder->m_eState)
		{
			vec3 vPos = beholder->GetPosition();
			int iX, iZ;
			beholder->GetIDByPos(vPos, iX, iZ);
			if (beholder->m_pMazeManager->CheckPrevDir(iX, iZ, beholder->m_eDir))
			{
				MapTileInfo tileInfo = beholder->m_pMazeManager->GetTargetTileInfo(iX, iZ, beholder->m_eDir);
				beholder->m_vTargetPos = tileInfo.m_vPos;
				beholder->m_eState = STATE_ROAMING_MOVE;
			}
			else
			{
				vector<eDir> vecDir;
				beholder->m_pMazeManager->GetDirToRoaming(iX, iZ, vecDir);
				if (vecDir.size() > 0)
				{
					random_shuffle(vecDir.begin(), vecDir.end());
					eDir pickedDir = vecDir[0];

					MapTileInfo tileInfo = beholder->m_pMazeManager->GetTargetTileInfo(iX, iZ, pickedDir);
					EnterCriticalSection(&beholder->m_CSBeholderKey);
					beholder->m_eTargetDir = pickedDir;
					beholder->m_vTargetPos = tileInfo.m_vPos;
					if (beholder->m_eDir != beholder->m_eTargetDir)
						beholder->m_eState = STATE_ROAMING_TURN;
					else
						beholder->m_eState = STATE_ROAMING_MOVE;
					LeaveCriticalSection(&beholder->m_CSBeholderKey);
				}
			}
		}

		// Check Distance from Camera Target (For Culling)
		vec3 vCameraPos = beholder->m_pCamera->GetCameraTarget();
		vec3 vMyPos = beholder->GetPosition();
		_float dist2 = distance(vCameraPos, vMyPos);

		if (dist2 > 110.f)
			beholder->SetRenderState(false);
		else
			beholder->SetRenderState(true);
	}

	return 0;
}

void Beholder::GetIDByPos(glm::vec3 pos, int& iX, int& iZ)
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

// Basic Update Function
void Beholder::Update(const _float& dt)
{
	switch (m_eState)
	{
	case STATE_CHASING_MOVE:
	{
		vec3 vPos = m_pTransform->GetPosition();

		float dist = distance(vPos, m_vTargetPos);
		if (dist < 0.1f)
		{
			m_pTransform->SetPosition(m_vTargetPos);
			m_eState = STATE_CHASING_TURN;
		}
		else
		{
			vec3 vDir = m_vTargetPos - vPos;
			vDir = normalize(vDir);
			m_pTransform->AddPosition(vDir * dt * m_fMoveSpeed);
		}
	}
		break;

	case STATE_CHASING_TURN:
	{
		vec3 vLook = m_pTransform->GetLookVector();
		vec3 vDir = vec3(0.f);
		switch (m_eTargetDir)
		{
		case DIR_LEFT:		vDir = vec3(-1.f, 0.f, 0.f);	break;
		case DIR_RIGHT:		vDir = vec3(1.f, 0.f, 0.f);		break;
		case DIR_UP:		vDir = vec3(0.f, 0.f, -1.f);	break;
		case DIR_DOWN:		vDir = vec3(0.f, 0.f, 1.f);		break;
		}
		_float fAngleGap = GetAngle(vLook, vDir);
		if (5.f < fAngleGap)
		{
			vec3 vRight = m_pTransform->GetRightVector();
			_float fDot = dot(vDir, vRight);
			_float fY = m_pTransform->GetRotationY();
			if (0.f <= fDot)
			{
				fY += fAngleGap * dt * m_fRotSpeed;
				if (fY > 360.f)
					fY -= 360.f;
			}
			else
			{
				fY -= fAngleGap * dt * m_fRotSpeed;
				if (fY < 0.f)
					fY += 360.f;
			}
			m_pTransform->SetRotationY(fY);
		}
		else
		{
			switch (m_eTargetDir)
			{
			case DIR_LEFT:		m_pTransform->SetRotationY(270.f);	break;
			case DIR_RIGHT:		m_pTransform->SetRotationY(90.f);	break;
			case DIR_UP:		m_pTransform->SetRotationY(180.f);	break;
			case DIR_DOWN:		m_pTransform->SetRotationY(0.f);	break;
			}

			m_eDir = m_eTargetDir;
			m_eState = STATE_NONE;
		}
	}
		break;

	case STATE_CHASING_ATTACK:
		if (nullptr != m_pTarget)
		{
			m_pTarget->Kill();
			m_pTarget = nullptr;
		}
		m_eState = STATE_NONE;
		break;

	case STATE_ROAMING_MOVE:
	{
		vec3 vPos = m_pTransform->GetPosition();

		float dist = distance(vPos, m_vTargetPos);
		if (dist < 0.1f)
		{
			m_pTransform->SetPosition(m_vTargetPos);
			m_eState = STATE_NONE;
		}
		else
		{
			vec3 vDir = m_vTargetPos - vPos;
			vDir = normalize(vDir);
			m_pTransform->AddPosition(vDir * dt * m_fMoveSpeed);
		}
	}
	break;

	case STATE_ROAMING_TURN:
	{
		vec3 vLook = m_pTransform->GetLookVector();
		vec3 vDir = vec3(0.f);
		switch (m_eTargetDir)
		{
		case DIR_LEFT:		vDir = vec3(-1.f, 0.f, 0.f);	break;
		case DIR_RIGHT:		vDir = vec3(1.f, 0.f, 0.f);		break;
		case DIR_UP:		vDir = vec3(0.f, 0.f, -1.f);	break;
		case DIR_DOWN:		vDir = vec3(0.f, 0.f, 1.f);		break;
		}
		_float fAngleGap = GetAngle(vLook, vDir);
		if (5.f < fAngleGap)
		{
			vec3 vRight = m_pTransform->GetRightVector();
			_float fDot = dot(vDir, vRight);
			_float fY = m_pTransform->GetRotationY();
			if (0.f <= fDot)
			{
				fY += fAngleGap * dt * m_fRotSpeed;
				if (fY > 360.f)
					fY -= 360.f;
			}
			else
			{
				fY -= fAngleGap * dt * m_fRotSpeed;
				if (fY < 0.f)
					fY += 360.f;
			}
			m_pTransform->SetRotationY(fY);
		}
		else
		{
			switch (m_eTargetDir)
			{
			case DIR_LEFT:		m_pTransform->SetRotationY(270.f);	break;
			case DIR_RIGHT:		m_pTransform->SetRotationY(90.f);	break;
			case DIR_UP:		m_pTransform->SetRotationY(180.f);	break;
			case DIR_DOWN:		m_pTransform->SetRotationY(0.f);	break;
			}

			m_eDir = m_eTargetDir;
			m_eState = STATE_ROAMING_MOVE;
		}
	}
	break;

	case STATE_DYING:
	{
		vec3 vScale = m_pTransform->GetScale();
		vScale -= dt * 0.1f;
		m_pTransform->SetScale(vScale);

		_float fY = m_pTransform->GetRotationY();
		fY += dt * m_fRotSpeed * 300.f;
		if (fY > 360.f)
			fY -= 360.f;
		m_pTransform->SetRotationY(fY);

		if (vScale.x < 0.5f)
		{
			m_pTransform->SetScale(vec3(0.5f));
			m_eState = STATE_DEAD;
		}
	}
		break;

	case STATE_DEAD:
	{
		vec3 vCameraPos = m_pCamera->GetCameraTarget();
		vec3 vMyPos = GetPosition();
		_float dist2 = distance(vCameraPos, vMyPos);

		if (dist2 > 110.f)
			SetRenderState(false);
		else
			SetRenderState(true);
	}
		break;

	case STATE_NONE:
		break;
	}
	
	CGameObject::Update(dt);

	if (m_bCanRender)
	{
		if (nullptr != m_pRenderer)
			m_pRenderer->AddRenderObj(this);
	}
}

// Basic Render Function
void Beholder::Render()
{
	CGameObject::Render();
}

// Call instead of destructor to manage class internal data
void Beholder::Destroy()
{
	m_bKilled = true;

	TerminateThread(m_hActionBeholder, 1);
	DeleteCriticalSection(&m_CSBeholderKey);

	CGameObject::Destroy();
}

// Initialize
RESULT Beholder::Ready(_uint sTag, _uint lTag, _uint oTag, CLayer* pLayer, string meshID, vec3 vPos, vec3 vRot, vec3 vScale)
{
	SetupGameObject(sTag, lTag, oTag);
	m_pLayer = pLayer;
	m_meshName = meshID;

	//Clone.Mesh
 	m_pMesh = CloneComponent<CMesh*>(meshID);
	if (nullptr != m_pMesh)
	{
		AttachComponent("Mesh", m_pMesh);
		m_pMesh->SetTransform(m_pTransform);
		m_pBoundingBox = m_pMesh->GetBoundingBox();
		if (nullptr != m_pBoundingBox)
			m_pBoundingBox->SetTransform(m_pTransform);
		m_pMesh->SetWireFrame(false);
		m_pMesh->SetDebugBox(false);
		m_pMesh->SetTransparency(false);
	}

	if (nullptr != m_pTransform)
	{
		m_pTransform->SetPosRotScale(vPos, vRot, vScale);
		m_pTransform->Update(0);
	}

	InitializeCriticalSection(&m_CSBeholderKey);

	return PK_NOERROR;
}

// Create an instance
Beholder* Beholder::Create(_uint sTag, _uint lTag, _uint oTag, CLayer* pLayer, string meshID, vec3 vPos, vec3 vRot, vec3 vScale)
{
	Beholder* pInstance = new Beholder();
	if (PK_NOERROR != pInstance->Ready(sTag, lTag, oTag, pLayer, meshID, vPos, vRot, vScale))
	{
		pInstance->Destroy();
		pInstance = nullptr;
	}

	return pInstance;
}
