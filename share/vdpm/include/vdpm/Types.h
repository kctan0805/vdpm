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
#ifndef VDPM_TYPES_H
#define VDPM_TYPES_H

#include <cstdint>
#include "vdpm/Config.h"
#include "vdpm/Utility.h"

namespace vdpm
{
    typedef Vector Point;

    struct AVertex;
    struct AFace;
    struct TStrip;
    struct VMorph;

    struct VGeom
    {
        Point point;
        Vector normal;
    };

    struct VGeomAll : public VGeom
    {
        float padding[VDPM_MAX_ATTRIBS];
    };

    struct Vertex
    {
        AVertex* avertex;
        Vertex* parent;
        unsigned int i;
    };

    struct Face
    {
        AFace* aface;
    };

    struct VSplit
    {
        unsigned int vt_i, vu_i;
        Face *fn0, *fn1, *fn2, *fn3;
        float radius, sin2alpha, uni_error, dir_error;
    };

    struct AVertex
    {
        AVertex *prev, *next;
        Vertex* vertex;
        unsigned int i;
        VMorph* vmorph;
    };

    struct AFace
    {
        AFace *prev, *next;
        AVertex *v0, *v1, *v2;
        AFace *n0, *n1, *n2;
        TStrip* tstrip;
    };

    struct TStrip
    {
        TStrip *prev, *next;
        AFace* afaces;
        unsigned int* vgIndices;
        unsigned int vgCount;
    #if defined(VDPM_GEOMORPHS) && !defined(VDPM_RECREATE_TSTRIPS)
        unsigned short gtime;
    #endif
    };

#ifdef VDPM_GEOMORPHS
    struct VMorph
    {
        VMorph *prev, *next;
        AVertex* avertex;
        bool coarsening;
        unsigned short gtime;
        unsigned int vgIndex;
        VGeomAll vgInc;
    };
#endif // VDPM_GEOMORPHS

    class Allocator;
    class Renderer;
    class SRMesh;
    class Viewport;

} // namespace vdpm

#endif // VDPM_TYPES_H
