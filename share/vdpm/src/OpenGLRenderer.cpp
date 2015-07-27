#include <cassert>
#include <cstddef>

#ifdef _WIN32
#include <windows.h>
#endif

#include <GL/gl.h>
#include "vdpm/OpenGLRenderer.h"
#include "vdpm/SRMesh.h"
#include "vdpm/Viewport.h"

using namespace std;
using namespace vdpm;

#define GL_BUFFER_SIZE 0x8764
#define GL_ARRAY_BUFFER 0x8892
#define GL_ELEMENT_ARRAY_BUFFER 0x8893
#define GL_STREAM_DRAW 0x88E0
#define GL_COPY_READ_BUFFER 0x8F36

#define GL_MAP_READ_BIT 0x0001
#define GL_MAP_WRITE_BIT 0x0002
#define GL_MAP_FLUSH_EXPLICIT_BIT 0x0010

typedef ptrdiff_t GLintptr;
typedef ptrdiff_t GLsizeiptr;

typedef void (APIENTRY * PFNGLMULTIDRAWELEMENTSPROC) (GLenum mode, const GLsizei *count, GLenum type, const void *const* indices, GLsizei drawcount);
typedef void (APIENTRY * PFNGLGENBUFFERSPROC) (GLsizei n, GLuint* buffers);
typedef void (APIENTRY * PFNGLBINDBUFFERPROC) (GLenum target, GLuint buffer);
typedef void (APIENTRY * PFNGLBUFFERDATAPROC) (GLenum target, GLsizeiptr size, const void* data, GLenum usage);
typedef void (APIENTRY * PFNGLBUFFERSUBDATAPROC) (GLenum target, GLintptr offset, GLsizeiptr size, const void* data);
typedef void (APIENTRY * PFNGLDELETEBUFFERSPROC) (GLsizei n, const GLuint* buffers);
typedef void (APIENTRY * PFNGLGETBUFFERPARAMETERIVPROC) (GLenum target, GLenum pname, GLint* params);
typedef void (APIENTRY * PFNGLCOPYBUFFERSUBDATAPROC) (GLenum readtarget, GLenum writetarget, GLintptr readoffset, GLintptr writeoffset, GLsizeiptr size);
typedef void * (APIENTRY * PFNGLMAPBUFFERRANGEPROC) (GLenum target, GLintptr offset, GLsizeiptr length, GLbitfield access);
typedef GLboolean(APIENTRY * PFNGLUNMAPBUFFERPROC) (GLenum target);
typedef void (APIENTRY * PFNGLFLUSHMAPPEDBUFFERRANGEPROC) (GLenum target, GLintptr offset, GLsizeiptr length);

static PFNGLMULTIDRAWELEMENTSPROC glMultiDrawElements;
static PFNGLGENBUFFERSPROC glGenBuffers;
static PFNGLBINDBUFFERPROC glBindBuffer;
static PFNGLBUFFERDATAPROC glBufferData;
static PFNGLBUFFERSUBDATAPROC glBufferSubData;
static PFNGLDELETEBUFFERSPROC glDeleteBuffers;
static PFNGLGETBUFFERPARAMETERIVPROC glGetBufferParameteriv;
static PFNGLCOPYBUFFERSUBDATAPROC glCopyBufferSubData;
static PFNGLMAPBUFFERRANGEPROC glMapBufferRange;
static PFNGLUNMAPBUFFERPROC glUnmapBuffer;
static PFNGLFLUSHMAPPEDBUFFERRANGEPROC glFlushMappedBufferRange;

OpenGLRenderer& OpenGLRenderer::getInstance()
{
    static OpenGLRenderer self;
    return self;
}

OpenGLRenderer::OpenGLRenderer()
{
    if (!glMultiDrawElements)
    {
        glMultiDrawElements = (PFNGLMULTIDRAWELEMENTSPROC)wglGetProcAddress("glMultiDrawElements");
        glGenBuffers = (PFNGLGENBUFFERSPROC)wglGetProcAddress("glGenBuffers");
        glBindBuffer = (PFNGLBINDBUFFERPROC)wglGetProcAddress("glBindBuffer");
        glBufferData = (PFNGLBUFFERDATAPROC)wglGetProcAddress("glBufferData");
        glBufferSubData = (PFNGLBUFFERSUBDATAPROC)wglGetProcAddress("glBufferSubData");
        glDeleteBuffers = (PFNGLDELETEBUFFERSPROC)wglGetProcAddress("glDeleteBuffers");
        glGetBufferParameteriv = (PFNGLGETBUFFERPARAMETERIVPROC)wglGetProcAddress("glGetBufferParameteriv");
        glCopyBufferSubData = (PFNGLCOPYBUFFERSUBDATAPROC)wglGetProcAddress("glCopyBufferSubData");
        glMapBufferRange = (PFNGLMAPBUFFERRANGEPROC)wglGetProcAddress("glMapBufferRange");
        glUnmapBuffer = (PFNGLUNMAPBUFFERPROC)wglGetProcAddress("glUnmapBuffer");
        glFlushMappedBufferRange = (PFNGLFLUSHMAPPEDBUFFERRANGEPROC)wglGetProcAddress("glFlushMappedBufferRange");
    }
}

OpenGLRenderer::~OpenGLRenderer()
{
    // do nothing
}

void* OpenGLRenderer::createBuffer(RendererBuffer target, unsigned int size, const void* data)
{
#ifdef VDPM_RENDERER_OPENGL_VBO
    GLenum gltarget = (target == RENDERER_VERTEX_BUFFER) ? GL_ARRAY_BUFFER : GL_ELEMENT_ARRAY_BUFFER;
    GLuint buf;
    glGenBuffers(1, &buf);
    glBindBuffer(gltarget, buf);
    glBufferData(gltarget, size, data, GL_STREAM_DRAW);
    glBindBuffer(gltarget, 0);
    return (void*)buf;
#else
    return Renderer::createBuffer(target, size, data);
#endif
}

void OpenGLRenderer::destroyBuffer(void* buf)
{
#ifdef VDPM_RENDERER_OPENGL_VBO
    glDeleteBuffers(1, (GLuint*)&buf);
#else
    Renderer::destroyBuffer(buf);
#endif
}

void OpenGLRenderer::setBufferData(RendererBuffer target, void* buf, unsigned int offset, unsigned int size, const void* data)
{
#ifdef VDPM_RENDERER_OPENGL_VBO
    GLenum gltarget = (target == RENDERER_VERTEX_BUFFER) ? GL_ARRAY_BUFFER : GL_ELEMENT_ARRAY_BUFFER;
    glBindBuffer(target, (GLuint)buf);
    glBufferSubData(target, offset, size, data);
    glBindBuffer(target, 0);
#else
    Renderer::setBufferData(target, buf, offset, size, data);
#endif
}

void* OpenGLRenderer::resizeBuffer(RendererBuffer target, void* buf, unsigned int size)
{
#ifdef VDPM_RENDERER_OPENGL_VBO
    GLenum gltarget = (target == RENDERER_VERTEX_BUFFER) ? GL_ARRAY_BUFFER : GL_ELEMENT_ARRAY_BUFFER;
    GLuint newbuf = 0;
    GLint oldsize = 0;
    glGenBuffers(1, &newbuf);
    glBindBuffer(GL_COPY_READ_BUFFER, newbuf);
    glBufferData(GL_COPY_READ_BUFFER, size, NULL, GL_STREAM_DRAW);

    glBindBuffer(gltarget, (GLuint)buf);
    glGetBufferParameteriv(gltarget, GL_BUFFER_SIZE, &oldsize);
    glCopyBufferSubData(gltarget, GL_COPY_READ_BUFFER, 0, 0, oldsize);
    glDeleteBuffers(1, (GLuint*)&buf);
    glBindBuffer(target, 0);
    return (void*)newbuf;
#else
    return Renderer::resizeBuffer(target, buf, size);
#endif
}

void* OpenGLRenderer::mapBuffer(RendererBuffer target, void* buf, unsigned int offset, unsigned int size, RendererAccess access)
{
#ifdef VDPM_RENDERER_OPENGL_VBO
    GLenum gltarget = (target == RENDERER_VERTEX_BUFFER) ? GL_ARRAY_BUFFER : GL_ELEMENT_ARRAY_BUFFER;
    GLbitfield fields;
    void* ptr;
    switch (access)
    {
    case RENDERER_READ_ONLY:
        fields = GL_MAP_READ_BIT;
        break;

    case RENDERER_WRITE_ONLY:
        fields = GL_MAP_WRITE_BIT | GL_MAP_FLUSH_EXPLICIT_BIT;
        break;

    default:
        fields = GL_MAP_READ_BIT | GL_MAP_WRITE_BIT | GL_MAP_FLUSH_EXPLICIT_BIT;
    }
    glBindBuffer(gltarget, (GLuint)buf);
    ptr = glMapBufferRange(gltarget, offset, size, fields);
    glBindBuffer(target, 0);
    return ptr;

#else
    return Renderer::mapBuffer(target, buf, offset, size, access);
#endif
}

void OpenGLRenderer::unmapBuffer(RendererBuffer target, void* buf)
{
#ifdef VDPM_RENDERER_OPENGL_VBO
    GLenum gltarget = (target == RENDERER_VERTEX_BUFFER) ? GL_ARRAY_BUFFER : GL_ELEMENT_ARRAY_BUFFER;
    glBindBuffer(gltarget, (GLuint)buf);
    glUnmapBuffer(gltarget);
    glBindBuffer(gltarget, 0);
#else
    Renderer::unmapBuffer(target, buf);
#endif
}

void OpenGLRenderer::flushBuffer(RendererBuffer target, void* buf, unsigned int offset, unsigned int size)
{
#ifdef VDPM_RENDERER_OPENGL_VBO
    GLenum gltarget = (target == RENDERER_VERTEX_BUFFER) ? GL_ARRAY_BUFFER : GL_ELEMENT_ARRAY_BUFFER;
    glBindBuffer(gltarget, (GLuint)buf);
    glFlushMappedBufferRange(gltarget, offset, size);
    glBindBuffer(gltarget, 0);
#else
    Renderer::flushBuffer(target, buf, offset, size);
#endif
}

void OpenGLRenderer::updateViewport(Viewport* viewport)
{
    float proj[16], modl[16], clip[16], x, y, z;

    // Get the current PROJECTION matrix from OpenGL
    glGetFloatv(GL_PROJECTION_MATRIX, proj);

    // Get the current MODELVIEW matrix from OpenGL
    glGetFloatv(GL_MODELVIEW_MATRIX, modl);

    // Combine the two matrices (multiply projection by modelview)
    clip[0] = modl[0] * proj[0] + modl[1] * proj[4] + modl[2] * proj[8] + modl[3] * proj[12];
    clip[1] = modl[0] * proj[1] + modl[1] * proj[5] + modl[2] * proj[9] + modl[3] * proj[13];
    clip[2] = modl[0] * proj[2] + modl[1] * proj[6] + modl[2] * proj[10] + modl[3] * proj[14];
    clip[3] = modl[0] * proj[3] + modl[1] * proj[7] + modl[2] * proj[11] + modl[3] * proj[15];

    clip[4] = modl[4] * proj[0] + modl[5] * proj[4] + modl[6] * proj[8] + modl[7] * proj[12];
    clip[5] = modl[4] * proj[1] + modl[5] * proj[5] + modl[6] * proj[9] + modl[7] * proj[13];
    clip[6] = modl[4] * proj[2] + modl[5] * proj[6] + modl[6] * proj[10] + modl[7] * proj[14];
    clip[7] = modl[4] * proj[3] + modl[5] * proj[7] + modl[6] * proj[11] + modl[7] * proj[15];

    clip[8] = modl[8] * proj[0] + modl[9] * proj[4] + modl[10] * proj[8] + modl[11] * proj[12];
    clip[9] = modl[8] * proj[1] + modl[9] * proj[5] + modl[10] * proj[9] + modl[11] * proj[13];
    clip[10] = modl[8] * proj[2] + modl[9] * proj[6] + modl[10] * proj[10] + modl[11] * proj[14];
    clip[11] = modl[8] * proj[3] + modl[9] * proj[7] + modl[10] * proj[11] + modl[11] * proj[15];

    clip[12] = modl[12] * proj[0] + modl[13] * proj[4] + modl[14] * proj[8] + modl[15] * proj[12];
    clip[13] = modl[12] * proj[1] + modl[13] * proj[5] + modl[14] * proj[9] + modl[15] * proj[13];
    clip[14] = modl[12] * proj[2] + modl[13] * proj[6] + modl[14] * proj[10] + modl[15] * proj[14];
    clip[15] = modl[12] * proj[3] + modl[13] * proj[7] + modl[14] * proj[11] + modl[15] * proj[15];

    // RIGHT plane
    viewport->setViewClipPlane(0, clip[3] - clip[0], clip[7] - clip[4], clip[11] - clip[8], clip[15] - clip[12]);

    // LEFT plane
    viewport->setViewClipPlane(1, clip[3] + clip[0], clip[7] + clip[4], clip[11] + clip[8], clip[15] + clip[12]);

    // BOTTOM plane
    viewport->setViewClipPlane(2, clip[3] + clip[1], clip[7] + clip[5], clip[11] + clip[9], clip[15] + clip[13]);

    // TOP plane
    viewport->setViewClipPlane(3, clip[3] - clip[1], clip[7] - clip[5], clip[11] - clip[9], clip[15] - clip[13]);

    // FAR plane
    viewport->setViewClipPlane(4, clip[3] - clip[2], clip[7] - clip[6], clip[11] - clip[10], clip[15] - clip[14]);

    // NEAR plane
    viewport->setViewClipPlane(5, clip[3] + clip[2], clip[7] + clip[6], clip[11] + clip[10], clip[15] + clip[14]);

    x = -(modl[0] * modl[12] + modl[1] * modl[13] + modl[2] * modl[14]);
    y = -(modl[4] * modl[12] + modl[5] * modl[13] + modl[6] * modl[14]);
    z = -(modl[8] * modl[12] + modl[9] * modl[13] + modl[10] * modl[14]);
    viewport->setViewPosition(x, y, z);
}

void OpenGLRenderer::draw(SRMesh* srmesh)
{
    unsigned int vgeomSize = srmesh->getVGeomSize();

    glPushClientAttrib(GL_CLIENT_VERTEX_ARRAY_BIT);

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);

#ifdef VDPM_RENDERER_OPENGL_VBO
    glBindBuffer(GL_ARRAY_BUFFER, (GLuint)srmesh->getArrayBuffer());

    glVertexPointer(3, GL_FLOAT, vgeomSize, (void*)offsetof(VGeom, point));
    glNormalPointer(GL_FLOAT, vgeomSize, (void*)offsetof(VGeom, normal));
#else
    glVertexPointer(3, GL_FLOAT, vgeomSize, srmesh->getVertexPointer());
    glNormalPointer(GL_FLOAT, vgeomSize, srmesh->getNormalPointer());
#endif // VDPM_RENDERER_OPENGL_VBO

    if (srmesh->hasColor())
    {
        glEnableClientState(GL_COLOR_ARRAY);

    #ifdef VDPM_RENDERER_OPENGL_VBO
        glColorPointer(3, GL_FLOAT, vgeomSize, (void*)srmesh->getColorOffset());
    #else
        glColorPointer(3, GL_FLOAT, vgeomSize, srmesh->getColorPointer());
    #endif
    }

    if (srmesh->hasTexCoord())
    {
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    #ifdef VDPM_RENDERER_OPENGL_VBO
        glTexCoordPointer(2, GL_FLOAT, vgeomSize, (void*)srmesh->getTexCoordOffset());
    #else
        glTexCoordPointer(2, GL_FLOAT, vgeomSize, srmesh->getTexCoordPointer());
    #endif
    }

#ifdef VDPM_RENDERER_OPENGL_IBO
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, (GLuint)srmesh->getElementArrayBuffer());

    glMultiDrawElements(GL_TRIANGLE_STRIP, (const GLsizei*)srmesh->getIndicesCountPointer(), GL_UNSIGNED_INT,
        (const GLvoid **)srmesh->getIndicesPointer(), srmesh->getTStripCount());

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
#else
    glMultiDrawElements(GL_TRIANGLE_STRIP, (const GLsizei*)srmesh->getIndicesCountPointer(), GL_UNSIGNED_INT,
        (const GLvoid **)srmesh->getIndicesPointer(), srmesh->getTStripCount());

#endif // VDPM_RENDERER_OPENGL_IBO

#ifdef VDPM_RENDERER_OPENGL_VBO
    glBindBuffer(GL_ARRAY_BUFFER, 0);
#endif

    glPopClientAttrib();
}
