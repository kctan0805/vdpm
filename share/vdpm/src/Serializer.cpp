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
#include <fstream>
#include "vdpm/Allocator.h"
#include "vdpm/Serializer.h"
#include "vdpm/SRMesh.h"
#include "vdpm/StdInStream.h"

using namespace std;
using namespace vdpm;

#define VDPM_FILE_FORMAT_MAGIC \
        ((long)'v' + ((long)'d' << 8) + ((long)'p' << 16) + ((long)'m' << 24))

#define VDPM_FILE_FORMAT_SRMESH     0x00000001
#define VDPM_FILE_FORMAT_TEXNAME    0x00000002
#define VDPM_FILE_FORMAT_END        0x00000003
#define VDPM_HAS_COLOR              0x00000001
#define VDPM_HAS_TEXCOORD           0x00000002

Serializer::Serializer()
{
    // do nothing
}

Serializer::~Serializer()
{
}

Serializer& Serializer::getInstance()
{
    static Serializer self;
    return self;
}

int Serializer::readTextureName(InStream& is, SRMesh* srmesh)
{
    uint32_t len;

    is.readUInt(len);
    srmesh->texname = new char[len];
    if (!srmesh->texname)
        goto error;

    for (unsigned int i = 0; i < len; ++i)
        is.readChar(srmesh->texname[i]);

    return 0;

error:
    delete[] srmesh->texname;
    return -1;
}

int Serializer::readSRMesh(InStream& is, SRMesh* srmesh)
{
    AVertex* avertex;
    AFace* aface;
    uint32_t index, vt_i, vu_i, i, flags;
    bool hasColor, hasTexCoord;
    unsigned int vgeomCount;

    is.readUInt(flags);

    is.readUInt(srmesh->baseVCount);
    is.readUInt(srmesh->baseFCount);
    is.readUInt(srmesh->vsplitCount);

    srmesh->vcount = srmesh->baseVCount + srmesh->vsplitCount * 2;
    srmesh->vertices = new Vertex[srmesh->vcount];
    if (!srmesh->vertices)
        goto error;

    for (i = 0; i < srmesh->vcount; ++i)
    {
        srmesh->vertices[i].avertex = NULL;
        is.readUInt(index);
        srmesh->vertices[i].parent = (index == UINT_MAX) ? NULL : &srmesh->vertices[index];
        is.readUInt(srmesh->vertices[i].i);
    }

    vgeomCount = srmesh->vcount;

#ifdef VDPM_GEOMORPHS
    srmesh->vmorphSize = srmesh->vcount / 16;
    if (srmesh->vmorphSize == 0)
        srmesh->vmorphSize = 32;

    vgeomCount += srmesh->vmorphSize;
#endif

    hasColor = (flags & VDPM_HAS_COLOR) ? true : false;
    hasTexCoord = (flags & VDPM_HAS_TEXCOORD) ? true : false;
    srmesh->geometry.create(vgeomCount, hasColor, hasTexCoord);

    for (i = 0; i < srmesh->baseVCount; ++i)
    {
        VGeom* vgeom = srmesh->geometry.getVGeom(i);

        avertex = Allocator::getInstance().allocAVertex();

        is.readFloat(vgeom->point.x);
        is.readFloat(vgeom->point.y);
        is.readFloat(vgeom->point.z);
        is.readFloat(vgeom->normal.x);
        is.readFloat(vgeom->normal.y);
        is.readFloat(vgeom->normal.z);

        if (hasColor)
        {
            float* ptr = srmesh->geometry.getVGeomColor(vgeom);
            for (int j = 0; j < 3; ++j)
                is.readFloat(*ptr++);
        }
        if (hasTexCoord)
        {
            float* ptr = srmesh->geometry.getVGeomTexCoord(vgeom);
            for (int j = 0; j < 2; ++j)
                is.readFloat(*ptr++);
        }

        avertex->i = i;
        srmesh->vertices[i].avertex = avertex;
        avertex->vertex = &srmesh->vertices[i];
        avertex->vmorph = NULL;
        srmesh->addAVertex(avertex);
    }
    for (i = 0; i < srmesh->vsplitCount; ++i)
    {
        VGeom* vgeom;

        vt_i = srmesh->baseVCount + i * 2;
        vu_i = srmesh->baseVCount + i * 2 + 1;

        vgeom = srmesh->geometry.getVGeom(vt_i);

        is.readFloat(vgeom->point.x);
        is.readFloat(vgeom->point.y);
        is.readFloat(vgeom->point.z);
        is.readFloat(vgeom->normal.x);
        is.readFloat(vgeom->normal.y);
        is.readFloat(vgeom->normal.z);

        if (hasColor)
        {
            float* ptr = srmesh->geometry.getVGeomColor(vgeom);
            for (int j = 0; j < 3; ++j)
                is.readFloat(*ptr++);
        }
        if (hasTexCoord)
        {
            float* ptr = srmesh->geometry.getVGeomTexCoord(vgeom);
            for (int j = 0; j < 2; ++j)
                is.readFloat(*ptr++);
        }

        vgeom = srmesh->geometry.getVGeom(vu_i);

        is.readFloat(vgeom->point.x);
        is.readFloat(vgeom->point.y);
        is.readFloat(vgeom->point.z);
        is.readFloat(vgeom->normal.x);
        is.readFloat(vgeom->normal.y);
        is.readFloat(vgeom->normal.z);

        if (hasColor)
        {
            float* ptr = srmesh->geometry.getVGeomColor(vgeom);
            for (int j = 0; j < 3; ++j)
                is.readFloat(*ptr++);
        }
        if (hasTexCoord)
        {
            float* ptr = srmesh->geometry.getVGeomTexCoord(vgeom);
            for (int j = 0; j < 2; ++j)
                is.readFloat(*ptr++);
        }
    }
#ifdef VDPM_GEOMORPHS
    for (i = srmesh->vcount; i < vgeomCount; ++i)
    {
        VGeom* vgeom = srmesh->geometry.getVGeom(i);
        *(unsigned int*)&vgeom->point.x = UINT_MAX;
    }
#endif // VDPM_GEOMORPHS

    srmesh->fcount = srmesh->baseFCount + srmesh->vsplitCount * 2;
    srmesh->faces = new Face[srmesh->fcount];
    if (!srmesh->faces)
        goto error;

    for (i = 0; i < srmesh->fcount; ++i)
        srmesh->faces[i].aface = NULL;

    for (i = 0; i < srmesh->baseFCount; ++i)
    {
        aface = Allocator::getInstance().allocAFace();

        is.readUInt(index);
        aface->v0 = srmesh->vertices[index].avertex;
        is.readUInt(index);
        aface->v1 = srmesh->vertices[index].avertex;
        is.readUInt(index);
        aface->v2 = srmesh->vertices[index].avertex;

        srmesh->faces[i].aface = aface;
        aface->tstrip = NULL;
        srmesh->addAFace(aface);
    }
    // update neighbors of afaces
    for (i = 0; i < srmesh->baseFCount; ++i)
    {
        is.readUInt(index);
        srmesh->faces[i].aface->n0 = (index == UINT_MAX) ? NULL : srmesh->faces[index].aface;
        is.readUInt(index);
        srmesh->faces[i].aface->n1 = (index == UINT_MAX) ? NULL : srmesh->faces[index].aface;
        is.readUInt(index);
        srmesh->faces[i].aface->n2 = (index == UINT_MAX) ? NULL : srmesh->faces[index].aface;
    }

    srmesh->vsplits = new VSplit[srmesh->vsplitCount];
    if (!srmesh->faces)
        goto error;

    for (i = 0; i < srmesh->vsplitCount; ++i)
    {
        vt_i = srmesh->baseVCount + i * 2;
        vu_i = srmesh->baseVCount + i * 2 + 1;

        srmesh->vsplits[i].vt_i = vt_i;
        srmesh->vsplits[i].vu_i = vu_i;

        is.readUInt(index);
        srmesh->vsplits[i].fn0 = (index == UINT_MAX) ? NULL : &srmesh->faces[index];
        is.readUInt(index);
        srmesh->vsplits[i].fn1 = (index == UINT_MAX) ? NULL : &srmesh->faces[index];
        is.readUInt(index);
        srmesh->vsplits[i].fn2 = (index == UINT_MAX) ? NULL : &srmesh->faces[index];
        is.readUInt(index);
        srmesh->vsplits[i].fn3 = (index == UINT_MAX) ? NULL : &srmesh->faces[index];

        is.readFloat(srmesh->vsplits[i].radius);
        is.readFloat(srmesh->vsplits[i].sin2alpha);
        is.readFloat(srmesh->vsplits[i].uni_error);
        is.readFloat(srmesh->vsplits[i].dir_error);
    }
    return 0;

error:
    delete[] srmesh->vsplits;
    delete[] srmesh->faces;
    srmesh->geometry.destroy();
    delete[] srmesh->vertices;
    return -1;
}

SRMesh* Serializer::readSRMesh(InStream& is)
{
    SRMesh* srmesh = NULL;

    srmesh = new SRMesh();
    if (!srmesh)
        goto error;

    if (readSRMesh(is, srmesh))
        goto error;

#ifdef VDPM_BOUNDS
    for (unsigned int i = 0; i < srmesh->baseVCount; ++i)
    {
        VGeom* vgeom = srmesh->getVGeom(i);
        srmesh->bounds.addPoint(vgeom->point, true);
    }
    srmesh->bounds.complete();

#endif // VDPM_BOUNDS

    srmesh->allocator = &Allocator::getInstance();

    return srmesh;

error:
    delete srmesh;
    return NULL;
}

SRMesh* Serializer::loadSRMesh(InStream& is)
{
    SRMesh* srmesh = NULL;
    uint32_t magic, token;
    bool finished = false;

    srmesh = new SRMesh();
    if (!srmesh)
        goto error;

    is.readUInt(magic);
    if (VDPM_FILE_FORMAT_MAGIC != magic)
        goto error;

    while (!finished)
    {
        is.readUInt(token);

        switch (token)
        {
        case VDPM_FILE_FORMAT_SRMESH:
            if (readSRMesh(is, srmesh))
                goto error;

            break;

        case VDPM_FILE_FORMAT_TEXNAME:
            if (readTextureName(is, srmesh))
                goto error;

            break;

        case VDPM_FILE_FORMAT_END:
            finished = true;
            break;
        }
    }

#ifdef VDPM_BOUNDS
    for (unsigned int i = 0; i < srmesh->baseVCount; ++i)
    {
        VGeom* vgeom = srmesh->getVGeom(i);
        srmesh->bounds.addPoint(vgeom->point, true);
    }
    srmesh->bounds.complete();

#endif // VDPM_BOUNDS

    srmesh->allocator = &Allocator::getInstance();

    return srmesh;

error:
    delete srmesh;
    return NULL;
}

SRMesh* Serializer::loadSRMesh(const char filePath[])
{
    StdInStream is(filePath);
    SRMesh* srmesh = loadSRMesh(is);
    return srmesh;
}

int Serializer::writeSRMesh(OutStream& os, SRMesh* srmesh)
{
    uint32_t index, vt_i, vu_i, i, flags;
    AFace* aface;

    flags = 0;

    if (srmesh->hasColor())
        flags |= VDPM_HAS_COLOR;

    if (srmesh->hasTexCoord())
        flags |= VDPM_HAS_TEXCOORD;

    os.writeUInt(flags);

    os.writeUInt(srmesh->baseVCount);
    os.writeUInt(srmesh->baseFCount);
    os.writeUInt(srmesh->vsplitCount);

    for (i = 0; i < srmesh->vcount; ++i)
    {
        if (srmesh->vertices[i].parent == NULL)
            index = UINT_MAX;
        else
            index = srmesh->vertices[i].parent - srmesh->vertices;

        os.writeUInt(index);
        os.writeUInt(srmesh->vertices[i].i);
    }

    for (i = 0; i < srmesh->baseVCount; ++i)
    {
        VGeom* vgeom = srmesh->geometry.getVGeom(i);

        os.writeFloat(vgeom->point.x);
        os.writeFloat(vgeom->point.y);
        os.writeFloat(vgeom->point.z);
        os.writeFloat(vgeom->normal.x);
        os.writeFloat(vgeom->normal.y);
        os.writeFloat(vgeom->normal.z);

        if (srmesh->hasColor())
        {
            float* ptr = srmesh->geometry.getVGeomColor(vgeom);
            for (int j = 0; j < 3; ++j)
                os.writeFloat(*ptr++);
        }
        if (srmesh->hasTexCoord())
        {
            float* ptr = srmesh->geometry.getVGeomTexCoord(vgeom);
            for (int j = 0; j < 2; ++j)
                os.writeFloat(*ptr++);
        }
    }

    for (i = 0; i < srmesh->vsplitCount; ++i)
    {
        VGeom* vgeom;

        vt_i = srmesh->baseVCount + i * 2;
        vu_i = srmesh->baseVCount + i * 2 + 1;

        vgeom = srmesh->geometry.getVGeom(vt_i);

        os.writeFloat(vgeom->point.x);
        os.writeFloat(vgeom->point.y);
        os.writeFloat(vgeom->point.z);
        os.writeFloat(vgeom->normal.x);
        os.writeFloat(vgeom->normal.y);
        os.writeFloat(vgeom->normal.z);

        if (srmesh->hasColor())
        {
            float* ptr = srmesh->geometry.getVGeomColor(vgeom);
            for (int j = 0; j < 3; ++j)
                os.writeFloat(*ptr++);
        }
        if (srmesh->hasTexCoord())
        {
            float* ptr = srmesh->geometry.getVGeomTexCoord(vgeom);
            for (int j = 0; j < 2; ++j)
                os.writeFloat(*ptr++);
        }

        vgeom = srmesh->geometry.getVGeom(vu_i);

        os.writeFloat(vgeom->point.x);
        os.writeFloat(vgeom->point.y);
        os.writeFloat(vgeom->point.z);
        os.writeFloat(vgeom->normal.x);
        os.writeFloat(vgeom->normal.y);
        os.writeFloat(vgeom->normal.z);

        if (srmesh->hasColor())
        {
            float* ptr = srmesh->geometry.getVGeomColor(vgeom);
            for (int j = 0; j < 3; ++j)
                os.writeFloat(*ptr++);
        }
        if (srmesh->hasTexCoord())
        {
            float* ptr = srmesh->geometry.getVGeomTexCoord(vgeom);
            for (int j = 0; j < 2; ++j)
                os.writeFloat(*ptr++);
        }
    }

    assert(srmesh->afaceCount == srmesh->baseFCount);
    aface = srmesh->afacesEnd.prev;
    while (aface != &srmesh->afaces)
    {
        index = aface->v0->vertex - srmesh->vertices;
        os.writeUInt(index);
        index = aface->v1->vertex - srmesh->vertices;
        os.writeUInt(index);
        index = aface->v2->vertex - srmesh->vertices;
        os.writeUInt(index);

        aface = aface->prev;
    }

    for (i = 0; i < srmesh->baseFCount; ++i)
    {
        if (srmesh->faces[i].aface->n0 == NULL)
        {
            index = UINT_MAX;
        }
        else
        {
            unsigned int j;

            for (j = 0; j < srmesh->baseFCount; ++j)
            {
                if (srmesh->faces[j].aface == srmesh->faces[i].aface->n0)
                {
                    index = j;
                    break;
                }
            }
            assert(j < srmesh->baseFCount);
        }
        os.writeUInt(index);

        if (srmesh->faces[i].aface->n1 == NULL)
        {
            index = UINT_MAX;
        }
        else
        {
            unsigned int j;

            for (j = 0; j < srmesh->baseFCount; ++j)
            {
                if (srmesh->faces[j].aface == srmesh->faces[i].aface->n1)
                {
                    index = j;
                    break;
                }
            }
            assert(j < srmesh->baseFCount);
        }
        os.writeUInt(index);

        if (srmesh->faces[i].aface->n2 == NULL)
        {
            index = UINT_MAX;
        }
        else
        {
            unsigned int j;

            for (j = 0; j < srmesh->baseFCount; ++j)
            {
                if (srmesh->faces[j].aface == srmesh->faces[i].aface->n2)
                {
                    index = j;
                    break;
                }
            }
            assert(j < srmesh->baseFCount);
        }
        os.writeUInt(index);
    }

    for (i = 0; i < srmesh->vsplitCount; ++i)
    {
        if (srmesh->vsplits[i].fn0 == NULL)
            index = UINT_MAX;
        else
            index = srmesh->vsplits[i].fn0 - srmesh->faces;

        os.writeUInt(index);

        if (srmesh->vsplits[i].fn1 == NULL)
            index = UINT_MAX;
        else
            index = srmesh->vsplits[i].fn1 - srmesh->faces;

        os.writeUInt(index);

        if (srmesh->vsplits[i].fn2 == NULL)
            index = UINT_MAX;
        else
            index = srmesh->vsplits[i].fn2 - srmesh->faces;

        os.writeUInt(index);

        if (srmesh->vsplits[i].fn3 == NULL)
            index = UINT_MAX;
        else
            index = srmesh->vsplits[i].fn3 - srmesh->faces;

        os.writeUInt(index);

        os.writeFloat(srmesh->vsplits[i].radius);
        os.writeFloat(srmesh->vsplits[i].sin2alpha);
        os.writeFloat(srmesh->vsplits[i].uni_error);
        os.writeFloat(srmesh->vsplits[i].dir_error);
    }

    return 0;
}