#ifndef _FRAMEBUFFEROBJECT_H_
#define _FRAMEBUFFEROBJECT_H_

#include "Component.h"

NAMESPACE_BEGIN(Engine)

class CShader;
// Component for frame buffer object data
class ENGINE_API CFrameBufferObject : public CComponent
{
private:
	_uint				m_iFrameBuffer;
	_uint				m_iColourTextureID;
	_uint				m_iRenderBuffer;
	//_uint				m_iVertexWorldPos_ID;
	//_uint				m_iDepthTextureID;

	_int				m_iWidth;
	_int				m_iHeight;

	_uint				m_iVAO_ID;
	_uint				m_iVB_ID;

	CShader*			m_pShader;

private:
	explicit CFrameBufferObject();
	explicit CFrameBufferObject(const CFrameBufferObject& rhs);
	virtual ~CFrameBufferObject();

private:
	// Call instead of destructor to manage class internal data
	virtual void Destroy();

public:
	_uint GetFramebufferID()		{ return m_iFrameBuffer; }
	//_uint GetColourTextureID()	{ return m_iColourTextureID; }
	//_uint GetVertexWorldPosID()	{ return m_iVertexWorldPos_ID; }
	//_uint GetDepthTextureID()		{ return m_iDepthTextureID; }
	_int GetWidth()					{ return m_iWidth; }
	_int GetHeight()				{ return m_iHeight; }

private:
	// Load texture information from file
	RESULT Ready(std::string ID, _int width, _int height);
	RESULT SetVIBuffer();
	RESULT SetShader();
	RESULT SetFrameBuffer();
public:
	void RenderStart();
	void Render(_bool second);
	void RenderEnd();

public:
	// Clone component
	virtual CComponent* Clone();
	// Create an instance
	static CFrameBufferObject* Create(std::string ID, _int width, _int height);
};

NAMESPACE_END

#endif //_FRAMEBUFFEROBJECT_H_