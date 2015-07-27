#ifndef VDPM_OPENGLRENDERER_H
#define VDPM_OPENGLRENDERER_H

#include "vdpm/Renderer.h"

namespace vdpm
{
    class OpenGLRenderer : public Renderer
    {
    public:
        static OpenGLRenderer& getInstance();

        void* createBuffer(RendererBuffer target, unsigned int size, const void* data);
        void destroyBuffer(void* buf);
        void setBufferData(RendererBuffer target, void* buf, unsigned int offset, unsigned int size, const void* data);
        void* resizeBuffer(RendererBuffer target, void* buf, unsigned int size);
        void* mapBuffer(RendererBuffer target, void* buf, unsigned int offset, unsigned int size, RendererAccess access);
        void unmapBuffer(RendererBuffer target, void* buf);
        void flushBuffer(RendererBuffer target, void* buf, unsigned int offset, unsigned int size);

        void updateViewport(Viewport* viewport);
        void draw(SRMesh* srmesh);

    protected:
        OpenGLRenderer();
        ~OpenGLRenderer();
    };
} // namespace vdpm

#endif // VDPM_OPENGLRENDERER_H
