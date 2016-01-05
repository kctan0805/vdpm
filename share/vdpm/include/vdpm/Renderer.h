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
