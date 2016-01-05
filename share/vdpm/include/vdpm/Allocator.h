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
#ifndef VDPM_ALLOCATOR_H
#define VDPM_ALLOCATOR_H

#include "vdpm/Types.h"

namespace vdpm
{
    class Allocator
    {
    public:
        ~Allocator();

        static Allocator& getInstance();

        AVertex* allocAVertex();
        void freeAVertex(AVertex* avertex);
        AFace* allocAFace();
        void freeAFace(AFace* aface);
        TStrip* allocTStrip();
        void freeTStrip(TStrip* tstrip, AFace& afaces);

    #ifdef VDPM_GEOMORPHS
        VMorph* allocVMorph();
        void freeVMorph(VMorph* vmorph);
    #endif // VDPM_GEOMORPHS

    private:
        Allocator();

    #ifdef VDPM_REUSE_OBJECTS
        static AVertex* freeAVertices;
        static AFace* freeAFaces;
        static TStrip* freeTStrips;
        static VMorph* freeVMorphs;
    #endif // VDPM_REUSE_OBJECTS
    };
} // namespace vdpm

#endif // VDPM_ALLOCATOR_H
