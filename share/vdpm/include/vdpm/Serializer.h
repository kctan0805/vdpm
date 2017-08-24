/* vdpm - View-dependent progressive meshes library
* Copyright 2015 Jim Tan
* https://github.com/kctan0805/vdpm
*
* vdpm is free software; you can redistribute it and/or modify
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
#ifndef VDPM_SERIALIZER_H
#define VDPM_SERIALIZER_H

#include "vdpm/InStream.h"
#include "vdpm/OutStream.h"
#include "vdpm/Types.h"

namespace vdpm
{
    class Serializer
    {
    public:
        ~Serializer();

        static Serializer& getInstance();
        SRMesh* loadSRMesh(InStream& is);
        SRMesh* loadSRMesh(const char filePath[]);

        SRMesh* readSRMesh(InStream& is);
        int writeSRMesh(OutStream& os, SRMesh* srmesh);

    private:
        Serializer();

        int readSRMesh(InStream& is, SRMesh* srmesh);
        int readTextureName(InStream& is, SRMesh* srmesh);
    };
} // namespace vdpm

#endif // VDPM_SERIALIZER_H
