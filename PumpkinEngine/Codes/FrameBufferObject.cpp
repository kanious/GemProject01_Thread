#include "pch.h"
#include "../Headers/FrameBufferObject.h"
#include "../Headers/OpenGLDefines.h"
#include "../Headers/Component.h"
#include "../Headers/ComponentMaster.h"
#include "../Headers/Shader.h"

USING(Engine)
USING(std)
USING(glm)

CFrameBufferObject::CFrameBufferObject()
    : m_iFrameBuffer(0), m_iColourTextureID(0), m_iRenderBuffer(0)
    , m_iWidth(0), m_iHeight(0)
    , m_iVAO_ID(0), m_iVB_ID(0)
    , m_pShader(nullptr)
{
}

CFrameBufferObject::CFrameBufferObject(const CFrameBufferObject& rhs)
    : m_iFrameBuffer(rhs.m_iFrameBuffer)
    , m_iColourTextureID(rhs.m_iColourTextureID)
    , m_iRenderBuffer(rhs.m_iRenderBuffer)
    , m_iWidth(rhs.m_iWidth), m_iHeight(rhs.m_iHeight)
    , m_iVAO_ID(rhs.m_iVAO_ID), m_iVB_ID(rhs.m_iVB_ID)
    , m_pShader(rhs.m_pShader)
{
    m_tag = rhs.m_tag;
}

CFrameBufferObject::~CFrameBufferObject()
{
}

// Call instead of destructor to manage class internal data
void CFrameBufferObject::Destroy()
{
    glDeleteTextures(1, &m_iFrameBuffer);
    glDeleteTextures(1, &m_iColourTextureID);
    glDeleteTextures(1, &m_iRenderBuffer);
    glDeleteVertexArrays(1, &m_iVAO_ID);
    glDeleteBuffers(1, &m_iVB_ID);
    //glDeleteTextures(1, &m_iVertexWorldPos_ID);
    //glDeleteTextures(1, &m_iDepthTextureID);
    SafeDestroy(m_pShader);

    CComponent::Destroy();
}

// Init Frame Buffer information
RESULT CFrameBufferObject::Ready(string ID, _int width, _int height)
{
    m_tag = ID;

    m_iWidth = width;
    m_iHeight = height;

    // Set screen quad VAO
    if (PK_NOERROR != SetVIBuffer())
        return PK_ERROR;

    // Set Screen Shader
    if (PK_NOERROR != SetShader())
        return PK_ERROR;

    // Set FrameBuffer
    if (PK_NOERROR != SetFrameBuffer())
        return PK_ERROR;

    return PK_NOERROR;
}

RESULT CFrameBufferObject::SetVIBuffer()
{
    float quadVertices[] = {
        // positions   // texCoords
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,

        -1.0f,  1.0f,  0.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f
    };

    //float quadVertices[] = {
    //    // positions   // texCoords
    //    -1.0f,  1.0f,  0.0f, 1.0f,
    //    -1.0f,  0.0f,  0.0f, 0.0f,
    //     0.0f,  0.0f,  1.0f, 0.0f,

    //    -1.0f,  1.0f,  0.0f, 1.0f,
    //     0.0f,  0.0f,  1.0f, 0.0f,
    //     0.0f,  1.0f,  1.0f, 1.0f
    //};
    
    //float quadVertices[] = {
    //    // positions   // texCoords
    //    -0.2f,  0.2f,  0.0f, 1.0f,
    //    -0.2f, -0.2f,  0.0f, 0.0f,
    //     0.2f, -0.2f,  1.0f, 0.0f,

    //    -0.2f,  0.2f,  0.0f, 1.0f,
    //     0.2f, -0.2f,  1.0f, 0.0f,
    //     0.2f,  0.2f,  1.0f, 1.0f
    //};

    glGenVertexArrays(1, &m_iVAO_ID);
    glBindVertexArray(m_iVAO_ID);

    glGenBuffers(1, &m_iVB_ID);
    glBindBuffer(GL_ARRAY_BUFFER, m_iVB_ID);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

    return PK_NOERROR;
}

RESULT CFrameBufferObject::SetShader()
{
    CComponent* pComponent = CloneComponent<CShader*>("ScreenShader");
    if (nullptr != pComponent)
    {
        m_pShader = dynamic_cast<CShader*>(pComponent);
        m_pShader->Use();
        m_pShader->SetInt("screenTexture", 0);
        return PK_NOERROR;
    }

    return PK_ERROR;
}

RESULT CFrameBufferObject::SetFrameBuffer()
{
    // FBO
    glGenFramebuffers(1, &m_iFrameBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, m_iFrameBuffer);

    // Colour
    glGenTextures(1, &m_iColourTextureID);
    glBindTexture(GL_TEXTURE_2D, m_iColourTextureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_iWidth, m_iHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_iColourTextureID, 0);

    // Render buffer
    glGenRenderbuffers(1, &m_iRenderBuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, m_iRenderBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, m_iWidth, m_iHeight);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_iRenderBuffer);



    // Check
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        return PK_ERROR;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);




    //glBindTexture(GL_TEXTURE_2D, m_iColourTextureID);
    //glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    //glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_iColourTextureID, 0);

    //// Depth,Stencil
    //glGenTextures(1, &m_iDepthTextureID);
    //glBindTexture(GL_TEXTURE_2D, m_iDepthTextureID);
    //glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, width, height, 0,
    //    GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);
    //glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, m_iDepthTextureID, 0);


    //glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGB8, width, height);

    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    //glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, black);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

    //// Vertex world position
    //glGenTextures(1, &m_iVertexWorldPos_ID);
    //glBindTexture(GL_TEXTURE_2D, m_iVertexWorldPos_ID);
    //glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, width, height);

    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    //glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, black);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

    //// Depth
    //glGenTextures(1, &m_iDepthTextureID);
    //glBindTexture(GL_TEXTURE_2D, m_iDepthTextureID);
    //glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH24_STENCIL8, width, height);


    //glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, m_iColourTextureID, 0);
    //glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, m_iVertexWorldPos_ID, 0);
    //glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, m_iDepthTextureID, 0);

    //static const GLenum draw_buffers[] =
    //{
    //    GL_COLOR_ATTACHMENT0,
    //    GL_COLOR_ATTACHMENT1
    //};
    //glDrawBuffers(2, draw_buffers);

    //_bool isFBOCreated = true;
    //switch (glCheckFramebufferStatus(GL_FRAMEBUFFER))
    //{
    //case GL_FRAMEBUFFER_COMPLETE:
    //    cout << m_tag << " created!" << endl;
    //    break;
    //case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
    //    cout << m_tag << " not created : GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT" << endl;
    //    return PK_FRAMEBUFFER_ERROR;
    //case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
    //    cout << m_tag << " not created : GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT" << endl;
    //    return PK_FRAMEBUFFER_ERROR;
    //case GL_FRAMEBUFFER_UNSUPPORTED:
    //    cout << m_tag << " not created : GL_FRAMEBUFFER_UNSUPPORTED" << endl;
    //    return PK_FRAMEBUFFER_ERROR;
    //default:
    //    cout << m_tag << " not created : UNKNOWN" << endl;
    //    return PK_FRAMEBUFFER_ERROR;
    //}

    //glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return PK_NOERROR;
}

void CFrameBufferObject::RenderStart()
{
    glBindFramebuffer(GL_FRAMEBUFFER, m_iFrameBuffer);
    glViewport(0, 0, m_iWidth, m_iHeight);
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.1f, 0.1f, 0.1f, 1.f);
}

void CFrameBufferObject::Render(_bool second)
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDisable(GL_DEPTH_TEST);
    glClearColor(1.f, 1.f, 1.f, 1.f);
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    if (!second)
    {
        glClear(GL_COLOR_BUFFER_BIT);
        m_pShader->Use();
        mat4x4 matWorld = mat4x4(1.f);
        m_pShader->SetMat4x4("matWorld", matWorld);
        glBindVertexArray(m_iVAO_ID);
        glActiveTexture(m_iColourTextureID);
        glBindTexture(GL_TEXTURE_2D, m_iColourTextureID);
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }
    else
    {
        m_pShader->Use();
        mat4x4 matWorld = mat4x4(1.f);
		matWorld = mat4x4(1.f);
		matWorld = translate(matWorld, vec3(1.f, 1.f, 0.f));
		//matWorld = scale(matWorld, vec3(0.5f, 0.5f, 1.f));
		m_pShader->SetMat4x4("matWorld", matWorld);
		glBindVertexArray(m_iVAO_ID);
		//glActiveTexture(m_iColourTextureID);
		//glBindTexture(GL_TEXTURE_2D, m_iColourTextureID);
		glDrawArrays(GL_TRIANGLES, 0, 6);
    }

    //matWorld = mat4x4(1.f);
    //matWorld = translate(matWorld, vec3(0.5f, 0.f, 0.f));
    //matWorld = scale(matWorld, vec3(0.5f, 0.5f, 1.f));
    //m_pShader->SetMat4x4("matWorld", matWorld);
    //glBindVertexArray(m_iVAO_ID);
    //glActiveTexture(m_iColourTextureID);
    //glBindTexture(GL_TEXTURE_2D, m_iColourTextureID);
    //glDrawArrays(GL_TRIANGLES, 0, 6);

    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    //m_pShader->Use();
    //if (second)
    //    m_pShader->SetBool("second", true);
    //else
    //    m_pShader->SetBool("second", false);
    //glBindVertexArray(m_iVAO_ID);
    //glActiveTexture(m_iColourTextureID);
    //glBindTexture(GL_TEXTURE_2D, m_iColourTextureID);
    //glDrawArrays(GL_TRIANGLES, 0, 6);
}

void CFrameBufferObject::RenderEnd()
{
    glBindVertexArray(m_iVAO_ID);
    glActiveTexture(m_iColourTextureID);
    glBindTexture(GL_TEXTURE_2D, m_iColourTextureID);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

// Clone component
CComponent* CFrameBufferObject::Clone()
{
    return new CFrameBufferObject(*this);
}

// Create an instance
CFrameBufferObject* CFrameBufferObject::Create(string ID, _int width, _int height)
{
    CFrameBufferObject* pInstance = new CFrameBufferObject();
    if (PK_NOERROR != pInstance->Ready(ID, width, height))
    {
        pInstance->Destroy();
        pInstance = nullptr;
    }

    return pInstance;
}
