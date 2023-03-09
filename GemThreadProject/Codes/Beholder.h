#ifndef _BEHOLDER_H_
#define _BEHOLDER_H_

#include "GameObject.h"
#include "Enums.h"
#include <Windows.h>

namespace Engine
{
	class CMesh;
}
class DefaultCamera;
class MazeManager;

// Static Object class
class Beholder : public Engine::CGameObject
{
private:
	Engine::CMesh*					m_pMesh;
	MazeManager*					m_pMazeManager;
	_uint							m_iBeholderID;

	_bool							m_bCanRender;
	DefaultCamera*					m_pCamera;
	eState							m_eState;
	eDir							m_eDir;
	_bool							m_bKilled;

	// Target
	glm::vec3						m_vTargetPos;
	eDir							m_eTargetDir;
	Beholder*						m_pTarget;

	_float							m_fMoveSpeed;
	_float							m_fRotSpeed;

	// Thread
	HANDLE							m_hActionBeholder;
	CRITICAL_SECTION				m_CSBeholderKey;

private:
	explicit Beholder();
	virtual ~Beholder();

public:
	void SetCamera(DefaultCamera* pCamera)		{ m_pCamera = pCamera; }
	void SetRenderState(_bool value)			{ m_bCanRender = value; }
	void SetState(eState state)					{ m_eState = state; }
	void SetDir(eDir dir)						{ m_eDir = dir; }
	void SetTargetPos(glm::vec3 vPos)			{ m_vTargetPos = vPos; }
	void SetBeholderID(_uint id)				{ m_iBeholderID = id; }
	void SetThread();
	eState GetState()							{ return m_eState; }
	eDir GetDir()								{ return m_eDir; }
	glm::vec3 GetTargetPos()					{ return m_vTargetPos; }
	_uint GetBeholderID()						{ return m_iBeholderID; }
	_bool GetKilled()							{ return m_bKilled; }
	void Kill();

public:
	static _ulong WINAPI ActionBeholderThread(LPVOID param);
private:
	void GetIDByPos(glm::vec3 pos, int& iX, int& iZ);

public:
	virtual void Update(const _float& dt);
	virtual void Render();
private:
	virtual void Destroy();
	RESULT Ready(_uint sTag, _uint lTag, _uint oTag, Engine::CLayer* pLayer, std::string meshID, glm::vec3 vPos, glm::vec3 vRot, glm::vec3 vScale);
public:
	static Beholder* Create(_uint sTag, _uint lTag, _uint oTag, Engine::CLayer* pLayer, std::string meshID, glm::vec3 vPos, glm::vec3 vRot, glm::vec3 vScale);
};

#endif //_BEHOLDER_H_