#ifndef VDPM_RENDERER_H
#define VDPM_RENDERER_H

#include "vdpm/Types.h"

namespace vdpm
{
    enum RendererAccess
    {
        RENDERER_READ_ONLY, 
        RENDERER_WRITE_ONLY,
        RENDERER_READ_WRITE
    };

    enum RendererBuffer
    {
        RENDERER_VERTEX_BUFFER,
        RENDERER_INDEX_BUFFER
    };

    class Renderer
    {
    public:
        virtual void* createBuffer(RendererBuffer target, unsigned int size, const void* data);
        virtual void destroyBuffer(void* buf);
        virtual void setBufferData(RendererBuffer target, void* buf, unsigned int offset, unsigned int size, const void* data);
        virtual void* resizeBuffer(RendererBuffer target, void* buf, unsigned int size);
        virtual void* mapBuffer(RendererBuffer target, void* buf, unsigned int offset, unsigned int size, RendererAccess access);
        virtual void unmapBuffer(RendererBuffer target, void* buf);
        virtual void flushBuffer(RendererBuffer target, void* buf, unsigned int offset, unsigned int size);

        virtual void updateViewport(Viewport* viewport) = 0;
        virtual void draw(SRMesh* srmesh) = 0;

    protected:
        Renderer();
    };
} // namespace vdpm

#endif // VDPM_RENDERER_H
