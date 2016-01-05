/* vdpm - View-dependent progressive meshes library
* Copyright 2015 Jim Tan
* https://github.com/kctan0805/vdpm
*
* osgEarth is free software; you can redistribute it and/or modify
* it under the terms of the GNU Lesser General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>
*/
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
