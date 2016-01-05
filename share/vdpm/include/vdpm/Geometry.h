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
#ifndef VDPM_GEOMETRY_H
#define VDPM_GEOMETRY_H

#include "vdpm/Types.h"
#include "vdpm/Renderer.h"

namespace vdpm
{
    class Geometry
    {
        friend class SRMesh;

    public:
        int create(unsigned int count, bool hasColor, bool hasTexCoord);
        void destroy();
        int realize(Renderer* renderer);
        int resize(unsigned int count);

    #ifdef VDPM_RENDERER_OPENGL_VBO_MAP_VGEOM
        void mapVGeom();
        void unmapVGeom();
    #endif // VDPM_RENDERER_OPENGL_VBO_MAP_VGEOM

        float* getVertexPointer() { return &vgeoms->point.x; }
        float* getNormalPointer() { return &vgeoms->normal.x; }
        float* getColorPointer() { return hasColor ? (float*)((uint8_t*)vgeoms + colorOffset) : 0; }
        float* getTexCoordPointer() { return hasTexCoord ? (float*)((uint8_t*)vgeoms + texCoordOffset) : 0; }
        VGeom* getVGeom(unsigned int index) { return (VGeom*)((uint8_t*)vgeoms + vgeomSize * index); }
        float* getVGeomColor(const VGeom* vgeom) { return (float*)((uint8_t*)vgeom + colorOffset); }
        float* getVGeomTexCoord(const VGeom* vgeom) { return (float*)((uint8_t*)vgeom + texCoordOffset); }

    private:
        Geometry() {}

        VGeom* vgeoms;
        void* vbo;

        unsigned int vgeomCount, vgeomSize, colorOffset, texCoordOffset;
        bool hasColor, hasTexCoord;

        Renderer* renderer;
    };

} // namespace vdpm

#endif // VDPM_GEOMETRY_H
