#include <cassert>
#include <cstdlib>
#include <cstring>
#include "vdpm/Log.h"
#include "vdpm/Renderer.h"

using namespace std;
using namespace vdpm;

Renderer::Renderer()
{
}

void* Renderer::createBuffer(RendererBuffer target, unsigned int size, const void* data)
{
    if (data)
        return (void*)data;

    return ::malloc(size);
}

void Renderer::destroyBuffer(void* buf)
{
    ::free(buf);
}

void Renderer::setBufferData(RendererBuffer target, void* buf, unsigned int offset, unsigned int size, const void* data)
{
    ::memcpy(((uint8_t*)buf) + offset, data, size);
}

void* Renderer::resizeBuffer(RendererBuffer target, void* buf, unsigned int size)
{
    return ::realloc(buf, size);
}

void* Renderer::mapBuffer(RendererBuffer target, void* buf, unsigned int offset, unsigned int size, RendererAccess access)
{
    return &((uint8_t*)buf)[offset];
}

void Renderer::unmapBuffer(RendererBuffer target, void* buf)
{
    // DO NOTHING
}

void Renderer::flushBuffer(RendererBuffer target, void* buf, unsigned int offset, unsigned int size)
{
    // DO NOTHING
}
