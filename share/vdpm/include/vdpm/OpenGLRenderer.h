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
