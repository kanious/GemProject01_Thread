#include "DefaultCamera.h"
#include "ComponentMaster.h"
#include "Layer.h"

#include "Component.h"
#include "Camera.h"
#include "Transform.h"
#include "OpenGLDefines.h"
#include "InputDevice.h"
#include "Shader.h"
#include "ComponentMaster.h"
#include "Component.h"

USING(Engine)
USING(glm)
USING(std)

DefaultCamera::DefaultCamera()
	: m_pCamera(nullptr), m_bMouseEnable(true), m_fAngleY(0.f), m_fAngleX(0.f)
	, m_fSpeed(30.f), m_fRotSpeed(20.f), m_fDistance(50.f)
	, m_iZoomLevel(5), m_iZoomMinLevel(1), m_iZoomMaxLevel(10)
{
	m_pInputDevice = CInputDevice::GetInstance(); m_pInputDevice->AddRefCnt();
}

DefaultCamera::~DefaultCamera()
{
}

vec3 DefaultCamera::GetCameraEye()
{
	if (nullptr == m_pCamera)
		return vec3(0.f);

	return m_pCamera->GetCameraEye();
}

vec3 DefaultCamera::GetCameraRot()
{
	if (nullptr == m_pTransform)
		return vec3(0.f);

	return m_pTransform->GetRotation();
}

vec3 DefaultCamera::GetCameraTarget()
{
	if (nullptr == m_pCamera)
		return vec3(0.f);

	return m_pCamera->GetCameraTarget();
}

void DefaultCamera::SetMouseEnable(_bool enable)
{
	m_bMouseEnable = enable;
	if (enable)
		m_pInputDevice->SetMouseCursorMode(GLFW_CURSOR_NORMAL);
	else
		m_pInputDevice->SetMouseCursorMode(GLFW_CURSOR_DISABLED);

	m_pInputDevice->InitMousePos();
}

void DefaultCamera::SetCameraEye(glm::vec3 eye)
{
	if (nullptr == m_pCamera)
		return;

	m_pCamera->SetCameraEye(eye);
}

void DefaultCamera::SetCameraRot(glm::vec3 rot)
{
	if (nullptr == m_pTransform)
		return;

	m_pTransform->SetRotation(rot);
}

void DefaultCamera::SetCameraTarget(glm::vec3 target)
{
	if (nullptr == m_pCamera)
		return;

	m_pCamera->SetCameraTarget(target);
}

// Save shaders to deliver camera information
void DefaultCamera::AddShaderLocation(string shaderTag)
{
	CComponent* shader = CComponentMaster::GetInstance()->FindComponent(shaderTag);
	if (nullptr != shader)
	{
		_uint shaderID = dynamic_cast<CShader*>(shader)->GetShaderProgram();
		_uint shaderLocation = glGetUniformLocation(shaderID, "eyeLocation");
		m_vecShaders.push_back(shaderLocation);
	}
}

// Check user input
void DefaultCamera::KeyCheck(const _float& dt)
{
	if (nullptr == m_pInputDevice || nullptr == m_pCamera)
		return;

	if (m_pInputDevice->IsKeyDown(GLFW_KEY_W))
	{
		vec3 vEye = m_pCamera->GetCameraEye();
		vec3 vTarget = m_pCamera->GetCameraTarget();
		vEye.z -= dt * 20.f;
		vTarget.z -= dt * 20.f;
		m_pCamera->SetCameraEye(vEye);
		m_pCamera->SetCameraTarget(vTarget);
	}

	if (m_pInputDevice->IsKeyDown(GLFW_KEY_S))
	{
		vec3 vEye = m_pCamera->GetCameraEye();
		vec3 vTarget = m_pCamera->GetCameraTarget();
		vEye.z += dt * 20.f;
		vTarget.z += dt * 20.f;
		m_pCamera->SetCameraEye(vEye);
		m_pCamera->SetCameraTarget(vTarget);
	}

	if (m_pInputDevice->IsKeyDown(GLFW_KEY_A))
	{
		vec3 vEye = m_pCamera->GetCameraEye();
		vec3 vTarget = m_pCamera->GetCameraTarget();
		vEye.x -= dt * 20.f;
		vTarget.x -= dt * 20.f;
		m_pCamera->SetCameraEye(vEye);
		m_pCamera->SetCameraTarget(vTarget);
	}

	if (m_pInputDevice->IsKeyDown(GLFW_KEY_D))
	{
		vec3 vEye = m_pCamera->GetCameraEye();
		vec3 vTarget = m_pCamera->GetCameraTarget();
		vEye.x += dt * 20.f;
		vTarget.x += dt * 20.f;
		m_pCamera->SetCameraEye(vEye);
		m_pCamera->SetCameraTarget(vTarget);
	}

	vec2 mouseScroll = m_pInputDevice->GetMouseScrollDistance();
	if (0 != mouseScroll.y)
	{
		if (0 < mouseScroll.y)
		{
			--m_iZoomLevel;
			if (m_iZoomLevel < m_iZoomMinLevel)
				m_iZoomLevel = m_iZoomMinLevel;
		}
		else
		{
			++m_iZoomLevel;
			if (m_iZoomLevel > m_iZoomMaxLevel)
				m_iZoomLevel = m_iZoomMaxLevel;
		}

		vec3 vTarget = m_pCamera->GetCameraTarget();
		vTarget.y += m_iZoomLevel * 20.f;
		vTarget.z += m_iZoomLevel * 15.f;
		m_pCamera->SetCameraEye(vTarget);
	}
}

// Basic Update Function
void DefaultCamera::Update(const _float& dt)
{
	KeyCheck(dt);

	CGameObject::Update(dt);

	vec3 vEye = m_pCamera->GetCameraEye();

	for (int i = 0; i < m_vecShaders.size(); ++i)
		glUniform4f(m_vecShaders[i], vEye.x, vEye.y, vEye.z, 1.0f);
}

// Call instead of destructor to manage class internal data
void DefaultCamera::Destroy()
{
	SafeDestroy(m_pInputDevice);

	CGameObject::Destroy();
}

// Initialize
RESULT DefaultCamera::Ready(_uint sTag, _uint lTag, _uint oTag, Engine::CLayer* pLayer, vec3 vPos, vec3 vRot, vec3 vScale, _float fov, _float fNear, _float fFar)
{
	SetupGameObject(sTag, lTag, oTag);
	m_pLayer = pLayer;

	CComponentMaster* pMaster = CComponentMaster::GetInstance();
	CComponent* pComponent = nullptr;

	//Clone.Camera
	m_pCamera = CloneComponent<CCamera*>("Camera");
	AttachComponent("Camera", m_pCamera);

	if (nullptr != m_pTransform)
	{
		m_pTransform->SetPosition(vPos);
		m_pTransform->SetRotation(vRot);
		m_pTransform->SetScale(vScale);
		m_pTransform->Update(0);

		if (nullptr != m_pCamera)
			m_pCamera->InitCameraSetting(m_pTransform->GetPosition(), m_pTransform->GetLookVector(), vec3(0.f, 1.f, 0.f), fov, fNear, fFar);
	}
	return PK_NOERROR;
}

// Create an instance
DefaultCamera* DefaultCamera::Create(_uint sTag, _uint lTag, _uint oTag, Engine::CLayer* pLayer, vec3 vPos, vec3 vRot, vec3 vScale, _float fov, _float fNear, _float fFar)
{
	DefaultCamera* pInstance = new DefaultCamera();
	if (PK_NOERROR != pInstance->Ready(sTag, lTag, oTag, pLayer, vPos, vRot, vScale, fov, fNear, fFar))
	{
		pInstance->Destroy();
		pInstance = nullptr;
	}

	return pInstance;
}